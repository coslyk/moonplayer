#ifndef UPDATECHECKER_H
#define UPDATECHECKER_H

#include <QObject>
class QNetworkReply;

class UpdateChecker : public QObject
{
    Q_OBJECT
public:
    explicit UpdateChecker(QObject *parent = 0);
    void check(void);

public slots:
    void onFinished(void);

private:
    QNetworkReply *reply;
};

#endif // UPDATECHECKER_H
