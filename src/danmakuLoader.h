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

#ifndef DANMAKULOADER_H
#define DANMAKULOADER_H

#include <QObject>
    class QNetworkReply;

class DanmakuLoader : public QObject
{
    Q_OBJECT
public:
    explicit DanmakuLoader(QObject *parent = nullptr);
    inline static DanmakuLoader* instance() { return &s_instance; }
    
    void start(const QUrl& srcUrl, int width, int height);

private slots:
    void onXmlDownloaded(void);

private:
    QNetworkReply *m_reply;
    int m_width;
    int m_height;
    
    static DanmakuLoader s_instance;
};

#endif // DANMAKULOADER_H
