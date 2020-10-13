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
