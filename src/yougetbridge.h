#ifndef YOUGETBRIDGE_H
#define YOUGETBRIDGE_H

#include <QObject>
class QProcess;

class YouGetBridge : public QObject
{
    Q_OBJECT
public:
    explicit YouGetBridge(QObject *parent = 0);
    void parse(const QString &url, bool download);

private:
    QProcess *process;
    bool download;

private slots:
    void onFinished(void);
};

extern YouGetBridge you_get_bridge;

#endif // YOUGETBRIDGE_H
