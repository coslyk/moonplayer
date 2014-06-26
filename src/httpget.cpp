#include "httpget.h"
#include <QNetworkAccessManager>
#include <QApplication>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QNetworkRequest>
#include <QFile>
#include <QDir>
#include <QTimer>
#include <iostream>

QNetworkAccessManager* HttpGet::manager = 0;

void HttpGet::setProxy(const QString &proxy, int port)
{
    if (manager == 0)
        manager = new QNetworkAccessManager(qApp);
    if (proxy.isEmpty())
        manager->setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
    else
        manager->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, proxy, port));
}

//start download task
HttpGet::HttpGet(const QUrl &url, const QString &filename, QObject *parent) :
    QObject(parent)
{
    //Create access manager
    if (manager == 0)
        manager = new QNetworkAccessManager(qApp);

    //open file
    last_finished = 0;
    prev_progress = 0;
    reply = 0;
    is_paused = true;
    name = filename;
    file = new QFile(filename);
    if (!file->open(QFile::WriteOnly))
    {
        emit finished(this, true);
        delete file;
        deleteLater();
        return;
    }
    //Set url
    this->url = url;
}

//start a request
void HttpGet::start()
{
    is_paused = false;
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "moonplayer");
    if (last_finished)
        request.setRawHeader("Range", "bytes=" + QByteArray::number(file->size()) + '-');
    reply = manager->get(request);
    connect(reply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(reply, SIGNAL(finished()), this, SLOT(onFinished()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onProgressChanged(qint64,qint64)));
    emit progressChanged(this, prev_progress, true);
}

void HttpGet::onFinished()
{
    Q_ASSERT(reply);
    file->write(reply->readAll());
    if (reply->error() != QNetworkReply::NoError) //has error or pause
    {
        last_finished = file->size();
        reply->deleteLater();
        reply = 0;
        is_paused = true;
        emit paused(this);
    }

    else
    {
        //check redirect
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (status == 301 || status == 302) //redirect
        {
            reply->deleteLater();
            file->seek(0);
            last_finished = 0;
            url = QString::fromUtf8(reply->rawHeader("Location"));
            start();
        }
        else  //finished
        {
            reply->deleteLater();
            reply = 0;
            emit finished(this, false);
            deleteLater(); //task end, delete self
            //close file
            file->close();
            delete file;
            file = 0;
        }
    }
}

void HttpGet::stop()
{
    disconnect(SIGNAL(finished(HttpGet*, bool)));
    if (!is_paused)
    {
        reply->disconnect();
        reply->abort();
        reply->deleteLater();
        reply = 0;
    }
    file->close();
    file->deleteLater();
    file = 0;
    deleteLater(); //task end, delete self
}

void HttpGet::onReadyRead()
{
    file->write(reply->readAll());
}

void HttpGet::onProgressChanged(qint64 received, qint64 total)
{
    bool is_percentage = (total > 0);
    int progress;
    if (is_percentage)  //Total size is known
        progress = (last_finished + received) * 100 / (last_finished + total);
    else
        progress = (received + last_finished) >> 20; //to MB
    if (progress != prev_progress)
    {
        prev_progress = progress;
        emit progressChanged(this, progress, is_percentage);
    }
}

void HttpGet::pause()
{
    if (is_paused)
        start();
    else
        reply->abort();
}
