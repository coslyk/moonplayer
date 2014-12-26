#include "httpget.h"
#include "accessmanager.h"
#include <QNetworkAccessManager>
#include <QApplication>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QDir>
#include <QTimer>
#include <iostream>

//start download task
HttpGet::HttpGet(const QUrl &url, const QString &filename, QObject *parent) :
    QObject(parent)
{
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
    reply = access_manager->get(request);
    connect(reply, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(reply, SIGNAL(finished()), this, SLOT(onFinished()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onProgressChanged(qint64,qint64)));
    emit progressChanged(this, prev_progress, true);
}

void HttpGet::onFinished()
{
    Q_ASSERT(reply);
    file->write(reply->readAll());

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

    else if (reply->error() != QNetworkReply::NoError) //has error or pause
    {
        QNetworkReply::NetworkError reason = reply->error();
        if (reason != QNetworkReply::OperationCanceledError)
            qDebug("Http status code: %d\n%s\n", status, reply->errorString().toUtf8().constData());
        last_finished = file->size();
        reply->deleteLater();
        reply = 0;
        is_paused = true;
        emit paused(this, (int) reason);
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
