#ifndef JSAPI_OBJECT_H
#define JSAPI_OBJECT_H

#include <QObject>
#include <QJSValue>

/* API for plugins */
class JSAPIObject: public QObject {
    Q_OBJECT
public:
    JSAPIObject(const QString &id, QObject* parent);

    // Networking
    Q_INVOKABLE void get_content(const QString &url, const QJSValue &callbackFunc);
    Q_INVOKABLE void post_content(const QString &url, const QByteArray &postData, const QJSValue &callbackFunc);

    // Dialogs
    Q_INVOKABLE void warning(const QString &msg);
    Q_INVOKABLE bool question(const QString &msg);
    Q_INVOKABLE QString get_text(const QString &msg);
    Q_INVOKABLE QString get_item(const QString &msg, const QStringList &items);

    // Show result
    Q_INVOKABLE void show_result(const QVariant &result);

    // Configurations
    Q_INVOKABLE QVariant get_configuration(const QString &name);
    Q_INVOKABLE void set_configuration(const QString &name, const QVariant &value);
    
signals:
    void showResultRequested(const QVariant& result);
    void jsError(const QJSValue& error);
    
private:
    void get_post_content(const QString &url, const QByteArray &postData, const QJSValue &callbackFunc);
    QString m_id;
};

#endif
