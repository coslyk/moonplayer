#ifndef PARSERBASE_H
#define PARSERBASE_H

#include <QObject>

class ParserBase : public QObject
{
    Q_OBJECT
public:
    explicit ParserBase(QObject *parent = NULL);
    virtual ~ParserBase();
    void parse(const QString &url, bool download);

protected slots:
    void showErrorDialog(const QString &errMsg);

protected:
    virtual void runParser(const QString &url) = 0;

    // following can be used in child class
    void finishParsing(void);
    int selectQuality(const QStringList &stream_types);

    // the following members should be filled in child class
    struct Result
    {
        QStringList urls;
        QString title;
        QString container;
        QString danmaku_url;
        QString referer;
        QString ua;
        bool seekable;
        bool is_dash;
    } result;

private:
    QString url;
    bool download;
};

void parseUrl(const QString &url, bool download);

#endif // PARSERBASE_H
