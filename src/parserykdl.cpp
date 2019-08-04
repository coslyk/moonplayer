#include "parserykdl.h"
#include "accessmanager.h"
#include "platform/paths.h"
#include "python_wrapper.h"
#include "settings_network.h"
#include <QDir>
#include <QGridLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QLabel>
#include <QMessageBox>
#include <QProcess>

ParserYkdl *parser_ykdl;

ParserYkdl::ParserYkdl(QObject *parent) : ParserBase(parent)
{
    process = new QProcess(this);
    connect(process, SIGNAL(finished(int)),this, SLOT(parseOutput()));
    msgWindow = NULL;
}

ParserYkdl::~ParserYkdl()
{
    if (process->state() == QProcess::Running)
    {
        process->kill();
        process->waitForFinished();
    }
}

bool ParserYkdl::isSupported(const QString &host)
{
    QDir extractorsDir(getUserPath() + "/ykdl/ykdl/extractors");
    QStringList extractors = extractorsDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
    QString shortName = host.section('.', -2, -2);
    if (shortName == "com" || shortName == "net" || shortName == "org")
        shortName = host.section('.', -3, -3);
    return extractors.contains(shortName) || extractors.contains(shortName + ".py");
}


void ParserYkdl::runParser(const QString &url)
{
    if (process->state() == QProcess::Running)
    {
        QMessageBox::warning(NULL, "Error", tr("Another file is being parsed."));
        return;
    }
    if (msgWindow == NULL)
    {
        msgWindow = new QWidget;
        msgWindow->setWindowFlags(msgWindow->windowFlags() | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        QLabel *label = new QLabel(tr("Parsing with Ykdl..."));
        QGridLayout *layout = new QGridLayout(msgWindow);
        layout->addWidget(label);
    }

    QStringList args;
    args << (getAppPath() + "/plugins/ykdl_patched.py");
    args << "--timeout" << "15" << "--user-agent" << DEFAULT_UA;
    if (!Settings::proxy.isEmpty() && Settings::proxyType == "http")
        args << "--http-proxy" << (Settings::proxy + ':' + QString::number(Settings::port));
    else if (!Settings::proxy.isEmpty() && Settings::proxyType == "socks5")
        args << "--socks-proxy" << (Settings::proxy + ':' + QString::number(Settings::port));

    args << url;
    process->start(PYTHON_BIN, args, QProcess::ReadOnly);
    msgWindow->show();
}


void ParserYkdl::parseOutput()
{
    msgWindow->close();
    QByteArray output = process->readAllStandardOutput();
    QJsonParseError json_error;
    QJsonDocument document = QJsonDocument::fromJson(output, &json_error);

    if (json_error.error != QJsonParseError::NoError)
    {
        showErrorDialog(QString::fromUtf8(process->readAllStandardError()));
        return;
    }

    // select episode
    if (document.isArray())
    {
        QJsonArray episodes = document.array();
        QStringList titles, urls;
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
        QString url = urls[titles.indexOf(selected)];
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
            showErrorDialog(QString::fromUtf8(process->readAllStandardError()));
            return;
        }

        // Make urls list
        for (int i = 0; i < json_urls.size(); i++)
            result.urls << json_urls[i].toString();
        finishParsing();
    }
    else
        showErrorDialog(QString::fromUtf8(process->readAllStandardError()));
}


