#include "parser.h"
#include <QString>
#include "pyapi.h"
#include "settings_network.h"

Parser::Parser(const QString &moduleName)
{
    //load module
    module = PyImport_ImportModule(moduleName.toUtf8().constData());
    if (module == NULL)
    {
        PyErr_Print();
        exit(-1);
    }
    name = moduleName.section('_', 1);

    // Get parse() function
    parseFunc = PyObject_GetAttrString(module, "parse");
    if (parseFunc == NULL)
    {
        PyErr_Print();
        exit(-1);
    }
}

void Parser::parse(const char *url, bool is_down)
{
    int options = (Settings::quality == Settings::SUPER) ? OPT_QL_SUPER : (Settings::quality == Settings::HIGH) ? OPT_QL_HIGH : 0;
    if (is_down)
        options |= OPT_DOWNLOAD;
    call_py_func_vsi(parseFunc, url, options);
}
