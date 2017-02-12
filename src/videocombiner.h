#ifndef VIDEOCOMBINER_H
#define VIDEOCOMBINER_H

#include <QProcess>
#include <QDir>

class VideoCombiner : public QProcess
{
    Q_OBJECT
public:
    explicit VideoCombiner(QObject *parent = 0, const QDir &dir = QDir());
private:
    QDir dir;
    QString save_as;
private slots:
    void onFinished(int status);
};

#endif // VIDEOCOMBINER_H
