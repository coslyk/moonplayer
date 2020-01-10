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
    void downloadTasksAdded(void);
    void playlistParsed(const QStringList& titles, const QList<QUrl>& urls, bool download);

protected slots:
    void showErrorDialog(const QString &errMsg);

protected:
    virtual void runParser(const QUrl &url) = 0;

    // following can be used in child class
    void finishParsing(void);
    int selectQuality(const QStringList &stream_types);
    void selectEpisode(const QStringList& titles, const QList<QUrl>& urls);

    // the following members should be filled in child class
    struct Result
    {
        QList<QUrl> urls;
        QString title;
        QString container;
        QString referer;
        QString ua;
        QUrl danmaku_url;
        bool seekable;
        bool is_dash;
    } result;

private:
    QUrl m_url;
    bool m_download;
};

#endif // PARSERBASE_H
