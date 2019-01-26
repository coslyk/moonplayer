#include "extractor.h"
#include "platforms.h"
#include "utils.h"
#include <QDir>

Extractor **extractors = NULL;
int n_extractors = 0;
QStringList Extractor::supportedHosts;

void initExtractors()
{
    static Extractor *array[128];
    extractors = array;

    QDir pluginsDir = QDir(getUserPath() + "/plugins");
    QStringList list = pluginsDir.entryList(QDir::Files, QDir::Name);

    while (!list.isEmpty())
    {
        QString filename = list.takeFirst();
        if (filename.startsWith("ext_") && filename.endsWith(".py"))
        {
            array[n_extractors] = new Extractor(filename.section('.', 0, 0));
            n_extractors++;
        }
    }
}


Extractor::Extractor(const QString &name)
{
    module = PyImport_ImportModule(name.toUtf8().constData());
    if (module == nullptr)
    {
        PyErr_Print();
        exit(EXIT_FAILURE);
    }

    PyObject *hosts = PyObject_GetAttrString(module, "supported_hosts");
    if (hosts == nullptr)
    {
        PyErr_Print();
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < PyTuple_Size(hosts); i++)
        supportedHosts << PyString_AsQString(PyTuple_GetItem(hosts, i));
    Py_DecRef(hosts);

    parseFunc = PyObject_GetAttrString(module, "parse");
    if (parseFunc == nullptr)
    {
        PyErr_Print();
        exit(EXIT_FAILURE);
    }

    PyObject *url_pattern = PyObject_GetAttrString(module, "url_pattern");
    if (url_pattern == nullptr)
    {
        PyErr_Print();
        exit(EXIT_FAILURE);
    }
    urlPattern = QRegularExpression(QString::fromUtf8(PyString_AsString(url_pattern)),
                                    QRegularExpression::DotMatchesEverythingOption);
}


bool Extractor::match(const QString &url)
{
    QRegularExpressionMatch match = urlPattern.match(url);
    return match.hasMatch();
}


PyObject *Extractor::parse(const QByteArray &data)
{
    return PyObject_CallFunction(parseFunc, "s", data.constData());
}

bool Extractor::isSupported(const QString &host)
{
    return supportedHosts.contains(host);
}
