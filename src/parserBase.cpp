#include "parserBase.h"
#include <QDir>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QSettings>
#include <QUrl>
#include "accessManager.h"
#include "console.h"
//#include "danmakudelaygetter.h"
#include "downloader.h"
#include "platform/paths.h"
#include "selectionDialog.h"
#include "playlistModel.h"

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
    result.container.clear();
    result.danmaku_url.clear();
    result.urls.clear();
    result.referer.clear();
    result.ua.clear();
    result.seekable = true;
    result.is_dash = false;
    runParser(url);
}


void ParserBase::finishParsing()
{
    // Check if source is empty
    if (result.urls.isEmpty())
    {
        showErrorDialog(tr("The video's url is empty. Maybe it is a VIP video and requires login."));
        return;
    }

    // Bind referer and user-agent
    if (!result.referer.isEmpty())
    {
        foreach (QUrl url, result.urls)
            NetworkAccessManager::instance()->addReferer(url, result.referer.toUtf8());
    }
    if (!result.ua.isEmpty())
    {
        foreach (QUrl url, result.urls)
            NetworkAccessManager::instance()->addUserAgent(url, result.ua.toUtf8());
    }
    if (!result.seekable)
    {
         foreach (QUrl url, result.urls)
            NetworkAccessManager::instance()->addUnseekableHost(url.host());
    }

    // replace illegal chars in title with .
    static QRegularExpression illegalChars("[\\\\/]");
    result.title.replace(illegalChars, ".");

    // Download
    if (m_download)
    {
        Downloader::instance()->addTasks(result.title + '.' + result.container, result.urls, result.danmaku_url, result.is_dash);
        emit downloadTasksAdded();
    }

    // Play
    else
    {
        if (result.is_dash)  // YouTube's dash stream
        {
            PlaylistModel::instance()->addItem(result.title, result.urls[0], result.danmaku_url, result.urls[1]);
        }
        else
        {
            PlaylistModel::instance()->addItems(result.title, result.urls, result.danmaku_url);
        }
    }
}


int ParserBase::selectQuality(const QStringList &stream_types)
{
    static SelectionDialog *selectionDialog = nullptr;
    if (selectionDialog == nullptr)
        selectionDialog = new SelectionDialog;

    // If there is only one quality, use it
    if (stream_types.length() == 1)
        return 0;

    // Select
    int selected = selectionDialog->showDialog_Index(stream_types, tr("Please select a video quality:"));
    return selected;
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
        updateParser();
}

void ParserBase::updateParser()
{
    static Console* c_console = nullptr;
    if (c_console == nullptr)
        c_console = new Console;
    QStringList args;
#ifdef Q_OS_WIN
    args << "-ExecutionPolicy" << "RemoteSigned";
    args << "-File" << (appResourcesPath() + "/update-parsers.ps1");
    c_console->launchScript("powershell", args);
#else
    args << appResourcesPath() + "/update-parsers.sh";
    c_console->launchScript("sh", args);
#endif
}

