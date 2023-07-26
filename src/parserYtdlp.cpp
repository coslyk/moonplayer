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

#include "parserYtdlp.h"
#include "accessManager.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QProcess>
#include <QSettings>
#include "dialogs.h"
#include "platform/paths.h"

ParserYtdlp ParserYtdlp::s_instance;

ParserYtdlp::ParserYtdlp(QObject *parent) : ParserBase(parent)
{
    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ParserYtdlp::parseOutput);
    connect(&m_process, &QProcess::errorOccurred, [&]()
            { showErrorDialog(m_process.errorString()); });
}

ParserYtdlp::~ParserYtdlp()
{
    if (m_process.state() == QProcess::Running)
    {
        m_process.kill();
        m_process.waitForFinished();
    }
}

void ParserYtdlp::runParser(const QUrl &url)
{
    if (m_process.state() == QProcess::Running)
    {
        Q_ASSERT(Dialogs::instance() != nullptr);
        Dialogs::instance()->messageDialog(tr("Error"), tr("Another file is being parsed."));
        return;
    }

    QSettings settings;
    auto proxyType = (NetworkAccessManager::ProxyType)settings.value(QStringLiteral("network/proxy_type")).toInt();
    auto proxy = settings.value(QStringLiteral("network/proxy")).toString();

    QStringList args;
    args << QStringLiteral("-j") << QStringLiteral("--user-agent") << QStringLiteral(DEFAULT_UA);
    if (!proxy.isEmpty() && proxyType == NetworkAccessManager::HTTP_PROXY)
    {
        args << QStringLiteral("--proxy") << proxy;
    }
    else if (!proxy.isEmpty() && proxyType == NetworkAccessManager::SOCKS5_PROXY)
    {
        args << QStringLiteral("--proxy") << QStringLiteral("socks5://%1/").arg(proxy);
    }
    args << url.toString();
    m_process.start(userResourcesPath() + QStringLiteral("/yt-dlp"), args, QProcess::ReadOnly);
}

void ParserYtdlp::parseOutput()
{
    QByteArray output = m_process.readAllStandardOutput();
#ifdef Q_OS_WIN
    output = QString::fromLocal8Bit(output).toUtf8();
#endif

    QJsonParseError json_error;
    QJsonObject root = QJsonDocument::fromJson(output, &json_error).object();
    if (json_error.error != QJsonParseError::NoError)
    {
        showErrorDialog(QString::fromUtf8(m_process.readAllStandardError()));
        return;
    }

    if (!root.contains(QStringLiteral("formats")))
    {
        showErrorDialog(QString::fromUtf8(m_process.readAllStandardError()));
        return;
    }
    result.title = root[QStringLiteral("title")].toString();

    // Get all available videos
    QJsonArray formats = root[QStringLiteral("formats")].toArray();
    QJsonArray videos;
    Stream bestMp4Audio, bestWebmAudio;
    int bestMp4AudioAsr = 0;
    int bestWebmAudioAsr = 0;

    for (const auto &format : formats)
    {
        QJsonObject item = format.toObject();

        // DASH Audio
        if (item[QStringLiteral("vcodec")].toString() == QStringLiteral("none"))
        {
            if (item[QStringLiteral("ext")].toString() == QStringLiteral("webm") && item[QStringLiteral("asr")].toInt() > bestWebmAudioAsr)
            {
                convertToStream(item, bestWebmAudio);
                bestWebmAudioAsr = item[QStringLiteral("asr")].toInt();
            }
            else if (item[QStringLiteral("ext")].toString() == QStringLiteral("m4a") && item[QStringLiteral("asr")].toInt() > bestMp4AudioAsr)
            {
                convertToStream(item, bestMp4Audio);
                bestMp4AudioAsr = item[QStringLiteral("asr")].toInt();
            }
        }
        // Videos
        else
        {
            QString formatName = QStringLiteral("%1 (%2)").arg(item[QStringLiteral("format")].toString(), item[QStringLiteral("ext")].toString());
            result.stream_types << formatName;
            videos << item;
        }
    }

    // Fill stream infos
    for (const auto &video : videos)
    {
        QJsonObject item = video.toObject();
        Stream stream;
        convertToStream(item, stream);

        // Video has no audio track? => Dash video, audio in seperate file
        if (item[QStringLiteral("acodec")] == QStringLiteral("none"))
        {
            if (stream.container == QStringLiteral("webm") && bestWebmAudioAsr > 0)
            {
                stream.is_dash = true;
                stream.urls << bestWebmAudio.urls[0];
            }
            else if (stream.container == QStringLiteral("mp4") && bestWebmAudioAsr > 0)
            {
                stream.is_dash = true;
                stream.urls << bestMp4Audio.urls[0];
            }
            else
            {
                continue;
            }
        }
        result.streams << stream;
    }

    // Add audio only options
    if (bestWebmAudioAsr > 0)
    {
        result.stream_types << tr("Audio only (webm)");
        result.streams << bestWebmAudio;
    }
    if (bestMp4AudioAsr > 0)
    {
        result.stream_types << tr("Audio only (m4a)");
        result.streams << bestMp4Audio;
    }

    finishParsing();
}

void ParserYtdlp::convertToStream(const QJsonObject &item, Stream &stream)
{
    // Basic stream infos
    stream.container = item[QStringLiteral("protocol")].toString() == QStringLiteral("m3u8") ? QStringLiteral("m3u8") : item[QStringLiteral("ext")].toString();
    stream.referer = item[QStringLiteral("http_headers")].toObject()[QStringLiteral("Referer")].toString();
    stream.seekable = true;
    stream.is_dash = false;

    QString ua = item[QStringLiteral("http_headers")].toObject()[QStringLiteral("User-Agent")].toString();
    if (ua != QStringLiteral(DEFAULT_UA))
    {
        stream.ua = ua;
    }
    // Urls
    stream.urls.clear();
    stream.urls << item[QStringLiteral("url")].toString();
}
