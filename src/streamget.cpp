#include "streamget.h"
#include <QProcess>
#include <QUrl>
#include "platforms.h"

StreamGet::StreamGet(const QUrl &url, const QString &filename, QObject *parent) :
    DownloaderItem (filename, parent)
{
    args << "-y" << "-i" << url.toString() << "-c" << "copy" << "-bsf:a" << "aac_adtstoasc" << filename;
    process = NULL;
}

void StreamGet::start()
{
    process = new QProcess(this);
    connect(process, SIGNAL(finished(int)), this, SLOT(onProcFinished(int)));
    process->start(ffmpegFilePath(), args);
    emit progressChanged(0, true);
}

void StreamGet::pause()
{

}

void StreamGet::stop()
{
    if (process)
    {
        process->write("q");
        process->waitForFinished(1000);
        if (process->state() == QProcess::Running)
            process->kill();
    }
    process->deleteLater();
    process = NULL;
}

StreamGet::~StreamGet()
{
    stop();
}

void StreamGet::onProcFinished(int code)
{
    emit finished(this, code);
    process->deleteLater();
    process = NULL;
}
