#include "videocombiner.h"
#include <QMessageBox>
#include "platforms.h"

VideoCombiner::VideoCombiner(QObject *parent, const QDir &dir) :
    QProcess(parent)
{
    // Write list file
    QStringList filelist = dir.entryList(QDir::Files, QDir::Name);
    QFile file(dir.filePath("filelist.txt"));
    if (!file.open(QFile::WriteOnly))
    {
        deleteLater();
        return;
    }
    foreach (QString filename, filelist)
    {
        if (!filename.endsWith(".danmaku"))
            file.write(QString("file '%1'\n").arg(filename).toUtf8());
    }
    file.close();
    // Run FFMPEG
    this->dir = dir;
    save_as = dir.absolutePath().section('.', 0, -2) + "(combine)." + dir.absolutePath().section('.', -1);
    QStringList args;
    args << "-f" << "concat" << "-safe" << "0" << "-i" << "filelist.txt" << "-c" << "copy" << save_as;
    setWorkingDirectory(dir.absolutePath());
    connect(this, SIGNAL(finished(int)), this, SLOT(onFinished(int)));
    start(ffmpegFilePath(), args, QProcess::ReadOnly);
}

void VideoCombiner::onFinished(int status)
{
    if (status == 0)
    {
        QMessageBox::information(NULL, "Information", tr("Finished combining:") + save_as);
        // Copy .danmaku file
        QStringList nameFilter;
        nameFilter << "*.danmaku";
        QStringList danmakuFiles = dir.entryList(nameFilter, QDir::Files, QDir::Name);
        if (!danmakuFiles.isEmpty())
            QFile::copy(danmakuFiles[0], save_as + ".danmaku");
    }
    else
    {
        QMessageBox::warning(NULL, "Error", tr("Failed to combine:") + save_as);
        qDebug("FFmpeg ERROR:\n%s", readAllStandardError().constData());
    }
    deleteLater();
}
