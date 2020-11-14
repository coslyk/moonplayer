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

#include "parserYkdl.h"
#include "accessManager.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QProcess>
#include <QSettings>
#include <QTextCodec>
#include "platform/paths.h"
#include "dialogs.h"
#include "playlistModel.h"

ParserYkdl ParserYkdl::s_instance;

ParserYkdl::ParserYkdl(QObject *parent) : ParserBase(parent)
{
    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ParserYkdl::parseOutput);
    connect(&m_process, &QProcess::errorOccurred, [&](){ showErrorDialog(m_process.errorString()); });
}

ParserYkdl::~ParserYkdl()
{
    if (m_process.state() == QProcess::Running)
    {
        m_process.kill();
        m_process.waitForFinished();
    }
}

bool ParserYkdl::isSupported ( const QUrl& url )
{
    QString ykdlPath = userResourcesPath() + QStringLiteral("/ykdl-moonplayer");
    if (QFile::exists(ykdlPath))
    {
        QProcess process;
        process.start(ykdlPath, { QStringLiteral("--check-support"), url.toString() });
        process.waitForStarted(-1);
        process.waitForFinished(-1);
        return process.readAllStandardOutput().simplified() == "Url is supported.";
    } else {
        return false;
    }
}


void ParserYkdl::runParser(const QUrl &url)
{
    if (m_process.state() == QProcess::Running)
    {
        Dialogs::instance()->messageDialog(tr("Error"), tr("Another file is being parsed."));
        return;
    }

    QSettings settings;
    NetworkAccessManager::ProxyType proxyType = (NetworkAccessManager::ProxyType) settings.value(QStringLiteral("network/proxy_type")).toInt();
    QString proxy = settings.value(QStringLiteral("network/proxy")).toString();
    
    QStringList args;
    args << QStringLiteral("--timeout") << QStringLiteral("15") << QStringLiteral("--user-agent") << QStringLiteral(DEFAULT_UA);
    
    if (!proxy.isEmpty() && proxyType == NetworkAccessManager::HTTP_PROXY)
        args << QStringLiteral("--http-proxy") << proxy;
    else if (!proxy.isEmpty() && proxyType == NetworkAccessManager::SOCKS5_PROXY)
        args << QStringLiteral("--socks-proxy") << proxy;
    
    args << url.toString();
    m_process.start(userResourcesPath() + QStringLiteral("/ykdl-moonplayer"), args, QProcess::ReadOnly);
}


void ParserYkdl::parseOutput()
{
    QByteArray output = m_process.readAllStandardOutput();
#ifdef Q_OS_WIN
    output = QTextCodec::codecForLocale()->toUnicode(output).toUtf8();
#endif

    QJsonParseError json_error;
    QJsonDocument document = QJsonDocument::fromJson(output, &json_error);

    if (json_error.error != QJsonParseError::NoError)
    {
        showErrorDialog(QString::fromUtf8(m_process.readAllStandardError()));
        return;
    }

    // Select episode
    if (document.isArray())
    {
        QJsonArray episodes = document.array();
        QStringList titles;
        QList<QUrl> urls;

        for (const auto& item : episodes)
        {
            titles << item.toObject()[QStringLiteral("title")].toString();
            urls << item.toObject()[QStringLiteral("url")].toString();
        }

        Dialogs::instance()->selectionDialog(tr("Select episode"), titles, [=](int index) {
            Q_ASSERT(PlaylistModel::instance() != nullptr);
            PlaylistModel::instance()->addUrl(urls[index], m_download);
        });
        
        return;
    }

    // Video
    QJsonObject root = document.object();
    if (root.contains(QStringLiteral("streams")))
    {
        result.title = root[QStringLiteral("title")].toString();
        result.danmaku_url = root[QStringLiteral("danmaku_url")].toString();
        
        // get all available streams
        QJsonObject streams = root[QStringLiteral("streams")].toObject();
        for (auto i = streams.constBegin(); i != streams.constEnd(); i++)
        {
            QJsonObject item = i.value().toObject();
            
            // Basic stream infos
            Stream stream;
            stream.container = item[QStringLiteral("container")].toString();
            stream.referer = root[QStringLiteral("extra")].toObject()[QStringLiteral("referer")].toString();
            stream.ua = root[QStringLiteral("extra")].toObject()[QStringLiteral("ua")].toString();
            stream.is_dash = false;
            stream.seekable = true;
            
            // Write urls list
            QJsonArray urls = item[QStringLiteral("src")].toArray();
            if (urls.count() == 0)   // this stream is not available, skip it
            {
                continue;
            }

            for (const auto& url : urls)
            {
                stream.urls << QUrl(url.toString());
            }

            QString profile = item[QStringLiteral("video_profile")].toString();
            result.stream_types << QStringLiteral("%1 (%2)").arg(i.key(), profile);
            
            result.streams << stream;
        }
        finishParsing();
    }
    else
        showErrorDialog(QString::fromUtf8(m_process.readAllStandardError()));
}


