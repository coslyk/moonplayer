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

#include "plugin.h"
#include "jsapiObject.h"
#include "platform/paths.h"
#include "playlistModel.h"
#include <QDir>
#include <QFile>
#include <QJSEngine>
#include <QLocale>
#include <stdexcept>

// Load all plugins
QObjectList Plugin::loadPlugins()
{
    QObjectList result;
    QDir pluginsDir = QDir(userResourcesPath() + QStringLiteral("/plugins"));
    QStringList list = pluginsDir.entryList(QDir::Files, QDir::Name);

    while (!list.isEmpty())
    {
        QString filename = list.takeFirst();
        if (filename.endsWith(QStringLiteral(".js")))
        {
            result << new Plugin(pluginsDir.filePath(filename));
        }
    }
    return result;
}



Plugin::Plugin(const QString& filepath, QObject* parent) :
    QObject(parent),
    m_id(QFileInfo(filepath).baseName()),
    m_page(1)
{
    // install api
    JSAPIObject *apiObject = new JSAPIObject(m_id, this);
    connect(apiObject, &JSAPIObject::showResultRequested, this, &Plugin::updateResult);
    connect(apiObject, &JSAPIObject::jsError, this, &Plugin::printJSError);
    QJSValue api = m_engine.newQObject(apiObject);
    m_engine.globalObject().setProperty(QStringLiteral("moonplayer"), api);
    m_engine.installExtensions(QJSEngine::ConsoleExtension);
    
    // open file
    QFile file(filepath);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        throw std::runtime_error("Cannot init plugin!");
    }
    
    // evaluate
    m_script = file.readAll();
    file.close();
    QString content = QString::fromUtf8(m_script);
    QJSValue result = m_engine.evaluate(content, filepath);
    if (result.isError())
    {
        printJSError(result);
        throw std::runtime_error("Cannot init plugin!");
    }
    
    // get name
    m_name = m_engine.globalObject().property(QStringLiteral("website_name_") + QLocale::system().name()).toString();
    if (m_name == QStringLiteral("undefined"))
    {
        m_name = m_engine.globalObject().property(QStringLiteral("website_name")).toString();
    }

    // get description
    m_description = m_engine.globalObject().property(QStringLiteral("website_description_") + QLocale::system().name()).toString();
    if (m_description == QStringLiteral("undefined"))
    {
        m_description = m_engine.globalObject().property(QStringLiteral("website_description")).toString();
        if (m_description == QStringLiteral("undefined"))
        {
            m_description.clear();
        }
    }
    
    // get search() function
    m_searchFunc = m_engine.globalObject().property(QStringLiteral("search"));
}

// Set keyword
void Plugin::setKeyword(const QString& keyword)
{
    Q_ASSERT(!m_searchFunc.isNull());

    if (m_keyword == keyword)
        return;
    m_keyword = keyword;
    emit keywordChanged();
    
    if (!m_keyword.isEmpty())
    {
        m_page = 1;
        emit pageChanged();
        
        // Call search()
        QJSValueList args;
        args << m_keyword << m_page;
        QJSValue retVal = m_searchFunc.call(args);
        if (retVal.isError())
        {
            printJSError(retVal);
        }
    }
}

// Set page
void Plugin::setPage(int page)
{
    Q_ASSERT(!m_searchFunc.isNull());

    if (m_page == page)
        return;
    m_page = page;
    emit pageChanged();
    if (!m_keyword.isEmpty())
    {
        // Call search()
        QJSValueList args;
        args << m_keyword << m_page;
        QJSValue retVal = m_searchFunc.call(args);
        if (retVal.isError())
        {
            printJSError(retVal);
        }
    }
}


// Show result
void Plugin::updateResult(const QVariant& result)
{
    m_titles.clear();
    m_urls.clear();
    QVariantList list = result.toList();
    for (const auto& item : list)
    {
        m_titles << item.toHash()[QStringLiteral("title")].toString();
        m_urls << item.toHash()[QStringLiteral("url")].toUrl();
    }
    emit resultModelChanged();
}

// Open item
void Plugin::openItem(int index)
{
    Q_ASSERT(PlaylistModel::instance() != nullptr);
    PlaylistModel::instance()->addUrl(m_urls[index]);
}


void Plugin::printJSError(const QJSValue& errValue)
{
    QString filename = errValue.property(QStringLiteral("fileName")).toString();
    int lineNumber = errValue.property(QStringLiteral("lineNumber")).toInt();
    QByteArray line = m_script.split('\n')[lineNumber - 1];
    qDebug("In file \"%s\", line %i:\n  %s\n%s",
           filename.toUtf8().constData(),
           lineNumber,
           line.constData(),
           errValue.toString().toUtf8().constData());
}
