#include "downloaderAbstractItem.h"
#include <QFileInfo>

DownloaderAbstractItem::DownloaderAbstractItem(const QString& filepath, const QUrl &danmakuUrl, QObject* parent) :
    QObject(parent),
    m_name(QFileInfo(filepath).fileName()),
    m_filePath(filepath),
    m_danmakuUrl(danmakuUrl),
    m_state(WAITING),
    m_progress(0)
{
}

DownloaderAbstractItem::~DownloaderAbstractItem()
{
}


void DownloaderAbstractItem::setName(const QString& name)
{
    if (m_name != name)
    {
        m_name = name;
        emit nameChanged();
    }
}

void DownloaderAbstractItem::setFilePath(const QString& filePath)
{
    if (m_filePath != filePath)
    {
        m_filePath = filePath;
        emit filePathChanged();
    }
}


void DownloaderAbstractItem::setProgress(int progress)
{
    if (m_progress != progress)
    {
        m_progress = progress;
        emit progressChanged();
    }
}

void DownloaderAbstractItem::setState(DownloaderAbstractItem::State state)
{
    if (m_state != state)
    {
        m_state = state;
        emit stateChanged();

        // Write danmaku url
        if (state == FINISHED && !m_danmakuUrl.isEmpty())
        {
            QFile f(m_filePath + ".danmaku");
            if (f.open(QFile::WriteOnly))
            {
                f.write(m_danmakuUrl.toString().toUtf8());
                f.close();
            }
        }
    }
}
