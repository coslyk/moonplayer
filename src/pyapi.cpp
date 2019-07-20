#include "pyapi.h"
#include "accessmanager.h"
#include "platform/paths.h"
#include "reslibrary.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QApplication>
#include <QMessageBox>
#include <QTimer>

#ifdef MP_ENABLE_WEBENGINE
#include "parserwebcatch.h"
#endif


/*****************************************
 ******** Some useful functions **********
 ****************************************/
#define RETURN_IF_ERROR(retval)  if ((retval) == NULL){printPythonException(); return;}
#define EXIT_IF_ERROR(retval)    if ((retval) == NULL){printPythonException(); exit(EXIT_FAILURE);}


/**********************************************
 ** Define get_content() function for python **
 **********************************************/

static QNetworkReply *reply = NULL;

bool PluginIsLoadingPage()
{
    return reply != NULL;
}

static void get_post_content(const QUrl &url,
                             PyObject *callback,
                             PyObject *data,
                             const QByteArray &referer = QByteArray(),
                             const QByteArray &postData = QByteArray())
{
    // another task is running?
    if (reply)
    {
        QMessageBox::warning(NULL, "Error", "Another task is running, please wait");
        return;
    }

    // inc ref
    Py_IncRef(callback);
    Py_IncRef(data);

    // start request
    QNetworkRequest request(url);
    if (!referer.isEmpty())
        request.setRawHeader("Referer", referer);
    if (postData.isEmpty())
        reply = access_manager->get(request);
    else
    {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        reply = access_manager->post(request, postData);
    }

    // on reply
    QObject::connect(reply, &QNetworkReply::finished, [callback, data, referer, postData]() {
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        reply->deleteLater();

        // redirection
        if (status == 301 || status == 302)
        {
            QUrl final_url = QString::fromUtf8(reply->rawHeader("Location"));
            reply = NULL;
            get_post_content(final_url, callback, data, referer, postData);
        }

        // error
        else if (reply->error() != QNetworkReply::NoError)
        {
            QString errStr = QString("Network Error: %1\n%2").arg(QString::number(status), reply->errorString());
            QMessageBox::warning(NULL, "Error", errStr);
            reply = NULL;
        }

        // call callback function
        else
        {
            QByteArray barray = reply->readAll();
            reply = NULL;
            PyObject *retVal = PyObject_CallFunction(callback, "sO", barray.constData(), data);
            if (retVal == NULL)
                printPythonException();
            else
                Py_DecRef(retVal);

        }
        Py_DecRef(callback);
        Py_DecRef(data);
    });
}

static PyObject *get_content(PyObject *, PyObject *args)
{
    PyObject *callback, *data;
    const char *url, *referer = NULL;
    if (!PyArg_ParseTuple(args, "sOO|s", &url, &callback, &data, &referer))
        return NULL;
    get_post_content(QString::fromUtf8(url), callback, data, referer);
    Py_IncRef(Py_None);
    return Py_None;
}

static PyObject *post_content(PyObject *, PyObject *args)
{
    PyObject *callback, *data;
    const char *url, *postData, *referer = NULL;
    if (!PyArg_ParseTuple(args, "ssOO|s", &url, &postData, &callback, &data, &referer))
        return NULL;
    get_post_content(QString::fromUtf8(url), callback, data, referer, postData);
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


/*******************
 ** ResLibrary    **
 *******************/
static PyObject *res_show(PyObject *, PyObject *args)
{
    PyObject *obj = NULL;
    if (!PyArg_ParseTuple(args, "O", &obj))
        return NULL;
    if (!PyList_Check(obj))
        return NULL;
    res_library->clearItem();

    QVariantList list = PyObject_AsQVariant(obj).toList();
    foreach (QVariant i, list)
    {
        QVariantHash item = i.toHash();
        res_library->addItem(item["name"].toString(), item["pic_url"].toString(), item["url"].toString());
    }
    Py_IncRef(Py_None);
    return Py_None;
}

static PyObject *show_detail(PyObject *, PyObject *args)
{
    PyObject *dict = NULL;
    if (!PyArg_ParseTuple(args, "O", &dict))
        return NULL;
    if (!PyDict_Check(dict))
    {
        PyErr_SetString(PyExc_TypeError, "The argument is not a dict.");
        return NULL;
    }
    QVariantHash data = PyObject_AsQVariant(dict).toHash();
    res_library->openDetailPage(data);
    Py_IncRef(Py_None);
    return Py_None;
}

/*******************
 ** Video parsing **
 *******************/
static PyObject *finish_parsing(PyObject *, PyObject *args)
{
#ifdef MP_ENABLE_WEBENGINE
    PyObject *dict = NULL;
    if (!PyArg_ParseTuple(args, "O", &dict))
        return NULL;
    if (!PyDict_Check(dict))
    {
        PyErr_SetString(PyExc_TypeError, "The argument is not a dict.");
        return NULL;
    }
    QVariantHash data = PyObject_AsQVariant(dict).toHash();
    parser_webcatch->onParseFinished(data);
    Py_IncRef(Py_None);
    return Py_None;
#else
    PyErr_SetString(PyExc_RuntimeError, "The current MoonPlayer is not built with Webengine support!");
    return NULL;
#endif
}

/*******************
 ** Define module **
 *******************/

static PyMethodDef methods[] = {
    {"download_page",    get_content,      METH_VARARGS, "Send a HTTP-GET request (Obsolete method)"},
    {"get_content",      get_content,      METH_VARARGS, "Send a HTTP-GET request"},
    {"post_content",     post_content,     METH_VARARGS, "Send a HTTP-POST request"},
    {"bind_referer",     bind_referer,     METH_VARARGS, "Bind a host with referer"},
    {"force_unseekable", force_unseekable, METH_VARARGS, "Force stream with specific host to be unseekable"},
    {"finish_parsing",   finish_parsing,   METH_VARARGS, "Finish video parsing"},
    {"warn",             warn,             METH_VARARGS, "Show warning message"},
    {"question",         question,         METH_VARARGS, "Show a question dialog"},
    {"res_show",         res_show,         METH_VARARGS, "Show resources result"},
    {"show_detail",      show_detail,      METH_VARARGS, "Show detail"},
    {NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef moonplayerModule =
{
    PyModuleDef_HEAD_INIT,
    "moonplayer",  // m_name
    NULL,       // m_doc
    -1,            // m_size
    methods,       // m_methods
    NULL,       // m_slots
    NULL,       // m_traverse
    NULL,       // m_clear
    NULL        // m_free
};
#endif

PyObject *apiModule = NULL;

void initPython()
{
    //init python
    qputenv("PYTHONIOENCODING", "utf-8");
    Py_Initialize();
    if (!Py_IsInitialized())
    {
        qDebug("Cannot initialize python.");
        exit(-1);
    }

    //init module
#if PY_MAJOR_VERSION >= 3
    apiModule = PyModule_Create(&moonplayerModule);
    PyObject *modules = PySys_GetObject("modules");
    PyDict_SetItemString(modules, "moonplayer", apiModule);
#else
    apiModule = Py_InitModule("moonplayer", methods);
#endif

    // plugins' dir
    PyRun_SimpleString("import sys");
    PyRun_SimpleString(QString("sys.path.insert(0, '%1/plugins')").arg(getAppPath()).toUtf8().constData());
    PyRun_SimpleString(QString("sys.path.append('%1/plugins')").arg(getUserPath()).toUtf8().constData());
}
