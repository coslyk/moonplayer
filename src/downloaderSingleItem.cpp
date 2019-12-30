#include "downloaderSingleItem.h"
#include <QFileInfo>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include "accessManager.h"

int DownloaderSingleItem::s_numOfRunning = 0;
QList<DownloaderSingleItem*> DownloaderSingleItem::s_waitingItems;

DownloaderSingleItem::DownloaderSingleItem(const QString& filepath, const QUrl& url, const QUrl& danmakuUrl, QObject* parent) :
    DownloaderAbstractItem(filepath, danmakuUrl, parent), m_file(filepath), m_url(url), m_lastPos(0)
{
    // Open file
    m_reply = nullptr;
    if (!m_file.open(QFile::WriteOnly))
    {
        qDebug("Create file failed: %s", filepath.toUtf8().constData());
        setState(ERROR);
        return;
    }
    
    // Start downloading
    s_waitingItems << this;
    continueWaitingTasks();
}

void DownloaderSingleItem::continueWaitingTasks()
{
    int maxThreads = QSettings().value("downloader/max_threads", 5).toInt();
    while (s_numOfRunning < maxThreads && !s_waitingItems.isEmpty())
    {
        s_numOfRunning++;
        DownloaderSingleItem* item = s_waitingItems.takeFirst();
        item->setState(PAUSED);
        item->start();
    }
}



//start a request
void DownloaderSingleItem::start()
{
    if (state() != PAUSED)
        return;
    
    setState(DOWNLOADING);
    
    // Continue from the last position
    QNetworkRequest request(m_url);
    if (m_lastPos)
        request.setRawHeader("Range", "bytes=" + QByteArray::number(m_file.size()) + '-');
    
    // Start download
    m_reply = NetworkAccessManager::instance()->get(request);
    connect(m_reply, &QNetworkReply::readyRead, this, &DownloaderSingleItem::onReadyRead);
    connect(m_reply, &QNetworkReply::finished, this, &DownloaderSingleItem::onFinished);
    connect(m_reply, &QNetworkReply::downloadProgress, this, &DownloaderSingleItem::onDownloadProgressChanged);
}


// Pause
void DownloaderSingleItem::pause()
{
    if (state() == DOWNLOADING)
        m_reply->abort();
}


// Stop
void DownloaderSingleItem::stop(bool continueWaiting)
{
    if (state() != PAUSED && state() != DOWNLOADING && state() != WAITING)
        return;
    
    if (state() == DOWNLOADING)
    {
        m_reply->disconnect();
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }
    
    if (state() == WAITING)
    {
        s_waitingItems.removeOne(this);
    }
    else
    {
        s_numOfRunning--;
    }
    
    m_file.close();
    setState(CANCELED);
    
    if (continueWaiting)
        continueWaitingTasks();
}


void DownloaderSingleItem::onFinished()
{
    Q_ASSERT(m_reply);
    m_file.write(m_reply->readAll());

    // Redirect?
    int status = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status == 301 || status == 302) //redirect
    {
        m_reply->deleteLater();
        m_reply = nullptr;
        m_file.seek(0);
        m_lastPos = 0;
        m_url = QString::fromUtf8(m_reply->rawHeader("Location"));
        setState(PAUSED);
        start();
    }

    // Pause or error?
    else if (m_reply->error() != QNetworkReply::NoError)
    {
        QNetworkReply::NetworkError reason = m_reply->error();
        
        // Pause
        if (reason == QNetworkReply::OperationCanceledError)
        {
            setState(PAUSED);
            m_lastPos = m_file.size();
            m_reply->deleteLater();
            m_reply = nullptr;
        }
        
        // Error because remote server reject "Range" in http header
        // Can be solved by downloading from beginning again
        else if (reason == QNetworkReply::ContentOperationNotPermittedError)
        {
            // Remote server reject "Range" in http header
            setState(PAUSED);
            m_lastPos = 0;
            m_file.seek(0);
            m_reply->deleteLater();
            m_reply = nullptr;
        }
        
        // Error
        else
        {
            qDebug("Http status code: %d\n%s\n", status, m_reply->errorString().toUtf8().constData());
            m_reply->deleteLater();
            m_reply = nullptr;
            m_file.close();
            setState(ERROR);
            s_numOfRunning--;
            continueWaitingTasks();
        }
    }

    // Finished
    else
    {
        m_reply->deleteLater();
        m_reply = nullptr;
        m_file.close();
        setState(FINISHED);
        s_numOfRunning--;
        continueWaitingTasks();
    }
}


void DownloaderSingleItem::onReadyRead()
{
    m_file.write(m_reply->readAll());
}

void DownloaderSingleItem::onDownloadProgressChanged(qint64 received, qint64 total)
{
    bool is_percentage = (total > 0);
    int progress;
    if (is_percentage)  // Total size is known
        progress = (m_lastPos + received) * 100 / (m_lastPos + total);
    else
        progress = (received + m_lastPos) >> 20; //to MB
    setProgress(progress);
}
