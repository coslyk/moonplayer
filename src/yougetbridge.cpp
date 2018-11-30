#include "yougetbridge.h"
#include "platforms.h"
#include "selectiondialog.h"
#include "settings_network.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMessageBox>
#include <QProcess>
#include <QRegularExpression>


// you-get uses User-Agent of Python3's urllib by default
static QString python3UA()
{
    static QString ua;
    if (ua.isNull())
    {
        QStringList args;
        args << "-c" << "import sys; print('Python-urllib/%d.%d' % sys.version_info[:2])";
        QProcess process;
        if (QFile::exists("/usr/bin/python3"))
            process.start("/usr/bin/python3", args, QProcess::ReadOnly);
        else if (QFile::exists("/usr/local/bin/python3"))
            process.start("/usr/local/bin/python3", args, QProcess::ReadOnly);
        else
        {
            ua = "Are you kidding me?!";
            return ua;
        }
        process.waitForFinished();
        ua = QString::fromUtf8(process.readAll().simplified());
    }
    return ua;
}



YouGetBridge you_get_bridge;

YouGetBridge::YouGetBridge(QObject *parent) : ParserBridge(parent)
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
    connect(process, SIGNAL(finished(int)),this, SLOT(parseOutput()));
}


YouGetBridge::~YouGetBridge()
{
    if (process->state() == QProcess::Running)
    {
        process->kill();
        process->waitForFinished();
    }
}


void YouGetBridge::runParser(const QString &url)
{
    if (process->state() == QProcess::Running)
    {
        QMessageBox::warning(NULL, "Error", tr("Another file is being parsed."));
        return;
    }

    QStringList args;
    args << "python3" << getAppPath() + "/you_get_patched.py";

    // http proxy
    if (!Settings::proxy.isEmpty() &&
            (Settings::proxyType == "http" ||
             (Settings::proxyType == "http_unblockcn" && !url.contains(".youtube.com"))))
        args << "--http-proxy" << (Settings::proxy + ':' + QString::number(Settings::port));

    //socks5 proxy
    else if (Settings::proxyType == "socks5" && !Settings::proxy.isEmpty())
        args << "--socks-proxy" << (Settings::proxy + ':' + QString::number(Settings::port));

    args << "-t" << "15" << "--debug" << "--json" << url;
    process->start("/usr/bin/env", args, QProcess::ReadOnly);
}


void YouGetBridge::parseOutput()
{
    QJsonParseError json_error;

    QByteArray output = process->readAllStandardOutput();
    // Output may include non-json content in the head, remove it (bug of you-get)
    output = output.mid(output.indexOf('{'));

    QJsonObject obj = QJsonDocument::fromJson(output, &json_error).object();
    if (json_error.error != QJsonParseError::NoError)
    {
        showErrorDialog(QString::fromUtf8(process->readAllStandardError()));
        return;
    }


    if (obj.contains("streams"))
    {
        result.title = obj["title"].toString();
        QJsonObject streams = obj["streams"].toObject();
        QJsonObject dash_streams = obj["dash_streams"].toObject();
        QJsonObject selectedItem;
        QJsonObject::const_iterator i;

        // Select video quality
        // get all available qualities
        QString selected;
        QStringList items;
        for (i = streams.constBegin(); i != streams.constEnd(); i++)
        {
            QString profile = i.value().toObject()["video_profile"].toString();
            items << QString("%1 (%2)").arg(i.key(), profile);
        }
        for (i = dash_streams.constBegin(); i != dash_streams.constEnd(); i++)
        {
            QString quality = i.value().toObject()["quality"].toString();
            items << QString("%1 (%2) [DASH]").arg(i.key(), quality);
        }

        // show dialog
        selected = selectionDialog->showDialog(items,
                                               tr("Please select a video quality:"));
        if (selected.isEmpty())
            return;
        selected = selected.section(' ', 0, 0);

        // Check if re-parsing is needed
        if (dash_streams.contains(selected))
        {
            selectedItem = dash_streams[selected].toObject();
            result.is_dash = true;
        }
        else
            selectedItem = streams[selected].toObject();

        result.container = selectedItem["container"].toString();
        QJsonArray json_urls = selectedItem["src"].toArray();
        result.seekable = obj["seekable"].toBool();
        result.referer = obj["referer"].toString();
        result.ua = obj["user-agent"].toString();
        if (result.ua.isEmpty())
            result.ua = python3UA();
        result.danmaku_url = obj["danmaku_url"].toString();

        if (json_urls.size() == 0)
        {
            showErrorDialog(QString::fromUtf8(process->readAllStandardError()));
            return;
        }

        // Generate urls list
        if (result.is_dash)
        {
            result.urls << json_urls[0].toString() << json_urls[1].toString();
            finishParsing();
            return;
        }

        for (int i = 0; i < json_urls.size(); i++)
            result.urls << json_urls[i].toString();
        finishParsing();
    }
    else
        showErrorDialog(QString::fromUtf8(process->readAllStandardError()));
}

