#include "downloaderHlsItem.h"
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
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
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &DownloaderHlsItem::onProcFinished);
    connect(m_process, &QProcess::readyReadStandardOutput, this, &DownloaderHlsItem::readOutput);
    m_process->setWorkingDirectory(QFileInfo(newPath).absolutePath());
    m_process->setReadChannelMode(QProcess::MergedChannels);
    m_process->start(QStringLiteral("moonplayer-hlsdl"), args, QProcess::ReadOnly);
    setState(DOWNLOADING);
}

// Get progress
void DownloaderHlsItem::readOutput()
{
    while (m_process->canReadLine())
    {
        QByteArray line = m_process->readLine();
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
        // Convert file to mp4
        // First, make a ffmpeg dry run to check the audio format
        QProcess proc;
        QStringList args;
        args << QStringLiteral("-y") << QStringLiteral("-i") << filePath();
        proc.start(QStringLiteral("ffmpeg"), args, QProcess::ReadOnly);
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
        proc.start(QStringLiteral("ffmpeg"), args, QProcess::ReadOnly);
        proc.waitForFinished();

        if (QFile::exists(newPath))  // has been converted to mp4
        {
            // Delete original .ts file
            QFile::remove(filePath());
            setFilePath(newPath);
        }
        setState(FINISHED);
    }
    m_process->deleteLater();
    m_process = nullptr;
}



