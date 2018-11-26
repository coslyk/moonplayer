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

private:
    PyObject *module;
    PyObject *parseFunc;
    QRegularExpression urlPattern;
};

void initExtractors(void);

extern Extractor **extractors;
extern int n_extractors;

#endif // EXTRACTOR_H
