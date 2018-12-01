#include "extractor.h"
#include "platforms.h"
#include <QDir>

Extractor **extractors = NULL;
int n_extractors = 0;


void initExtractors()
{
    static Extractor *array[128];
    extractors = array;

    QDir pluginsDir = QDir(getUserPath() + "/plugins");
    QStringList list = pluginsDir.entryList(QDir::Files, QDir::Name);

    while (!list.isEmpty())
    {
        QString filename = list.takeFirst();
        if (filename.startsWith("extractor_") && filename.endsWith(".py"))
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
