#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QWidget>
#include <QHash>
#include <QLinkedList>

class QVBoxLayout;
class QTreeWidget;
class QPushButton;
class QTreeWidgetItem;
class HttpGet;
class DownloaderGroup;

class Downloader : public QWidget
{
    Q_OBJECT

signals:
    void newPlay(const QString &name, const QString &file);
    void newFile(const QString &name, const QString &file);

public:
    explicit Downloader(QWidget *parent = 0);
    inline bool hasTask(){return n_downloading != 0;}
    
public slots:
    void addTask(const QByteArray &url, const QString &filename, bool in_group);
    
private:
    QTreeWidget *treeWidget;
    QHash<HttpGet*, QTreeWidgetItem*> get2item;
    QHash<QTreeWidgetItem*, HttpGet*> item2get;
    QHash<QString, DownloaderGroup*> dir2group;
    QLinkedList<HttpGet*> waitings;
    int n_downloading;

private slots:
    void onFinished(HttpGet *get, bool error);
    void onProgressChanged(HttpGet *get, int progress, bool is_percentage);
    void onPaused(HttpGet *get, int reason);
    void onPauseButton(void);
    void onPlayButton(void);
    void onDelButton(void);
};
extern Downloader *downloader;

#endif // DOWNLOADER_H
