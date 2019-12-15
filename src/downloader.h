#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>

class Downloader : public QObject
{
    Q_OBJECT

public:
    static Downloader* instance(void);
    Downloader(QObject* parent = nullptr);
    ~Downloader();
    
    void addTasks(const QString& name, const QList<QUrl>& urls, bool isDash = false);
    QList<QObject*> model(void);
    
signals:
    void modelUpdated(void);
    
private:
    QList<QObject*> m_model;
};

#endif // DOWNLOADER_H
