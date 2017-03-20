#ifndef YOUGETBRIDGE_H
#define YOUGETBRIDGE_H

#include <QObject>
class QProcess;
class SelectionDialog;

class YouGetBridge : public QObject
{
    Q_OBJECT
public:
    explicit YouGetBridge(QObject *parent = 0);
    ~YouGetBridge();
    void parse(const QString &url, bool download, const QString &danmaku = QString(),
               const QString &format = QString());

#ifdef Q_OS_MAC
public slots:
    void updateYouGet(void);
#endif

private:
    SelectionDialog *selectionDialog;
    QProcess *process;
    QString url;
    QString danmaku;
    QString format;
    bool download;

private slots:
    void onFinished(void);
};

extern YouGetBridge you_get_bridge;

#endif // YOUGETBRIDGE_H
