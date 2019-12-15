#include "downloaderMultipleItem.h"
#include "downloaderSingleItem.h"
#include <QDebug>
#include <QMessageBox>
#include <QProcess>
#include "platform/paths.h"

DownloaderMultipleItem::DownloaderMultipleItem(const QString& filepath, const QList<QUrl>& urls, bool isDash, QObject* parent) :
    DownloaderAbstractItem(filepath, parent),
    m_finished(0),
    m_total(urls.length()),
    m_isDash(isDash),
    m_process(nullptr)
{
    // Create tempdir
    QString tempPath = QDir::tempPath() + '/'  + name();
    m_tempDir = QDir(tempPath);
    if (!m_tempDir.exists())
    {
        m_tempDir.mkpath(tempPath);
    }
    
    // Start task
    QString fileSuffix = filepath.section('.', -1);
    for (int i = 0; i < m_total; i++)
    {
        QString itemFilePath = m_tempDir.filePath(QString::number(i).rightJustified(3, '0')) + '.' + fileSuffix;
        DownloaderSingleItem* item = new DownloaderSingleItem(itemFilePath, urls[i], this);
        m_items << item;
        connect(item, &DownloaderSingleItem::stateChanged, [=](){ this->onItemStateChanged(item); });
        connect(item, &DownloaderSingleItem::progressChanged, this, &DownloaderMultipleItem::onItemProgressChanged);
    }
    setState(DOWNLOADING);
}

// Start
void DownloaderMultipleItem::start()
{
    if (state() != PAUSED)
        return;
    
    foreach (DownloaderSingleItem* item, m_items)
    {
        item->start();
    }
    setState(DOWNLOADING);
}

// Pause
void DownloaderMultipleItem::pause()
{
    if (state() != DOWNLOADING)
        return;
    
    foreach (DownloaderSingleItem* item, m_items)
    {
        item->pause();
    }
    setState(PAUSED);
}

// Stop
void DownloaderMultipleItem::stop(bool continueWaiting)
{
    Q_UNUSED(continueWaiting);
    
    if (state() != DOWNLOADING && state() != PAUSED)
        return;
    
    while (m_items.length() > 1)
    {
        DownloaderSingleItem* item = m_items.takeFirst();
        item->disconnect();
        item->stop(false);  // Do not continue waitings
    }
    if (m_items.length() == 1)
    {
        DownloaderSingleItem* item = m_items.takeFirst();
        item->disconnect();
        item->stop();  // Continue waitings
    }
    setState(CANCELED);
}


// Update progress
void DownloaderMultipleItem::onItemProgressChanged()
{
    int progress = 0;
    foreach (DownloaderSingleItem* item, m_items)
    {
        progress += item->progress();
    }
    progress /= m_total;
    setProgress(progress);
}


// Update state
void DownloaderMultipleItem::onItemStateChanged(DownloaderSingleItem* item)
{
    switch (item->state())
    {
        case ERROR:
            stop();  // Stop other items
            setState(ERROR);
            break;
            
        case FINISHED:
            m_finished++;
            if (m_finished == m_total)
            {
                concatVideos();
            }
            break;
            
        default:
            break;
    }
}


// Concat videos
void DownloaderMultipleItem::concatVideos()
{
    QStringList filelist = m_tempDir.entryList(QDir::Files, QDir::Name);
    QStringList args;
    
    // Youtube's dash videos?
    if (m_isDash)
    {
        args << "-y" << "-i" << filelist[0] << "-i" << filelist[1] << "-c:v" << "copy";
        if (filelist[0].endsWith(".mp4"))
            args << "-c:a" << "aac";
        else if (filelist[0].endsWith(".webm"))
            args << "-c:a" << "vorbis";
        args << "-strict" << "experimental" << filePath();
    }

    // Video clips
    else
    {
        // Write list file
        QFile file(m_tempDir.filePath("filelist.txt"));
        if (!file.open(QFile::WriteOnly))
        {
            setState(ERROR);
            QMessageBox::warning(nullptr, "Error", tr("Failed to write: ") + file.fileName());
            return;
        }
        foreach (QString filename, filelist)
        {
            file.write(QString("file '%1'\n").arg(filename).toUtf8());
        }
        file.close();
    
        // Set arguments
        args << "-y" << "-f" << "concat" << "-safe" << "0" << "-i" << "filelist.txt" << "-c" << "copy" << filePath();
    }

    // Run FFMPEG
    m_process = new QProcess(this);
    m_process->setWorkingDirectory(m_tempDir.absolutePath());
    connect(m_process, SIGNAL(finished(int)), this, SLOT(onConcatFinished(int)));
    connect(m_process, &QProcess::errorOccurred, [=]() { qDebug() << "FFMpeg ERROR: " << m_process->errorString(); });
    m_process->start(ffmpegFilePath(), args, QProcess::ReadOnly);
}

void DownloaderMultipleItem::onConcatFinished(int status)
{
    if (status == 0)  // Success, remove temp file
    {
        setState(FINISHED);
        QStringList filelist = m_tempDir.entryList(QDir::Files, QDir::Name);
        foreach (QString file, filelist)
        {
            m_tempDir.remove(file);
        }
        m_tempDir.cdUp();
        m_tempDir.rmdir(name());
    }
    else
    {
        setState(ERROR);
        QMessageBox::warning(nullptr, "Error", tr("Failed to concat: ") + filePath());
        qDebug("FFmpeg ERROR:\n%s", m_process->readAllStandardError().constData());
    }
    m_process->deleteLater();
    m_process = nullptr;
}

