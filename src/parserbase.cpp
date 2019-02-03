#include "parserbase.h"
#include <QDir>
#include <QMessageBox>
#include <QPushButton>
#include "accessmanager.h"
#include "danmakudelaygetter.h"
#include "downloader.h"
#include "platforms.h"
#include "playlist.h"
#include "reslibrary.h"
#include "selectiondialog.h"
#include "settings_network.h"
#include "terminal.h"
#include "parserykdl.h"
#include "youtubedlbridge.h"
#include "extractor.h"
#include "simuparserbridge.h"


SelectionDialog *ParserBase::selectionDialog = NULL;

ParserBase::ParserBase(QObject *parent) : QObject(parent)
{
}


ParserBase::~ParserBase()
{
}


void ParserBase::parse(const QString &url, bool download)
{
    if (selectionDialog == NULL)
        selectionDialog = new SelectionDialog;
    this->url = url;
    this->download = download;
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

    // Bind referer and use-agent
    if (!result.referer.isEmpty())
    {
        foreach (QString url, result.urls)
            referer_table[QUrl(url).host()] = result.referer.toUtf8();
    }
    if (!result.ua.isEmpty())
    {
        foreach (QString url, result.urls)
            ua_table[QUrl(url).host()] = result.ua.toUtf8();
    }
    if (!result.seekable)
    {
         foreach (QString url, result.urls) {
            unseekable_hosts.append(QUrl(url).host());
        }
    }

    // replace illegal chars in title with .
    static QRegularExpression illegalChars("[\\\\/]");
    result.title.replace(illegalChars, ".");

    // Generate names list
    QStringList names;
    if (result.is_dash)
        names << ("video." + result.container) << ("audio." + result.container);
    else
    {
        for (int i = 0; i < result.urls.size(); i++)
            names << QString("%1_%2.%3").arg(result.title, QString::number(i), result.container);
    }

    // Download
    if (download)
    {
        // Build file path list
        QDir dir = QDir(Settings::downloadDir);
        QString dirname = result.title + '.' + result.container;
        if (result.urls.size() > 1)
        {
            if (!dir.cd(dirname))
            {
                dir.mkdir(dirname);
                dir.cd(dirname);
            }
        }
        for (int i = 0; i < names.size(); i++)
             names[i] = dir.filePath(QString(names[i]));

        // Download videos with danmaku
        if (!result.danmaku_url.isEmpty())
        {
            if (result.urls.size() > 1)
                new DanmakuDelayGetter(names, result.urls, result.danmaku_url, true, this);
            else
                downloader->addTask(result.urls[0].toUtf8(), names[0], false, result.danmaku_url.toUtf8());
        }
        // Download videos without danmaku
        else
        {
            for (int i = 0; i < result.urls.size(); i++)
                 downloader->addTask(result.urls[i].toUtf8(), names[i], result.urls.size() > 1);
        }
        QMessageBox::information(NULL, "Message", tr("Add download task successfully!"));
    }

    // Play
    else if (result.is_dash) // dash streams
    {
        playlist->addFileAndPlay(result.title, result.urls[0], 0, result.urls[1]);
        res_library->close();
    }
    else if (!result.danmaku_url.isEmpty()) // with danmaku
    {
        if (result.urls.size() > 1)
            new DanmakuDelayGetter(names, result.urls, result.danmaku_url, false, this);
        else
            playlist->addFileAndPlay(names[0], result.urls[0], result.danmaku_url);
        res_library->close();
    }
    else
    {
        playlist->addFileAndPlay(names[0], result.urls[0]);
        for (int i = 1; i < result.urls.size(); i++)
            playlist->addFile(names[i], result.urls[i]);
        res_library->close();
    }
}

void ParserBase::showErrorDialog(const QString &errMsg)
{
    QMessageBox msgBox;
    msgBox.setText("Error");
    msgBox.setInformativeText("Parse failed!\nURL:" + url);
    msgBox.setDetailedText("URL: " + url + "\n\n" + errMsg);
    QPushButton *updateButton = msgBox.addButton(tr("Upgrade parser"), QMessageBox::ActionRole);
    msgBox.addButton(QMessageBox::Cancel);
    msgBox.exec();
    if (msgBox.clickedButton() == updateButton)
        upgradeParsers();
}


void ParserBase::upgradeParsers()
{
    upgradeParsers();
}

void upgradeParsers()
{
    execShell(parserUpgraderPath());
}


void parseUrl(const QString &url, bool download)
{
    QString host = QUrl(url).host();
    if (Extractor::isSupported(host))
        simuParserBase->parse(url, download);
    else if (ParserYkdl::isSupported(host))
        parser_ykdl->parse(url, download);
    else
        youtubedl_bridge->parse(url, download);
}

