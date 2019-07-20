#ifndef PYAPI_H
#define PYAPI_H

/********************
 ** API for python **
 ********************/

#include "python_wrapper.h"
#include <QObject>

extern PyObject *apiModule;
void initPython(void);
bool PluginIsLoadingPage(void);

#endif // PYAPI_H
