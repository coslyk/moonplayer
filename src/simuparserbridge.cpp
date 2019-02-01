#include "simuparserbridge.h"
#include "simuparser.h"
#include "selectiondialog.h"
#include <QRegularExpression>

SimuParserBridge simuParserBridge;

SimuParserBridge::SimuParserBridge(QObject *parent) :
    ParserBridge(parent)
{
    parser = NULL;
}

void SimuParserBridge::initParser()
{
    parser = new SimuParser(this);
    connect(parser, &SimuParser::parseError, this, &SimuParserBridge::showErrorDialog);
}

void SimuParserBridge::runParser(const QString &url)
{
    Q_ASSERT(parser != nullptr);
    parser->parse(url);
}

void SimuParserBridge::onParseFinished(const QVariantHash &data)
{
    parser->closeWebview();

    result.title = data["title"].toString();
    result.danmaku_url = data["danmaku_url"].toString();

    // read streams
    QVariantList streams = data["streams"].toList();
    QStringList stream_types;
    foreach (QVariant item, streams)
        stream_types << item.toHash()["type"].toString();

    // select video quality
    int selected = selectionDialog->showDialog_Index(stream_types,
                                                     tr("Please select a video quality:"));
    if (selected == -1) // no item selected
        return;

    // Set source urls
    result.urls = streams[selected].toHash()["srcs"].toStringList();

    // find out container
    if (!result.urls.isEmpty())
        result.container = QUrl(result.urls[0]).path().section('.', -1);
    finishParsing();
}
