#ifndef DANMAKULOADER_H
#define DANMAKULOADER_H

#include <QObject>
#include <Python.h>
class QNetworkReply;

class DanmakuLoader : public QObject
{
    Q_OBJECT
public:
    explicit DanmakuLoader(QObject *parent = 0);

public slots:
    void load(const QString &srcFile, int width, int height);

signals:
    void finished(const QString &assFile);

private slots:
    void reload(void);
    void onXmlDownloaded(void);

private:
    QNetworkReply *reply;
    QString xmlFile;
    PyObject *module;
    PyObject *danmaku2assFunc;
    int width;
    int height;
};

#endif // DANMAKULOADER_H
