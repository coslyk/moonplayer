#include "extractor.h"
#include "platform/paths.h"
#include <QDir>

Extractor **extractors = nullptr;
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
            bool ok = false;
            Extractor *extractor = new Extractor(filename.section('.', 0, 0), &ok);
            if (ok)
            {
                array[n_extractors] = extractor;
                n_extractors++;
            }
            else
            {
                delete extractor;
                qDebug("[plugin] Fails to load: %s", filename.toUtf8().constData());
            }
        }
    }
}


Extractor::Extractor(const QString &name, bool *ok)
{
    module = parseFunc = nullptr;
    module = PyImport_ImportModule(name.toUtf8().constData());
    if (module == nullptr)
    {
        printPythonException();
        *ok = false;
        return;
    }

    PyObject *hosts = PyObject_GetAttrString(module, "supported_hosts");
    if (hosts == nullptr)
    {
        printPythonException();
        *ok = false;
        return;
    }
    for (int i = 0; i < PyTuple_Size(hosts); i++)
        supportedHosts << PyString_AsQString(PyTuple_GetItem(hosts, i));
    Py_DecRef(hosts);

    parseFunc = PyObject_GetAttrString(module, "parse");
    if (parseFunc == nullptr)
    {
        printPythonException();
        *ok = false;
        return;
    }

    PyObject *url_pattern = PyObject_GetAttrString(module, "url_pattern");
    if (url_pattern == nullptr)
    {
        printPythonException();
        *ok = false;
        return;
    }
    urlPattern = QRegularExpression(PyString_AsQString(url_pattern),
                                    QRegularExpression::DotMatchesEverythingOption);
    Py_DecRef(url_pattern);
    *ok = true;
}

Extractor::~Extractor()
{
    Py_DecRef(module);
    Py_DecRef(parseFunc);
}


bool Extractor::match(const QString &url)
{
    QRegularExpressionMatch match = urlPattern.match(url);
    return match.hasMatch();
}

Extractor *Extractor::getMatchedExtractor(const QString &url)
{
    for (int i = 0; i < n_extractors; i++)
    {
        if (extractors[i]->match(url))
            return extractors[i];
    }
    return nullptr;
}


QString Extractor::parse(const QByteArray &data)
{
    PyObject *result = PyObject_CallFunction(parseFunc, "s", data.constData());
    if (result == nullptr)
        return QString("Python Exception:\n%1\n\nResponse Content:\n%2").arg(
                    fetchPythonException(), QString::fromUtf8(data));
    else
    {
        Py_DecRef(result);
        return QString();
    }
}

bool Extractor::isSupported(const QString &host)
{
    return supportedHosts.contains(host);
}
