#include "streamget.h"
#include <QMessageBox>
#include <QProcess>
#include <QUrl>
#include "platform/paths.h"
#include "python_wrapper.h"
#include "settings_network.h"

StreamGet::StreamGet(const QUrl &url, const QString &filename, QObject *parent) :
    DownloaderItem (filename, parent)
{
    args << getAppPath() + "/plugins/hls_downloader.py";
    if (Settings::proxyType == "http" && !Settings::proxy.isEmpty() && !Settings::proxyOnlyForParsing)
        args << "--http-proxy" << (Settings::proxy + ':' + QString::number(Settings::port));
    else if (Settings::proxyType == "socks5" && !Settings::proxy.isEmpty() && !Settings::proxyOnlyForParsing)
        args << "--socks-proxy" << (Settings::proxy + ':' + QString::number(Settings::port));
    args << "--title" << filename.section('.', 0, -2) << url.toString();
    process = NULL;
}

void StreamGet::start()
{
    process = new QProcess(this);
    connect(process, SIGNAL(finished(int)), this, SLOT(onProcFinished(int)));
    connect(process, &QProcess::readyReadStandardOutput, this, &StreamGet::readOutput);
    process->start(PYTHON_BIN, args);
    emit progressChanged(0, true);
}

void StreamGet::readOutput()
{
    static QRegularExpression re("\\((\\d+)/(\\d+)\\)");
    while (process->canReadLine())
    {
        QByteArray line = process->readLine();
        QRegularExpressionMatch match = re.match(line);
        if (match.hasMatch())
        {
            int i = match.captured(1).toInt();
            int total = match.captured(2).toInt();
            emit progressChanged(i * 100 / total, true);
        }
    }
}

void StreamGet::pause()
{
    QMessageBox::warning(NULL, "Error", tr("Cannot pause the download of stream medias"));
}

void StreamGet::stop()
{
    if (process)
    {
        if (process->state() == QProcess::Running)
            process->kill();
        process->deleteLater();
        process = NULL;
    }
}

StreamGet::~StreamGet()
{
    stop();
}

void StreamGet::onProcFinished(int code)
{
    if (code) // Error
    {
        QByteArray errOutput = process->readAllStandardError();
        qDebug("%s", errOutput.constData());
    }
    emit finished(this, code);
    process->deleteLater();
    process = NULL;

}
