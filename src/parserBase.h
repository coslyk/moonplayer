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
    
    Q_INVOKABLE static void updateParser(void);
    
signals:
    // Emit it to show the Downloader
    void downloadTasksAdded(void);
    
    // Emit it to show a QML dialog to select episode from an album
    void albumParsed(const QStringList& titles, const QList<QUrl>& urls, bool download);
    
    // Emit it to show a QML dialog to select streams
    void streamSelectionNeeded(const QStringList& stream_types);
    
public slots:
    // Called from QML's dialog after a stream is selected
    void finishStreamSelection(int index);  

protected:
    // Implemented in child class to run the parser
    virtual void runParser(const QUrl &url) = 0;
    
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
    
    // Following functions can be used in child class
    // Called after "result" is filled
    void finishParsing(void);
    // Request to select episode from album
    void selectEpisode(const QStringList& titles, const QList<QUrl>& urls);
    // Show error dialog
    void showErrorDialog(const QString &errMsg);

private:
    QUrl m_url;
    bool m_download;
};

#endif // PARSERBASE_H
