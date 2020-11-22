/* Copyright 2013-2020 Yikun Liu <cos.lyk@gmail.com>
 *
 * This program is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see http://www.gnu.org/licenses/.
 */

#ifndef JSAPI_OBJECT_H
#define JSAPI_OBJECT_H

#include <QObject>
#include <QJSValue>

/* API for plugins */
class JSAPIObject : public QObject
{
    Q_OBJECT
public:
    JSAPIObject(const QString &id, QObject* parent);

    // Networking
    Q_INVOKABLE void get_content(const QString &url, const QJSValue &callbackFunc);
    Q_INVOKABLE void post_content(const QString &url, const QByteArray &postData, const QJSValue &callbackFunc);

    // Dialogs
    Q_INVOKABLE void warning(const QString &msg);
    Q_INVOKABLE void get_text(const QString &msg, const QString &defaultValue, const QJSValue &callbackFunc);

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
