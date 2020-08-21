#ifndef DANMAKULOADER_H
#define DANMAKULOADER_H

#include <QObject>
class QNetworkReply;

class DanmakuLoader : public QObject
{
    Q_OBJECT
public:
    explicit DanmakuLoader(QObject *parent = nullptr);
    inline static DanmakuLoader* instance() { return &s_instance; }
    
    void start(const QUrl& srcUrl, int width, int height);

private slots:
    void onXmlDownloaded(void);

private:
    QNetworkReply *m_reply;
    int m_width;
    int m_height;
    
    static DanmakuLoader s_instance;
};

#endif // DANMAKULOADER_H
