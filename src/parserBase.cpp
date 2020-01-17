#include "parserBase.h"
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QUrl>
#include "accessManager.h"
#include "downloader.h"
#include "playlistModel.h"
#include "utils.h"

ParserBase::ParserBase(QObject *parent) : QObject(parent)
{
}


ParserBase::~ParserBase()
{
}


void ParserBase::parse(const QUrl &url, bool download)
{
    m_url = url;
    m_download = download;
    result.title.clear();
    result.stream_types.clear();
    result.streams.clear();
    result.danmaku_url.clear();
    runParser(url);
}


void ParserBase::finishParsing()
{
    // replace illegal chars in title with .
    static QRegularExpression illegalChars("[\\\\/]");
    result.title.replace(illegalChars, ".");
    
    // Stream is empty
    if (result.streams.isEmpty())
        showErrorDialog(tr("The video has no streams. Maybe it is a VIP video and requires login."));

    // Has only one stream, no selection needed
    else if (result.streams.count() == 1)
        finishStreamSelection(0);
    
    // More than one stream, selection is needed
    else
        emit streamSelectionNeeded(result.stream_types);
}


void ParserBase::finishStreamSelection(int index)
{
    Stream stream = result.streams[index];
    
    // Bind referer and user-agent
    if (!stream.referer.isEmpty())
    {
        foreach (QUrl url, stream.urls)
            NetworkAccessManager::instance()->addReferer(url, stream.referer.toUtf8());
    }
    if (!stream.ua.isEmpty())
    {
        foreach (QUrl url, stream.urls)
            NetworkAccessManager::instance()->addUserAgent(url, stream.ua.toUtf8());
    }
    if (!stream.seekable)
    {
         foreach (QUrl url, stream.urls)
            NetworkAccessManager::instance()->addUnseekableHost(url.host());
    }

    // Download
    if (m_download)
    {
        Downloader::instance()->addTasks(result.title + '.' + stream.container, stream.urls, result.danmaku_url, stream.is_dash);
        emit downloadTasksAdded();
    }

    // Play
    else
    {
        PlaylistModel::instance()->addItems(result.title, stream.urls, result.danmaku_url, stream.is_dash);
    }
}


void ParserBase::selectEpisode(const QStringList& titles, const QList<QUrl>& urls)
{
    emit albumParsed(titles, urls, m_download);
}


void ParserBase::showErrorDialog(const QString &errMsg)
{
    QMessageBox msgBox;
    msgBox.setText("Error");
    msgBox.setInformativeText("Parse failed!\nURL:" + m_url.toString());
    msgBox.setDetailedText("URL: " + m_url.toString() + "\n\n" + errMsg);
    QPushButton *updateButton = msgBox.addButton(tr("Upgrade parser"), QMessageBox::ActionRole);
    msgBox.addButton(QMessageBox::Cancel);
    msgBox.exec();
    if (msgBox.clickedButton() == updateButton)
        Utils::updateParser();
}

