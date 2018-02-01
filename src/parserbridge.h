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

private slots:
    void onFinished(void);
    void onError(void);

protected:
    static SelectionDialog *selectionDialog;
    QProcess *process;

    virtual void runParser(const QString &url) = 0;
    virtual void parseOutput(const QByteArray &jsonData) = 0;

    // the following members should be filled in parseJson
    QStringList names;
    QStringList urls;
    QString title;
    QString container;
    QString danmaku_url;
    QString referer;
    QString ua;
    bool seekable;
    bool is_dash;

private:
    QString url;
    bool download;
};

#endif // PARSERBRIDGE_H
