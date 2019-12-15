#ifndef DANMAKULOADER_H
#define DANMAKULOADER_H

#include <QObject>
class QNetworkReply;
class QProcess;

class DanmakuLoader : public QObject
{
    Q_OBJECT
public:
    explicit DanmakuLoader(QObject *parent = nullptr);
    inline static DanmakuLoader* instance() { return &s_instance; }
    
    void start(const QUrl& srcUrl, int width, int height);

private slots:
    void onXmlDownloaded(void);
    void onProcessFinished(void);

private:
    QNetworkReply *m_reply;
    QProcess *m_process;
    QString m_outputFile;
    int m_width;
    int m_height;
    
    static DanmakuLoader s_instance;
};

#endif // DANMAKULOADER_H
