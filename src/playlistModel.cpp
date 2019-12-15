#include "playlistModel.h"
#include "mpvObject.h"
#include "parserYkdl.h"
#include "parserYoutubedl.h"
#include <QFileInfo>
#include <QSettings>

PlaylistModel PlaylistModel::s_instance;

PlaylistModel::PlaylistModel(QObject* parent) :
    QAbstractListModel(parent), m_playingIndex(-1)
{
}


// Add item to playlist
void PlaylistModel::addItem(const QString& title, const QUrl& fileUrl, const QUrl& danmakuUrl, const QUrl& audioTrackUrl)
{
    int index = m_titles.count();
    beginInsertRows(QModelIndex(), index, index);
    m_titles << title;
    m_fileUrls << fileUrl;
    m_danmakuUrls << danmakuUrl;
    m_audioTrackUrls << audioTrackUrl;
    endInsertRows();
    playItem(index);
}


void PlaylistModel::addItems(const QString& title, const QList<QUrl>& fileUrls, const QUrl& danmakuUrl)
{
    int start = m_titles.count();
    int count = fileUrls.count();
    beginInsertRows(QModelIndex(), start, start + count - 1);
    for (int i = 0; i < count; i++)
    {
        m_titles << (title + "_" + QString::number(i));
        m_fileUrls << fileUrls[i];
        m_danmakuUrls << QUrl();
        m_audioTrackUrls << QUrl();
    }
    endInsertRows();
    playItem(start);
}

void PlaylistModel::addLocalFiles(const QList<QUrl>& fileUrls)
{
    int start = m_titles.count();
    int count = fileUrls.count();
    beginInsertRows(QModelIndex(), start, start + count - 1);
    for (int i = 0; i < count; i++)
    {
        m_titles << QFileInfo(fileUrls[i].toLocalFile()).fileName();
        m_fileUrls << fileUrls[i];
        m_audioTrackUrls << QUrl();
        QFile danmakuFile(fileUrls[i].toLocalFile() + ".danmaku");
        if (danmakuFile.open(QFile::ReadOnly | QFile::Text))
        {
            m_danmakuUrls << QString::fromUtf8(danmakuFile.readAll());
            danmakuFile.close();
        }
        else
        {
            m_danmakuUrls << QUrl();
        }
    }
    endInsertRows();
    playItem(start);
}



void PlaylistModel::addUrl ( const QUrl& url, bool download )
{
    if (ParserYkdl::isSupported(url))
        ParserYkdl::instance()->parse(url, download);
    else
        ParserYoutubeDL::instance()->parse(url, download);
}


void PlaylistModel::addUrl(const QUrl& url)
{
    QSettings settings;
    OpenUrlAction action = static_cast<OpenUrlAction>(settings.value("player/url_open_mode").toInt());
    if (action == QUESTION)
        emit urlDialogRequested(url);
    else
        addUrl(url, action == DOWNLOAD);
}


void PlaylistModel::removeItem(int index)
{
    if (index < 0)
        return;
    beginRemoveRows(QModelIndex(), index, index);
    m_titles.removeAt(index);
    m_fileUrls.removeAt(index);
    m_danmakuUrls.removeAt(index);
    m_audioTrackUrls.removeAt(index);
    endRemoveRows();
}

void PlaylistModel::clear()
{
    if (m_titles.count() == 0)
        return;
    beginRemoveRows(QModelIndex(), 0, m_titles.count() - 1);
    m_titles.clear();
    m_fileUrls.clear();
    m_danmakuUrls.clear();
    m_audioTrackUrls.clear();
    endRemoveRows();
}


void PlaylistModel::playItem(int index)
{
    if (index < m_titles.count())
    {
        MpvObject::instance()->open(m_fileUrls[index], m_danmakuUrls[index], m_audioTrackUrls[index]);
    }
    if (m_playingIndex != index)
    {
        m_playingIndex = index;
        emit playingIndexChanged();
    }
}


void PlaylistModel::playNextItem()
{
    playItem(m_playingIndex + 1);
}



int PlaylistModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_titles.count();
}

QVariant PlaylistModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    switch (role) {
        case TitleRole:
            return m_titles[row];
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> PlaylistModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TitleRole] = "title";
    return roles;
}


