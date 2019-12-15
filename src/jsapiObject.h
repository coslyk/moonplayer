#ifndef JSAPI_OBJECT_H
#define JSAPI_OBJECT_H

#include <QObject>
#include <QJSValue>

/* API for plugins */
class JSAPIObject: public QObject {
    Q_OBJECT
public:
    JSAPIObject(QObject* parent);
    Q_INVOKABLE void get_content(const QString &url, const QJSValue &callbackFunc);
    Q_INVOKABLE void post_content(const QString &url, const QByteArray &postData, const QJSValue &callbackFunc);
    Q_INVOKABLE void warning(const QString &msg);
    Q_INVOKABLE bool question(const QString &msg);
    Q_INVOKABLE void show_result(const QVariant &result);
    
signals:
    void showResultRequested(const QVariant& result);
    void jsError(const QJSValue& error);
    
private:
    void get_post_content(const QString &url, const QByteArray &postData, const QJSValue &callbackFunc);
};

#endif
