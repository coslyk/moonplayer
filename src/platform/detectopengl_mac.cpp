#include "detectopengl.h"
#include <mpv/client.h>
#include <QSurfaceFormat>
#include <QSettings>

void detectOpenGLEarly()
{
    // Request OpenGL 4.1 if possible on OSX, otherwise it defaults to 2.0
    // This needs to be done before we create the QGuiApplication
    //
    QSurfaceFormat format = QSurfaceFormat::defaultFormat();
    format.setMajorVersion(3);
    format.setMinorVersion(2);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);
}

void detectOpenGLLate()
{
}
