#include "terminal.h"

void execShell(const QString &shellFile)
{
    system(("open -a Terminal.app '" + shellFile + '\'').toUtf8().constData());
}
