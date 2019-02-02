#ifndef EXTRACTOR_H
#define EXTRACTOR_H

#include <Python.h>
#include <QByteArray>
#include <QRegularExpression>

class Extractor
{
public:
    Extractor(const QString &name);
    QString parse(const QByteArray &data); // Parse the catched data, return error string
    bool match(const QString &url);        // Check if the catched data with url can be processed

    static bool isSupported(const QString &host);  // Check if the website with host can be extracted

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
