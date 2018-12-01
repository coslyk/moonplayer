#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include <Python.h>
#include <QByteArray>
#include <QRegularExpression>

class Extractor
{
public:
    Extractor(const QString &name);
    PyObject *parse(const QByteArray &data);
    bool match(const QString &url);
    static bool isSupported(const QString &host);

private:
    PyObject *module;
    PyObject *parseFunc;
    QRegularExpression urlPattern;
    static QStringList supportedHosts;
};

void initExtractors(void);

extern Extractor **extractors;
extern int n_extractors;

#endif // EXTRACTOR_H
