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

#include "downloaderHlsItem.h"
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSettings>
#include "accessManager.h"
#include "dialogs.h"
#include "platform/paths.h"

DownloaderHlsItem::DownloaderHlsItem(const QString &filepath, const QUrl &url, const QUrl &danmakuUrl, QObject *parent) : DownloaderAbstractItem(filepath, danmakuUrl, parent)
{
    // Read proxy settings
    QSettings settings;
    auto proxyType = (NetworkAccessManager::ProxyType) settings.value(QStringLiteral("network/proxy_type")).toInt();
    auto proxy = settings.value(QStringLiteral("network/proxy")).toString();
    bool proxyOnlyForParsing = settings.value(QStringLiteral("network/proxy_only_for_parsing")).toBool();
    
    // Set new filePath
    QString newPath = filepath.section(QLatin1Char('.'), 0, -2) + QStringLiteral(".ts");
    setFilePath(newPath);
    setName(QFileInfo(newPath).fileName());

    // Delete file if it exists
    if (QFile::exists(newPath))
    {
        QFile::remove(newPath);
    }
    
    QStringList args;

    // Choose best quality
    args << QStringLiteral("-b");

    // Set proxy
    if (proxyType == NetworkAccessManager::HTTP_PROXY && !proxy.isEmpty() && !proxyOnlyForParsing)
    {
        args << QStringLiteral("-p") << (QStringLiteral("http://") + proxy);
    }
    else if (proxyType == NetworkAccessManager::SOCKS5_PROXY && !proxy.isEmpty() && !proxyOnlyForParsing)
    {
        args << QStringLiteral("-p") << (QStringLiteral("socks5://") + proxy);
    }

    // Output file
    args << QStringLiteral("-o") << (name().section(QLatin1Char('.'), 0, -2) + QStringLiteral(".ts"));

    // Url of m3u8 file
    args << url.toString();
    
    // Run
    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &DownloaderHlsItem::onProcFinished);
    connect(&m_process, &QProcess::readyReadStandardOutput, this, &DownloaderHlsItem::readOutput);
    connect(&m_process, &QProcess::errorOccurred, [&]() {
        qDebug() <<"Fails to run hlsdl!\n" << m_process.errorString();
    });
    
    m_process.setWorkingDirectory(QFileInfo(newPath).absolutePath());
    m_process.setProcessChannelMode(QProcess::MergedChannels);
    m_process.start(hlsdlFilePath(), args, QProcess::ReadOnly);
    setState(DOWNLOADING);
}

// Get progress
void DownloaderHlsItem::readOutput()
{
    while (m_process.canReadLine())
    {
        QByteArray line = m_process.readLine();
        QJsonDocument info = QJsonDocument::fromJson(line);
        if (info.isObject())
        {
            QJsonObject obj = info.object();
            int i = obj[QStringLiteral("d_d")].toInt();
            int total = obj[QStringLiteral("t_d")].toInt();
            if (total)
            {
                setProgress(i * 100 / total);
            }
        }
    }
}

// Start, pause, stop
void DownloaderHlsItem::start()
{
}

void DownloaderHlsItem::pause()
{
    Q_ASSERT(Dialogs::instance() != nullptr);
    Dialogs::instance()->messageDialog(tr("Error"), tr("Cannot pause the download of HLS streams."));
}

void DownloaderHlsItem::stop()
{
    if (m_process.state() == QProcess::Running)
    {
        m_process.kill();
        setState(CANCELED);
    }
}

DownloaderHlsItem::~DownloaderHlsItem()
{
    stop();
}

void DownloaderHlsItem::onProcFinished(int code)
{
    if (code) // Error
    {
        qDebug() << m_process.readAllStandardError();
        setState(ERROR);
    }
    else
    {
        // Convert file to mp4
        // First, make a ffmpeg dry run to check the audio format
        QProcess proc;
        QStringList args;
        args << QStringLiteral("-y") << QStringLiteral("-i") << filePath();
        proc.start(ffmpegFilePath(), args, QProcess::ReadOnly);
        proc.waitForFinished();
        QString output = QString::fromUtf8(proc.readAllStandardError());

        static QRegularExpression re(QStringLiteral(R"delimiter(Stream\s*#\d+:\d+(?:\[0x[0-9a-f]+\])?(?:\([a-z]{3}\))?:\s*Audio:\s*([0-9a-z]+))delimiter"));
        QRegularExpressionMatch match = re.match(output);
        QString audioFormat = match.captured(1);

        // Then, convert to mp4
        QString newPath = filePath().chopped(3) + QStringLiteral(".mp4");
        args << QStringLiteral("-c") << QStringLiteral("copy") << QStringLiteral("-f") << QStringLiteral("mp4");
        if (audioFormat == QStringLiteral("aac"))
        {
            args << QStringLiteral("-bsf:a") << QStringLiteral("aac_adtstoasc");
        }
        args << newPath;
        proc.start(ffmpegFilePath(), args, QProcess::ReadOnly);
        proc.waitForFinished();

        if (QFile::exists(newPath))  // has been converted to mp4
        {
            // Delete original .ts file
            QFile::remove(filePath());
            setFilePath(newPath);
        }
        setState(FINISHED);
    }
}



