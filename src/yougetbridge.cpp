#include "yougetbridge.h"
#include "platforms.h"
#include "selectiondialog.h"
#include "settings_network.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QProcess>
#include <QRegularExpression>

YouGetBridge you_get_bridge;

YouGetBridge::YouGetBridge(QObject *parent) : ParserBridge(parent)
{
}

void YouGetBridge::runParser(const QString &url)
{
    QStringList args;
    args << "python3" << yougetFilePath();
    if (Settings::proxyType != "no" && !Settings::proxy.isEmpty())
    {
        args << (Settings::proxyType == "socks5" ? "--socks-proxy" : "--http-proxy");
        args << (Settings::proxy + ':' + QString::number(Settings::port));
    }
    args << "-t" << "10" << "--json" << url;
    process->start("/usr/bin/env", args, QProcess::ReadOnly);
}


void YouGetBridge::parseOutput(const QByteArray &jsonData)
{
    QJsonParseError json_error;

    // Output may include non-json content in the head, remove it
    QByteArray output = jsonData.mid(jsonData.indexOf('{'));

    QJsonObject obj = QJsonDocument::fromJson(output, &json_error).object();
    if (json_error.error != QJsonParseError::NoError)
        return;


    if (obj.contains("streams"))
    {
        title = obj["title"].toString();
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
            is_dash = true;
        }
        else
            selectedItem = streams[selected].toObject();

        container = selectedItem["container"].toString();
        QJsonArray json_urls = selectedItem["src"].toArray();
        seekable = obj["seekable"].toBool();
        referer = obj["referer"].toString();
        ua = obj["user-agent"].toString();
        danmaku_url = obj["danmaku_url"].toString();

        if (json_urls.size() == 0)
            return;

        // replace illegal chars in title with .
        static QRegularExpression illegalChars("[\\\\/]");
        title.replace(illegalChars, ".");

        // Generate names-urls-list
        if (is_dash)
        {
            names << ("video." + container) << ("audio." + container);
            urls << json_urls[0].toString() << json_urls[1].toString();
            return;
        }

        for (int i = 0; i < json_urls.size(); i++)
        {
            names << QString("%1_%2.%3").arg(title, QString::number(i), container);
            urls << json_urls[i].toString();
        }
    }
}

