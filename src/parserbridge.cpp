#include "parserbridge.h"
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QPushButton>
#include "accessmanager.h"
#include "danmakudelaygetter.h"
#include "downloader.h"
#include "platforms.h"
#include "playlist.h"
#include "reslibrary.h"
#include "selectiondialog.h"
#include "settings_network.h"
#include "settings_plugins.h"
#include "settingsdialog.h"
#include "terminal.h"
#include "ykdlbridge.h"
#include "yougetbridge.h"

SelectionDialog *ParserBridge::selectionDialog = NULL;

ParserBridge::ParserBridge(QObject *parent) : QObject(parent)
{
    process = new QProcess(this);

    // Set environments
    QStringList envs = QProcess::systemEnvironment();
    envs << "PYTHONIOENCODING=utf8";
#ifdef Q_OS_MAC
    envs << "LC_CTYPE=en_US.UTF-8";
    // add "/usr/local/bin" to path
    for (int i = 0; i < envs.size(); i++)
    {
        if (envs[i].startsWith("PATH=") && !envs[i].contains("/usr/local/bin"))
        {
            envs[i] += ":/usr/local/bin";
            break;
        }
    }
#endif
    process->setEnvironment(envs);
    connect(process, SIGNAL(finished(int)),this, SLOT(onFinished()));
}


ParserBridge::~ParserBridge()
{
    if (process->state() == QProcess::Running)
    {
        process->kill();
        process->waitForFinished();
    }
}


void ParserBridge::parse(const QString &url, bool download)
{
    if (process->state() == QProcess::Running)
    {
        QMessageBox::warning(NULL, "Error", tr("Another file is being parsed."));
        return;
    }
    this->url = url;
    this->download = download;
    result.title.clear();
    result.container.clear();
    result.danmaku_url.clear();
    result.names.clear();
    result.urls.clear();
    result.referer.clear();
    result.ua.clear();
    result.seekable = true;
    result.is_dash = false;
    runParser(url);
}


void ParserBridge::onFinished()
{
    if (selectionDialog == NULL)
        selectionDialog = new SelectionDialog;
    QByteArray output = process->readAllStandardOutput();
    parseOutput(output);

    // Error
    if (result.urls.isEmpty())
    {
        showErrorDialog(QString::fromUtf8(process->readAllStandardError()));
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
        for (int i = 0; i < result.names.size(); i++)
             result.names[i] = dir.filePath(QString(result.names[i]));

        // Download videos with danmaku
        if (!result.danmaku_url.isEmpty())
        {
            if (result.urls.size() > 1)
                new DanmakuDelayGetter(result.names, result.urls, result.danmaku_url, true, this);
            else
                downloader->addTask(result.urls[0].toUtf8(), result.names[0], false, result.danmaku_url.toUtf8());
        }
        // Download videos without danmaku
        else
        {
            for (int i = 0; i < result.urls.size(); i++)
                 downloader->addTask(result.urls[i].toUtf8(), result.names[i], result.urls.size() > 1);
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
            new DanmakuDelayGetter(result.names, result.urls, result.danmaku_url, false, this);
        else
            playlist->addFileAndPlay(result.names[0], result.urls[0], result.danmaku_url);
    }
    else
    {
        playlist->addFileAndPlay(result.names[0], result.urls[0]);
        for (int i = 1; i < result.urls.size(); i++)
            playlist->addFile(result.names[i], result.urls[i]);
        res_library->close();
    }
}

void ParserBridge::showErrorDialog(const QString &errMsg)
{
    QMessageBox msgBox;
    msgBox.setText("Error");
    msgBox.setInformativeText("Parse failed!\nURL:" + url);
    msgBox.setDetailedText("URL: " + url + "\n\n" + errMsg);
    QPushButton *updateButton = msgBox.addButton(tr("Upgrade parser"), QMessageBox::ActionRole);
    QPushButton *switchButton = msgBox.addButton(tr("Use another parser"), QMessageBox::ActionRole);
    msgBox.addButton(QMessageBox::Cancel);
    msgBox.exec();
    if (msgBox.clickedButton() == updateButton)
        upgradeParsers();
    else if (msgBox.clickedButton() == switchButton)
        settingsDialog->exec();
}


void ParserBridge::upgradeParsers()
{
    execShell(parserUpgraderPath());
}

