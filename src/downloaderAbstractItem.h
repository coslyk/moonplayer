#ifndef DOWNLOADERABSTRACTITEM_H
#define DOWNLOADERABSTRACTITEM_H

#include <QObject>
#include <QUrl>

class DownloaderAbstractItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name      READ name        NOTIFY nameChanged)
    Q_PROPERTY(QString filePath  READ filePath    NOTIFY filePathChanged)
    Q_PROPERTY(QUrl danmakuUrl   READ danmakuUrl  NOTIFY danmakuUrlChanged)
    Q_PROPERTY(State state       READ state       NOTIFY stateChanged)
    Q_PROPERTY(int progress      READ progress    NOTIFY progressChanged)
    
    
public:
    enum State {WAITING, PAUSED, DOWNLOADING, FINISHED, ERROR, CANCELED};
    Q_ENUM(State);
    
    DownloaderAbstractItem(const QString &filepath, const QUrl &danmakuUrl = QUrl(), QObject *parent = nullptr);
    virtual ~DownloaderAbstractItem();
    Q_INVOKABLE virtual void pause(void) = 0;
    Q_INVOKABLE virtual void start(void) = 0;
    Q_INVOKABLE virtual void stop(void) = 0;
    
    inline QString name() { return m_name; }
    inline QString filePath() { return m_filePath; }
    inline QUrl danmakuUrl() { return m_danmakuUrl; }
    inline State state() { return m_state; }
    inline int progress() { return m_progress; }
    
protected:
    void setName(const QString& name);
    void setFilePath(const QString& filePath);
    void setState(State state);
    void setProgress(int progress);
    
signals:
    void danmakuUrlChanged(void);
    void nameChanged(void);
    void filePathChanged(void);
    void stateChanged(void);
    void progressChanged(void);

private:
    QString m_name;
    QString m_filePath;
    QUrl m_danmakuUrl;
    State m_state;
    int m_progress;
};

#endif // DOWNLOADERABSTRACTITEM_H
