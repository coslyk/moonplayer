/* Copyright 2013-2022 Yikun Liu <cos.lyk@gmail.com>
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

#ifndef WEBSITESETTINGS_H
#define WEBSITESETTINGS_H

#include <QMap>
#include <QObject>
#include <QUrl>
#include <QtQml/qqmlregistration.h>

// Store website settings
class WebsiteSettings : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QStringList websites READ getWebsites NOTIFY websitesChanged)

public:
    inline static WebsiteSettings* instance(void) { return s_instance; };
    
    WebsiteSettings(QObject* parent = nullptr);
    ~WebsiteSettings();

    void set(const QString& website, const QString& profile);
    QString get(const QString& website) const;
    Q_INVOKABLE void remove(const QString& website);

    inline QStringList getWebsites() const { return m_settings.keys(); }

signals:
    void websitesChanged(void);

private:
    QMap<QString, QString> m_settings;    // website, profile
    bool m_changed = false;

    static WebsiteSettings *s_instance;
};

#endif // WEBSITESETTINGS_H
