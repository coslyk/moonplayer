#include "danmakudelaygetter.h"
#include <QProcess>

DanmakuDelayGetter::DanmakuDelayGetter(QStringList &names, QStringList &urls,
                                       const QString &danmakuUrl, QObject *parent) :
    QObject(parent), names(names), urls(urls), danmakuUrl(danmakuUrl)
{
    process = new QProcess(this);
    connect(process, SIGNAL(finished(int)), this, SLOT(onFinished()));
    delay = 0;
    start();
}

void DanmakuDelayGetter::start()
{
    QStringList args;
    args << "-nosound" << "-vc" << "dummy" << "-vo" << "null" << "-identify" << urls.first();
    process->start("mplayer", args);
}

void DanmakuDelayGetter::onFinished()
{
    QString output = QString::fromUtf8(process->readAllStandardOutput());
    double length = output.section("ID_LENGTH=", 1).section('\n', 0, 0).toDouble();

    if (delay < 0.5) // first video clip
        emit newPlay(names.takeFirst(), urls.takeFirst(), danmakuUrl);
    else
        emit newFile(names.takeFirst(), urls.takeFirst(), QString::number(delay) + ' ' + danmakuUrl);

    delay += length;

    if (names.count() == 1) // the last clip
        emit newFile(names.takeFirst(), urls.takeFirst(), QString::number(delay) + ' ' + danmakuUrl);

    if (names.isEmpty())
        deleteLater();
    else
        start();
}
