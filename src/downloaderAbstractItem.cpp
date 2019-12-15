#include "downloaderAbstractItem.h"
#include <QFileInfo>

DownloaderAbstractItem::DownloaderAbstractItem(const QString& filepath, QObject* parent) :
    QObject(parent), m_name(QFileInfo(filepath).fileName()), m_filePath(filepath), m_state(WAITING), m_progress(0)
{
}

DownloaderAbstractItem::~DownloaderAbstractItem()
{
}

QString DownloaderAbstractItem::name()
{
    return m_name;
}

QString DownloaderAbstractItem::filePath()
{
    return m_filePath;
}

DownloaderAbstractItem::State DownloaderAbstractItem::state()
{
    return m_state;
}

int DownloaderAbstractItem::progress()
{
    return m_progress;
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
    }
}






