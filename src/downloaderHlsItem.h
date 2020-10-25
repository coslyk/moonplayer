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

#ifndef DOWNLAODERHLSITEM_H
#define DOWNLAODERHLSITEM_H

#include "downloaderAbstractItem.h"
#include <QProcess>

    class DownloaderHlsItem : public DownloaderAbstractItem
{
    Q_OBJECT
    
public:
    DownloaderHlsItem(const QString& filepath, const QUrl& url, const QUrl& danmakuUrl = QUrl(), QObject* parent = nullptr);
    ~DownloaderHlsItem();
    virtual void pause(void) override;
    virtual void start(void) override;
    virtual void stop(void) override;

private:
    QProcess m_process;

private slots:
    void readOutput(void);
    void onProcFinished(int code);
};

#endif // DOWNLAODERHLSITEM_H
