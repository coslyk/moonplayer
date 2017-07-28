#include "pyapi.h"
#include "settings_network.h"
#include "settings_plugins.h"
#include "accessmanager.h"
#include "webvideo.h"
#include "downloader.h"
#include "playlist.h"
#include "playercore.h"
#include "reslibrary.h"
#include "detailview.h"
#include "utils.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QApplication>
#include <QColor>
#include <QMessageBox>
#include <QNetworkProxy>
#include <QDir>
#include <QTimer>
#include "danmakudelaygetter.h"
#include "yougetbridge.h"


bool win_debug = false;

/*****************************************
 ******** Some useful functions **********
 ****************************************/
#define RETURN_IF_ERROR(retval)  if ((retval) == NULL){show_pyerr(); return;}
#define EXIT_IF_ERROR(retval)    if ((retval) == NULL){show_pyerr(); exit(EXIT_FAILURE);}

void call_py_func_vsi(PyObject *func, const char *first, int second)
{
    PyObject *ret = PyObject_CallFunction(func, "si", first, second);
    RETURN_IF_ERROR(ret)
    Py_DecRef(ret);
}

void show_pyerr()
{
#ifdef Q_OS_WIN
    if (!win_debug)
    {
        PyErr_Clear();
        return;
    }
	PyErr_Print();
    PyRun_SimpleString("sys.stderr.flush()");
#else
    PyErr_Print();
#endif
}

/************************************************
 ** Define download_page() function for python **
 ************************************************/

GetUrl *geturl_obj = NULL;
PyObject *exc_GetUrlError = NULL;

GetUrl::GetUrl(QObject *parent) : QObject(parent)
{
    reply = NULL;
    callbackFunc = NULL;
    data = NULL;
    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeOut()));
    exc_GetUrlError = PyErr_NewException("moonplayer.GetUrlError", NULL, NULL);
}

void GetUrl::start(const char *url, PyObject *callback, PyObject *_data,
                   const QByteArray &referer, const QByteArray &postData)
{
    //save callback function
    callbackFunc = callback;
    data = _data;
    Py_IncRef(callbackFunc);
    Py_IncRef(data);
    //start request
    QNetworkRequest request = QNetworkRequest(QString::fromUtf8(url));
    request.setHeader(QNetworkRequest::UserAgentHeader, generateUA(request.url()));
    if (!referer.isEmpty())
        request.setRawHeader("Referer", referer);
    if (postData.isEmpty())
        reply = access_manager->get(request);
    else
    {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        reply = access_manager->post(request, postData);
    }
    connect(reply, SIGNAL(finished()), this, SLOT(onFinished()));
    timer->start(10000);
    // Set moonplayer.final_url in Python
    PyObject *str = PyString_FromString(url);
    PyObject_SetAttrString(apiModule, "final_url", str);
    Py_DecRef(str);
}

void GetUrl::onTimeOut()
{
    Q_ASSERT(reply);
    reply->abort();
}

void GetUrl::onFinished()
{
    timer->stop();
    //check redirection
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status == 301 || status == 302)
    {
        reply->deleteLater();
        QByteArray final_url = reply->rawHeader("Location");
        PyObject *str = PyString_FromString(final_url.constData());
        PyObject_SetAttrString(apiModule, "final_url", str);
        Py_DecRef(str);
        //start request
        QNetworkRequest request = QNetworkRequest(QString::fromUtf8(final_url));
        request.setRawHeader("User-Agent", generateUA(request.url()));
        reply = access_manager->get(request);
        connect(reply, SIGNAL(finished()), this, SLOT(onFinished()));
        return;
    }
    PyObject *callback = callbackFunc;
    PyObject *_data = data;
    callbackFunc = NULL;
    data = NULL;
    QNetworkReply::NetworkError error = reply->error();
    QByteArray barray = reply->readAll();
    reply->deleteLater();
    if (error != QNetworkReply::NoError)
    {
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QString errStr = QString().sprintf("Network Error: %d\n%s\n", status, reply->errorString().toUtf8().constData());
        QMessageBox::warning(NULL, "Error", errStr);
        Py_DecRef(_data);
        Py_DecRef(callback);
        return;
    }
    PyObject *retVal = PyObject_CallFunction(callback, "sO", barray.constData(), _data);
    Py_DecRef(_data);
    Py_DecRef(callback);
    RETURN_IF_ERROR(retVal)
    Py_DecRef(retVal);
}

static PyObject *get_url(PyObject *, PyObject *args)
{
    PyObject *callback, *data;
    const char *url, *referer = NULL;
    if (!PyArg_ParseTuple(args, "sOO|s", &url, &callback, &data, &referer))
        return NULL;
    if (geturl_obj->hasTask())
    {
        PyErr_SetString(exc_GetUrlError, "Another task is running.");
        return NULL;
    }
    geturl_obj->start(url, callback, data, referer, NULL);
    Py_IncRef(Py_None);
    return Py_None;
}

static PyObject *post_content(PyObject *, PyObject *args)
{
    PyObject *callback, *data;
    const char *url, *post, *referer = NULL;
    if (!PyArg_ParseTuple(args, "ssOO|s", &url, &post, &callback, &data, &referer))
        return NULL;
    if (geturl_obj->hasTask())
    {
        PyErr_SetString(exc_GetUrlError, "Another task is running.");
        return NULL;
    }
    geturl_obj->start(url, callback, data, referer, post);
    Py_IncRef(Py_None);
    return Py_None;
}

static PyObject *bind_referer(PyObject *, PyObject *args)
{
    const char *host, *url;
    if (!PyArg_ParseTuple(args, "ss", &host, &url))
        return NULL;
    referer_table[QString::fromUtf8(host)] = url;
    return Py_None;
}

static PyObject *force_unseekable(PyObject *, PyObject *args)
{
    const char *s;
    if (!PyArg_ParseTuple(args, "s", &s))
        return NULL;
    QString host = QString::fromUtf8(s);
    if (!unseekable_hosts.contains(host))
        unseekable_hosts.append(host);
    return Py_None;
}

/********************
 * Dialog functions *
 ********************/
static PyObject *warn(PyObject *, PyObject *args)
{
    const char *msg;
    if (!PyArg_ParseTuple(args, "s", &msg))
        return NULL;
    QMessageBox::warning(NULL, "Warning", QString::fromUtf8(msg));
    Py_IncRef(Py_None);
    return Py_None;
}

static PyObject *question(PyObject *, PyObject *args)
{
    const char *msg;
    if (!PyArg_ParseTuple(args, "s", &msg))
        return NULL;
    if (QMessageBox::question(NULL, "question", QString::fromUtf8(msg), QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes)
    {
        Py_IncRef(Py_True);
        return Py_True;
    }
    Py_IncRef(Py_False);
    return Py_False;
}

/************************
 ** WebVideo functions **
 ************************/
static PyObject *show_list(PyObject *, PyObject *args)
{
    PyObject *list;
    if (!PyArg_ParseTuple(args, "O", &list))
        return NULL;
    return webvideo->showList(list);
}


/*******************
 ** ResLibrary    **
 *******************/
static PyObject *res_show(PyObject *, PyObject *args)
{
    PyObject *list = NULL;
    if (!PyArg_ParseTuple(args, "O", &list))
        return NULL;
    if (!PyList_Check(list))
        return NULL;
    res_library->clearItem();
    int size = PyList_Size(list);
    for (int i = 0; i < size; i++)
    {
        PyObject *dict = PyList_GetItem(list, i);
        PyObject *name_obj, *pic_url_obj, *flag_obj;
        QString name;
        const char *pic_url, *flag;
        if (NULL == (name_obj = PyDict_GetItemString(dict, "name")))
            return NULL;
        if (NULL == (flag_obj = PyDict_GetItemString(dict, "url")))
            return NULL;
        if (NULL == (pic_url_obj = PyDict_GetItemString(dict, "pic_url")))
            return NULL;
        if ((name = PyString_AsQString(name_obj)).isNull())
            return NULL;
        if (NULL == (flag = PyString_AsString(flag_obj)))
            return NULL;
        if (NULL == (pic_url = PyString_AsString(pic_url_obj)))
            return NULL;
        res_library->addItem(name, pic_url, flag);
    }
    Py_IncRef(Py_None);
    return Py_None;
}

static PyObject *show_detail(PyObject *, PyObject *args)
{
    PyObject *dict = NULL;
    if (!PyArg_ParseTuple(args, "O", &dict))
        return NULL;
    DetailView *detailview;
    if (webvideo->count() == 4) // DetailView page has been created
        detailview = static_cast<DetailView*>(webvideo->widget(3));
    else
    {
        detailview = new DetailView(res_library);
        webvideo->addTab(detailview, "");
    }
    PyObject *retVal = detailview->loadDetail(dict);
    webvideo->setCurrentWidget(detailview);
    webvideo->setTabText(3, detailview->windowTitle());
    return retVal;
}

/*******************
 ** Define module **
 *******************/

static PyMethodDef methods[] = {
    {"download_page",    get_url,          METH_VARARGS, "Send a HTTP-GET request"},
    {"get_url",          get_url,          METH_VARARGS, "Send a HTTP-GET request (Obsolete method)"},
    {"post_content",     post_content,     METH_VARARGS, "Send a HTTP-POST request"},
    {"bind_referer",     bind_referer,     METH_VARARGS, "Bind a host with referer"},
    {"force_unseekable", force_unseekable, METH_VARARGS, "Force stream with specific host to be unseekable"},
    {"warn",             warn,             METH_VARARGS, "Show warning message"},
    {"question",         question,         METH_VARARGS, "Show a question dialog"},
    {"show_list",        show_list,        METH_VARARGS, "Show searching result on the list"},
    {"res_show",         res_show,         METH_VARARGS, "Show resources result"},
    {"show_detail",      show_detail,      METH_VARARGS, "Show detail"},
    {NULL, NULL, 0, NULL}
};

PyObject *apiModule = NULL;

void initAPI()
{
    //init python
    Py_Initialize();
    if (!Py_IsInitialized())
    {
        qDebug("Cannot initialize python.");
        exit(-1);
    }
    //init module
    geturl_obj = new GetUrl(qApp);
    apiModule = Py_InitModule("moonplayer", methods);
    PyModule_AddStringConstant(apiModule, "final_url", "");
    Py_IncRef(exc_GetUrlError);
    PyModule_AddObject(apiModule, "GetUrlError", exc_GetUrlError);
}
