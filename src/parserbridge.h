#ifndef PARSERBRIDGE_H
#define PARSERBRIDGE_H

#include <QObject>
class QProcess;
class SelectionDialog;

class ParserBridge : public QObject
{
    Q_OBJECT
public:
    explicit ParserBridge(QObject *parent = 0);
    virtual ~ParserBridge();
    void parse(const QString &url, bool download);

public slots:
    void upgradeParsers(void);


protected:
    virtual void runParser(const QString &url) = 0;

    // following can be used in child class
    static SelectionDialog *selectionDialog;
    void finishParsing(void);
    void showErrorDialog(const QString &errMsg);

    // the following members should be filled in child class
    struct Result
    {
        QStringList names;
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

#endif // PARSERBRIDGE_H
