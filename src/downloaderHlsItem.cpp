#include "downloaderHlsItem.h"
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QProcess>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QSettings>
#include "accessManager.h"
#include "platform/paths.h"

DownloaderHlsItem::DownloaderHlsItem(const QString& filepath, const QUrl& url, const QUrl& danmakuUrl, QObject* parent) :
    DownloaderAbstractItem(filepath, danmakuUrl, parent), m_process(new QProcess(this))
{
    // Read proxy settings
    QSettings settings;
    auto proxyType = (NetworkAccessManager::ProxyType) settings.value(QStringLiteral("network/proxy_type")).toInt();
    auto proxy = settings.value(QStringLiteral("network/proxy")).toString();
    bool proxyOnlyForParsing = settings.value(QStringLiteral("network/proxy_only_for_parsing")).toBool();
    
    // Set new filePath
    QString newPath = filepath.section(QLatin1Char('.'), 0, -2) + QStringLiteral(".ts");
    setFilePath(newPath);
    
    QStringList args;
#ifndef Q_OS_WIN
    args << appResourcesPath() + QStringLiteral("/hls_downloader.py");
#endif
    if (proxyType == NetworkAccessManager::HTTP_PROXY && !proxy.isEmpty() && !proxyOnlyForParsing)
        args << QStringLiteral("--http-proxy") << proxy;
    else if (proxyType == NetworkAccessManager::SOCKS5_PROXY && !proxy.isEmpty() && !proxyOnlyForParsing)
        args << QStringLiteral("--socks-proxy") << proxy;
    args << QStringLiteral("--title") << name().section(QLatin1Char('.'), 0, -2) << url.toString();
    
    // Run
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &DownloaderHlsItem::onProcFinished);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &DownloaderHlsItem::readOutput);
    m_process->setWorkingDirectory(QFileInfo(newPath).absolutePath());
#ifdef Q_OS_WIN
    m_process->start(appResourcesPath() + QStringLiteral("/hls_downloader.exe"), args, QProcess::ReadOnly);
#else
    m_process->start(QStringLiteral("python"), args, QProcess::ReadOnly);
#endif
    setState(DOWNLOADING);
}

// Get progress
void DownloaderHlsItem::readOutput()
{
    static QRegularExpression re(QStringLiteral("\\((\\d+)/(\\d+)\\)"));
    while (m_process->canReadLine())
    {
        QByteArray line = m_process->readLine();
        QRegularExpressionMatch match = re.match(QString::fromLatin1(line));
        if (match.hasMatch())
        {
            int i = match.captured(1).toInt();
            int total = match.captured(2).toInt();
            setProgress(i * 100 / total);
        }
    }
}

// Start, pause, stop
void DownloaderHlsItem::start()
{
}

void DownloaderHlsItem::pause()
{
    QMessageBox::warning(nullptr, tr("Error"), tr("Cannot pause the download of HLS streams."));
}

void DownloaderHlsItem::stop(bool continueWaiting)
{
    Q_UNUSED(continueWaiting)
    if (m_process)
    {
        m_process->disconnect();
        if (m_process->state() == QProcess::Running)
            m_process->kill();
        m_process->deleteLater();
        m_process = nullptr;
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
        qDebug() << m_process->readAllStandardError();
        setState(ERROR);
    }
    else
    {
        if (!QFile::exists(filePath()))  // has been converted to mp4
        {
            QString newPath = filePath().section(QLatin1Char('.'), 0, -2) + QStringLiteral(".mp4");
            setFilePath(newPath);
        }
        setState(FINISHED);
    }
    m_process->deleteLater();
    m_process = nullptr;
}



