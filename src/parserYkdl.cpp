#include "parserYkdl.h"
#include "accessManager.h"
#include <QDir>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include "platform/paths.h"

ParserYkdl ParserYkdl::s_instance;

ParserYkdl::ParserYkdl(QObject *parent) : ParserBase(parent)
{
    m_process = new QProcess(this);
    connect(m_process, SIGNAL(finished(int)),this, SLOT(parseOutput()));
    connect(m_process, &QProcess::errorOccurred, [=](){ showErrorDialog(m_process->errorString()); });
}

ParserYkdl::~ParserYkdl()
{
    if (m_process->state() == QProcess::Running)
    {
        m_process->kill();
        m_process->waitForFinished();
    }
}

bool ParserYkdl::isSupported ( const QUrl& url )
{
#ifdef Q_OS_WIN
    QString ykdlPath = userResourcesPath() + "/ykdl-moonplayer.exe";
#else
    QString ykdlPath = userResourcesPath() + "/ykdl-moonplayer";
#endif
    if (QFile::exists(ykdlPath))
    {
        QProcess process;
        QStringList args;
        args << "--check-support" << url.toString();
        process.start(ykdlPath, args);
        process.waitForStarted(-1);
        process.waitForFinished(-1);
        return process.readAllStandardOutput().simplified() == "Url is supported.";
    } else {
        return false;
    }
}


void ParserYkdl::runParser(const QUrl &url)
{
    if (m_process->state() == QProcess::Running)
    {
        QMessageBox::warning(nullptr, "Error", tr("Another file is being parsed."));
        return;
    }

    QSettings settings;
    NetworkAccessManager::ProxyType proxyType = (NetworkAccessManager::ProxyType) settings.value("network/proxy_type").toInt();
    QString proxy = settings.value("network/proxy").toString();
    
    QStringList args;
    args << "--timeout" << "15" << "--user-agent" << DEFAULT_UA;
    
    if (!proxy.isEmpty() && proxyType == NetworkAccessManager::HTTP_PROXY)
        args << "--http-proxy" << proxy;
    else if (!proxy.isEmpty() && proxyType == NetworkAccessManager::SOCKS5_PROXY)
        args << "--socks-proxy" << proxy;
    
    args << url.toString();
    m_process->start(userResourcesPath() + "/ykdl-moonplayer", args, QProcess::ReadOnly);
}


void ParserYkdl::parseOutput()
{
    QByteArray output = m_process->readAllStandardOutput();
    QJsonParseError json_error;
    QJsonDocument document = QJsonDocument::fromJson(output, &json_error);

    if (json_error.error != QJsonParseError::NoError)
    {
        showErrorDialog(QString::fromUtf8(m_process->readAllStandardError()));
        return;
    }

    // select episode
    if (document.isArray())
    {
        QJsonArray episodes = document.array();
        QStringList titles;
        QList<QUrl> urls;
        foreach (QJsonValue item, episodes)
        {
            titles << item.toObject()["title"].toString();
            urls << item.toObject()["url"].toString();
        }
        bool ok = false;
        QString selected = QInputDialog::getItem(NULL, "Select episode",
                                             tr("Please select episode:"),
                                             titles,
                                             0,
                                             false,
                                             &ok);
        if (!ok)
            return;
        QUrl url = urls[titles.indexOf(selected)];
        runParser(url);
        return;
    }

    QJsonObject obj = document.object();
    if (obj.contains("streams"))
    {
        result.title = obj["title"].toString();
        QJsonObject streams = obj["streams"].toObject();
        QJsonObject selectedItem;
        QJsonObject::const_iterator i;

        // Select video quality
        // get all available qualities
        QStringList items;
        for (i = streams.constBegin(); i != streams.constEnd(); i++)
        {
            QString profile = i.value().toObject()["video_profile"].toString();
            items << QString("%1 (%2)").arg(i.key(), profile);
        }

        // show dialog
        int index = selectQuality(items);
        if (index == -1)
            return;
        QString selected = items[index].section(" (", 0, 0);
        selectedItem = streams[selected].toObject();

        // Write names-urls-list
        QJsonArray json_urls = selectedItem["src"].toArray();
        result.container = selectedItem["container"].toString();
        result.referer = obj["extra"].toObject()["referer"].toString();
        result.ua = obj["extra"].toObject()["ua"].toString();
        result.danmaku_url = obj["danmaku_url"].toString();

        if (json_urls.size() == 0)
        {
            showErrorDialog(QString::fromUtf8(m_process->readAllStandardError()));
            return;
        }

        // Make urls list
        for (int i = 0; i < json_urls.size(); i++)
            result.urls << json_urls[i].toString();
        finishParsing();
    }
    else
        showErrorDialog(QString::fromUtf8(m_process->readAllStandardError()));
}


