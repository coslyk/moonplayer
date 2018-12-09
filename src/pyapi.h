#ifndef PYAPI_H
#define PYAPI_H

/********************
 ** API for python **
 ********************/

#include <Python.h>
#include <QObject>

class QNetworkReply;
class QTimer;

//////define get_url function for Python
class GetUrl : public QObject
{
    Q_OBJECT
public:
    explicit GetUrl(QObject *parent = 0);
    void start(const char *url, PyObject *callback, PyObject *_data,
               const QByteArray &referer, const QByteArray &postData);
    inline bool hasTask(){return callbackFunc != NULL;}
private:
    PyObject *callbackFunc;
    PyObject *data;
    QNetworkReply *reply;
    QTimer *timer;
private slots:
    void onFinished(void);
    void onTimeOut(void);
};
extern GetUrl *geturl_obj;
extern PyObject *exc_GetUrlError;

///////Module
extern PyObject *apiModule;
void initAPI(void);
void call_py_func_vsi(PyObject *func, const char *first, int second);
QString fetchPythonException();
extern bool win_debug;

#endif // PYAPI_H
