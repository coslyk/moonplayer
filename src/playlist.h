#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QWidget>
#include <QVector>
class QListWidgetItem;
class QMenu;
class VideoParser;
class QDir;

namespace Ui {
class Playlist;
}

class Playlist : public QWidget
{
    Q_OBJECT
    
public:
    explicit Playlist(QWidget *parent = 0);
    void playNext(void);
    void setSkin(const QDir &dir);
    void setNoSkin(void);
    ~Playlist();

public slots:
    void addFile(const QString& name, const QString& file);
    void addFileAndPlay(const QString& name, const QString& file);
    void addList(const QString& filename);
    void addUrl(const QString& url);

signals:
    void fileSelected(const QString&);
    void needPause(bool);

private slots:
    void selectFile(QListWidgetItem* item);
    void onAddItem(void);
    void onNetItem(void);
    void onDelButton(void);
    void onListItem(void);
    void clearList(void);
    void showMenu(void);
    
private:
    Ui::Playlist *ui;
    QMenu *menu;
    QVector<QString> filelist;
    int last_index;
    void startPlayingFile(const QString &file);
};
extern Playlist *playlist;

#endif // PLAYLIST_H
