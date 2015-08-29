#ifndef DANMAKULOADER_H
#define DANMAKULOADER_H

#include <QObject>
#include <QProcess>
class QNetworkReply;

class DanmakuLoader : public QObject
{
    Q_OBJECT
public:
    explicit DanmakuLoader(QObject *parent = 0);
    void load(const QString &xmlFile, int width, int height);

signals:
    void finished(const QString &assFile);

private slots:
    void reload(void);
    void onXmlDownloaded(void);
    void onProcessFinished(void);

private:
    QProcess *process;
    QNetworkReply *reply;
    QString xmlFile;
    int width;
    int height;
};

#endif // DANMAKULOADER_H
