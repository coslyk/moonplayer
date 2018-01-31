#include "parserbridge.h"
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include "accessmanager.h"
#include "downloader.h"
#include "playlist.h"
#include "reslibrary.h"
#include "selectiondialog.h"
#include "settings_network.h"

SelectionDialog *ParserBridge::selectionDialog = NULL;

ParserBridge::ParserBridge(QObject *parent) : QObject(parent)
{
    process = new QProcess(this);

    // Set environments
    QStringList envs = QProcess::systemEnvironment();
    envs << "PYTHONIOENCODING=utf8";
#ifdef Q_OS_MAC
    envs << "LC_CTYPE=en_US.UTF-8";
#endif
    process->setEnvironment(envs);
    connect(process, SIGNAL(finished(int)),this, SLOT(onFinished()));
    connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onError()));
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
    runParser(url);
}


void ParserBridge::onFinished()
{
    if (selectionDialog == NULL)
        selectionDialog = new SelectionDialog;
    QByteArray output = process->readAllStandardOutput();
    title.clear();
    container.clear();
    names.clear();
    urls.clear();
    referer.clear();
    ua.clear();
    parseJson(output);

    // Error
    if (urls.isEmpty())
    {
        onError();
        return;
    }

    // Bind referer and use-agent
    if (!referer.isEmpty())
    {
        foreach (QString url, urls)
            referer_table[QUrl(url).host()] = referer.toUtf8();
    }
    if (!ua.isEmpty())
    {
        foreach (QString url, urls)
            ua_table[QUrl(url).host()] = ua.toUtf8();
    }

    // Download
    if (download)
    {
        // Build file path list
        QDir dir = QDir(Settings::downloadDir);
        QString dirname = title + '.' + container;
        if (urls.size() > 1)
        {
            if (!dir.cd(dirname))
            {
                dir.mkdir(dirname);
                dir.cd(dirname);
            }
        }
        for (int i = 0; i < names.size(); i++)
             names[i] = dir.filePath(QString(names[i]));

        for (int i = 0; i < urls.size(); i++)
             downloader->addTask(urls[i].toUtf8(), names[i], urls.size() > 1);
        QMessageBox::information(NULL, "Message", tr("Add download task successfully!"));
    }

    // Play
    else
    {
        playlist->addFileAndPlay(names[0], urls[0]);
        for (int i = 1; i < urls.size(); i++)
            playlist->addFile(names[i], urls[i]);
        res_library->close();
    }
}

void ParserBridge::onError()
{
    QMessageBox::warning(NULL, "Error",
                        "Parse failed!\nURL:" + url + "\n" +
                         QString::fromUtf8(process->readAllStandardError()));
}
