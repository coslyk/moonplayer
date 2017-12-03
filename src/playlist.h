#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <QWidget>
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
    void initClassicUI(void);
    void playNext(void);
    ~Playlist();

public slots:
    void addFile(const QString& name, const QString& file, const QString &danmaku = QString());
    void addFileAndPlay(const QString& name, const QString& file, const QString &danmaku = QString());
    void addList(const QString& filename);
    void addUrl(const QString& url);
    void onAddItem(void);
    void onNetItem(void);
    void onDelButton(void);
    void onListItem(void);

signals:
    void fileSelected(const QString &file, const QString &danmaku, const QString &audioTrack = QString());
    void needPause(bool);

private slots:
    void selectFile(QListWidgetItem* item);
    void clearList(void);
    void showMenu(void);
    
private:
    Ui::Playlist *ui;
    QMenu *menu;
    int last_index;
};
extern Playlist *playlist;

#endif // PLAYLIST_H
