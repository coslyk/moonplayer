#ifndef PLAYLISTMODEL_H
#define PLAYLISTMODEL_H

#include <QAbstractListModel>
#include <QUrl>

class PlaylistModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int playingIndex MEMBER m_playingIndex NOTIFY playingIndexChanged);
    
    enum OpenUrlAction {
        QUESTION,
        PLAY,
        DOWNLOAD
    };

public:
    enum PlaylistRoles {
        TitleRole = Qt::UserRole + 1
    };

public:
    inline static PlaylistModel* instance(void) { return &s_instance; };
    
    PlaylistModel(QObject* parent = nullptr);
    
    Q_INVOKABLE void addItem(const QString& title, const QUrl& fileUrl, const QUrl& danmakuUrl = QUrl(), const QUrl& audioTrackUrl = QUrl());
    Q_INVOKABLE void addItems(const QString& title, const QList<QUrl>& fileUrls, const QUrl& danmakuUrl = QUrl(), bool isDash = false);
    Q_INVOKABLE void addLocalFiles(const QList<QUrl>& fileUrls);
    Q_INVOKABLE void addUrl(const QUrl& url, bool download);
    Q_INVOKABLE void removeItem(int index);
    Q_INVOKABLE void clear(void);
    Q_INVOKABLE void playItem(int index);
    Q_INVOKABLE void playNextItem(void);
    void addUrl(const QUrl& url);   // Load "download" option from settings or open a dialog to question
    
    int rowCount(const QModelIndex & parent) const override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex & index, int role) const override;
    
signals:
    void playingIndexChanged(void);
    void urlDialogRequested(const QUrl& url);

private:
    QStringList m_titles;
    QList<QUrl> m_fileUrls;
    QList<QUrl> m_danmakuUrls;
    QList<QUrl> m_audioTrackUrls;
    int m_playingIndex;
    
    static PlaylistModel s_instance;
};

#endif // PLAYLISTMODEL_H
