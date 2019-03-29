#include "videocombiner.h"
#include <QMessageBox>
#include "platform/paths.h"
#include "playlist.h"

VideoCombiner::VideoCombiner(QObject *parent, const QDir &dir) :
    QProcess(parent)
{
    QStringList filelist = dir.entryList(QDir::Files, QDir::Name);
    this->dir = dir;
    QString ext = dir.absolutePath().section('.', -1);
    save_as = dir.absolutePath().section('.', 0, -2) + "(combine)." + ext;
    QStringList args;


    // this is a dash stream from youtube
    if (filelist[0].startsWith("audio.") && filelist[1].startsWith("video."))
    {
        args << "-i" << filelist[1] << "-i" << filelist[0] << "-c:v" << "copy";
        if (ext == "mp4")
            args << "-c:a" << "aac";
        else if (ext == "webm")
            args << "-c:a" << "vorbis";
        args << "-strict" << "experimental" << save_as;
    }

    else
    {
        // Write list file
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
        // Set arguments
        args << "-f" << "concat" << "-safe" << "0" << "-i" << "filelist.txt" << "-c" << "copy" << save_as;
    }

    // Run FFMPEG
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
        QString newDanmakuFile;
        if (!danmakuFiles.isEmpty())
        {
            newDanmakuFile = save_as + ".danmaku";
            QFile::copy(dir.filePath(danmakuFiles[0]), newDanmakuFile);
        }
        playlist->addFile(QFileInfo(save_as).fileName(), save_as, newDanmakuFile);
    }
    else
    {
        QMessageBox::warning(NULL, "Error", tr("Failed to combine:") + save_as);
        qDebug("FFmpeg ERROR:\n%s", readAllStandardError().constData());
    }
    deleteLater();
}
