#include "simuparserbridge.h"
#include "simuparser.h"
#include "utils.h"
#include "selectiondialog.h"
#include <QRegularExpression>

SimuParserBridge simuParserBridge;

SimuParserBridge::SimuParserBridge(QObject *parent) :
    ParserBridge(parent)
{
    parser = NULL;
}

void SimuParserBridge::runParser(const QString &url)
{
    if (parser == NULL)
    {
        parser = new SimuParser(this);
        connect(parser, &SimuParser::parseFinished, this, &SimuParserBridge::onParseFinished);
    }
    parser->parse(url);
}

void SimuParserBridge::onParseFinished(PyObject *dict)
{
    PyObject *obj = PyDict_GetItemString(dict, "title");
    result.title = PyString_AsQString(obj);// replace illegal chars in title with .
    static QRegularExpression illegalChars("[\\\\/]");
    result.title.replace(illegalChars, ".");

    // read streams
    obj = PyDict_GetItemString(dict, "streams");
    QStringList stream_types;
    QHash<QString, PyObject*> srcs;
    for (int i = 0; i < PyList_Size(obj); i++)
    {
        PyObject *item = PyList_GetItem(obj, i);
        QString type = PyString_AsQString(PyDict_GetItemString(item, "type"));
        stream_types << type;
        srcs[type] = PyDict_GetItemString(item, "srcs");
    }

    // select video quality
    QString selected = selectionDialog->showDialog(stream_types,
                                               tr("Please select a video quality:"));
    if (selected.isEmpty())
        return;
    obj = srcs[selected];
    for (int i = 0; i < PyList_Size(obj); i++)
    {
        PyObject *item = PyList_GetItem(obj, i);
        result.names << QString("%1_%2.%3").arg(result.title, QString::number(i), result.container);
        result.urls << PyString_AsQString(item);
    }

    // find out container
    result.container = QUrl(result.urls[0]).path().section('.', -1);

    // user agent
    result.ua = "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) "
                "Ubuntu Chromium/71.0.3578.20 "
                "Chrome/71.0.3578.20 Safari/537.36";

    finishParsing();
}
