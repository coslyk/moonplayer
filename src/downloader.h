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

#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QUrl>

    class Downloader : public QObject
{
    Q_OBJECT

public:
    static Downloader* instance(void);
    Downloader(QObject* parent = nullptr);
    ~Downloader();
    
    void addTasks(const QString& name, const QList<QUrl>& urls, const QUrl& danmakuUrl = QUrl(), bool isDash = false);
    QObjectList model(void);
    
signals:
    void modelUpdated(void);
    
private:
    QObjectList m_model;
};

#endif // DOWNLOADER_H
