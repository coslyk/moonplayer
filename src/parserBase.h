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

#ifndef PARSERBASE_H
#define PARSERBASE_H

#include <QObject>
#include <QUrl>

class ParserBase : public QObject
{
    Q_OBJECT
public:
    explicit ParserBase(QObject *parent = nullptr);
    virtual ~ParserBase();
    
    void parse(const QUrl &url, bool download);
    
signals:
    // Emit it to show the Downloader
    void downloadTasksAdded(void);

protected:
    // Implemented in child class to run the parser
    virtual void runParser(const QUrl &url) = 0;

    // Call it after "result" is filled
    void finishParsing(void);

    // Show error dialog
    void showErrorDialog(const QString &errMsg);

    // the member "result" should be filled in child class
    struct Stream
    {
        QList<QUrl> urls;
        QString container;
        QString referer;
        QString ua;
        bool seekable;
        bool is_dash;
    };
    struct Result
    {
        QString title;
        QStringList stream_types;
        QList<Stream> streams;
        QUrl danmaku_url;
    } result;
    
    QUrl m_url;
    bool m_download;

private:
    void finishStreamSelection(int index);
};

#endif // PARSERBASE_H
