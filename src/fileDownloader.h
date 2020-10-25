/* Copyright 2013-2020 Yikun Liu <cos.lyk@gmail.com>
 *
 * This program is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see http://www.gnu.org/licenses/.
 */

#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QFile>
#include <QUrl>

    class QNetworkReply;

class FileDownloader : public QObject
{
    Q_OBJECT

signals:
    void started();
    void paused();
    void stopped();
    void finished();
    void progressChanged(int progress);
    
public:
    FileDownloader(const QString &filepath, const QUrl &url, QObject *parent = nullptr);
    virtual ~FileDownloader();
    void pause(void);
    void start(void);
    void stop(void);
    inline int progress() const { return m_progress; }

private:
    QNetworkReply* m_reply;
    QFile m_file;
    QUrl m_url;
    qint64 m_lastPos;
    int m_progress;
    
private slots:
    void onFinished(void);
    void onReadyRead(void);
    void onDownloadProgressChanged(qint64 received, qint64 total);
};

#endif // FILEDOWNLOADER_H
