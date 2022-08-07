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

#include "websiteSettings.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include "platform/paths.h"

WebsiteSettings* WebsiteSettings::s_instance = nullptr;

WebsiteSettings::WebsiteSettings(QObject* parent) :
    QObject(parent)
{
    Q_ASSERT(s_instance == nullptr);
    s_instance = this;

    // Load stored settings
    QFile file(QDir(userResourcesPath()).filePath(QStringLiteral("website_settings.json")));
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        return;
    }
    QJsonParseError jsonErr;
    QJsonDocument json = QJsonDocument::fromJson(file.readAll(), &jsonErr);
    if (jsonErr.error != QJsonParseError::NoError) {
        file.close();
        return;
    }
    QJsonObject jsonObj = json.object();
    for (const auto& website : jsonObj.keys()) {
        m_settings[website] = jsonObj[website].toString();
    }
}


WebsiteSettings::~WebsiteSettings()
{
    // Store website setting if changed
    if (!m_changed)
    {
        return;
    }
    QFile file(QDir(userResourcesPath()).filePath(QStringLiteral("website_settings.json")));
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        qWarning("Failed to save website settings.");
        return;
    }
    QJsonObject json;
    for (const auto& website : m_settings.keys())
    {
        json[website] = m_settings[website];
    }
    QJsonDocument doc(json);
    file.write(doc.toJson());
    file.close();
}


void WebsiteSettings::set(const QString& website, const QString& profile)
{
    if (m_settings.value(website) != profile)
    {
        m_settings[website] = profile;
        m_changed = true;
        emit websitesChanged();
    }
}

QString WebsiteSettings::get(const QString& website) const
{
    return m_settings.value(website);
}

void WebsiteSettings::remove(const QString& website)
{
    if (m_settings.contains(website))
    {
        m_settings.remove(website);
        m_changed = true;
        emit websitesChanged();
    }
}
