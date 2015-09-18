#include "pyapi.h"
#include "settings_network.h"
#include "settings_plugins.h"
#include "accessmanager.h"
#include "webvideo.h"
#include "downloader.h"
#include "playlist.h"
#include "mplayer.h"
#include "reslibrary.h"
#include "detailview.h"
#include "danmakudelaygetter.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QApplication>
#include <QColor>
#include <QMessageBox>
#include <QNetworkProxy>
#include <QDir>
#include <QTimer>

/*****************************************
 ******** Some useful functions **********
 ****************************************/
#define RETURN_IF_ERROR(retval)  if ((retval) == NULL){PyErr_Print(); return;}
#define EXIT_IF_ERROR(retval)    if ((retval) == NULL){PyErr_Print(); exit(-1);}

void call_py_func_vsi(PyObject *func, const char *first, int second)
{
    PyObject *ret = PyObject_CallFunction(func, "si", first, second);
    RETURN_IF_ERROR(ret)
    Py_DecRef(ret);
}

/******************************************
 ** Define get_url() function for python **
 *****************************************/

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

void GetUrl::start(const char *url, PyObject *callback, PyObject *_data)
{
    //save callback function
    callbackFunc = callback;
    data = _data;
    Py_IncRef(callbackFunc);
    Py_IncRef(data);
    //start request
    QNetworkRequest request = QNetworkRequest(QString::fromUtf8(url));
    request.setRawHeader("User-Agent", "moonplayer");
    reply = access_manager->get(request);
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
        request.setRawHeader("User-Agent", "moonplayer");
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
    reply = 0;
    if (error != QNetworkReply::NoError)
    {
        QMessageBox::warning(NULL, "Error", "Network Error");
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
    const char *url;
    if (!PyArg_ParseTuple(args, "sOO", &url, &callback, &data))
        return NULL;
    if (geturl_obj->hasTask())
    {
        PyErr_SetString(exc_GetUrlError, "Another task is running.");
        return NULL;
    }
    geturl_obj->start(url, callback, data);
    Py_IncRef(Py_None);
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

static PyObject *download(PyObject *, PyObject *args)
{
    //read args
    PyObject *list;
    const char *childDir = NULL;
    int ok;
    ok = PyArg_ParseTuple(args, "O|s", &list, &childDir);
    if (!ok)
        return NULL;
    int size = PyList_Size(list);
    if (size < 0)
        return NULL;
    //set save dir
    QDir dir(Settings::downloadDir);
    if (childDir)
    {
        QString child = QString::fromUtf8(childDir);
        if (!dir.cd(child))
        {
            dir.mkdir(child);
            dir.cd(child);
        }
    }
    //add task
    PyObject *item;
    const char *str;
    for (int i = 0; i < size; i += 2)
    {
        if ((item = PyList_GetItem(list, i)) == NULL)
            return NULL;
        if ((str = PyString_AsString(item)) == NULL)
            return NULL;
        QString name = QString::fromUtf8(str);
        if ((item = PyList_GetItem(list, i+1)) == NULL)
            return NULL;
        if ((str = PyString_AsString(item)) == NULL)
            return NULL;
        downloader->addTask(str, dir.filePath(name), (bool) childDir);
    }
    QMessageBox::information(webvideo, "Message", "Add task successfully.");
    Py_IncRef(Py_None);
    return Py_None;
}

static PyObject *play(PyObject *, PyObject *args)
{
    PyObject *list;
    const char *danmaku_url = NULL;
    if (!PyArg_ParseTuple(args, "O|s", &list, &danmaku_url))
        return NULL;
    if (!PyList_Check(list))
        return NULL;

    int size = PyList_Size(list);
    PyObject *item;
    const char *str;
    QStringList names;
    QStringList urls;
    for (int i = 0; i < size; i += 2)
    {
        if ((item = PyList_GetItem(list, i)) == NULL)
            return NULL;
        if ((str = PyString_AsString(item)) == NULL)
            return NULL;
        names << QString::fromUtf8(str);
        if ((item = PyList_GetItem(list, i+1)) == NULL)
            return NULL;
        if ((str = PyString_AsString(item)) == NULL)
            return NULL;
        urls << QString::fromUtf8(str);
    }

    if (danmaku_url && size > 2) //video clips with danmaku
    {
        DanmakuDelayGetter *get = new DanmakuDelayGetter(names, urls, danmaku_url);
        QObject::connect(get, &DanmakuDelayGetter::newPlay, playlist, &Playlist::addFileAndPlay);
        QObject::connect(get, &DanmakuDelayGetter::newFile, playlist, &Playlist::addFile);
    }
    else
    {
        playlist->addFileAndPlay(names.takeFirst(), urls.takeFirst(), danmaku_url); //first clip, maybe has danmaku
        while (!names.isEmpty())
            playlist->addFile(names.takeFirst(), urls.takeFirst());
    }
    if (Settings::autoCloseWindow)
        webvideo->close();
    Py_IncRef(Py_None);
    return Py_None;
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
        const char *name, *pic_url, *flag;
        if (NULL == (name_obj = PyDict_GetItemString(dict, "name")))
            return NULL;
        if (NULL == (flag_obj = PyDict_GetItemString(dict, "url")))
            return NULL;
        if (NULL == (pic_url_obj = PyDict_GetItemString(dict, "pic_url")))
            return NULL;
        if (NULL == (name = PyString_AsString(name_obj)))
            return NULL;
        if (NULL == (flag = PyString_AsString(flag_obj)))
            return NULL;
        if (NULL == (pic_url = PyString_AsString(pic_url_obj)))
            return NULL;
        res_library->addItem(QString::fromUtf8(name), pic_url, flag);
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
    {"get_url",     get_url,     METH_VARARGS, "Get url"},
    {"warn",        warn,        METH_VARARGS, "Show warning message"},
    {"question",    question,    METH_VARARGS, "Show a question dialog"},
    {"show_list",   show_list,   METH_VARARGS, "Show searching result on the list"},
    {"download",    download,    METH_VARARGS, "Download file"},
    {"play",        play,        METH_VARARGS, "Play online"},
    {"res_show",    res_show,    METH_VARARGS, "Show resources result"},
    {"show_detail", show_detail, METH_VARARGS, "Show detail"},
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
    PyModule_AddIntConstant(apiModule, "OPT_QL_HIGH",  OPT_QL_HIGH);
    PyModule_AddIntConstant(apiModule, "OPT_QL_SUPER", OPT_QL_SUPER);
    PyModule_AddIntConstant(apiModule, "OPT_DOWNLOAD",  OPT_DOWNLOAD);
    PyModule_AddStringConstant(apiModule, "final_url", "");
    Py_IncRef(exc_GetUrlError);
    PyModule_AddObject(apiModule, "GetUrlError", exc_GetUrlError);
}
