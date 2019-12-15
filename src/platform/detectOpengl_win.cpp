#include "detectOpengl.h"
#include <QCoreApplication>
#include <QOpenGLContext>
#include <QSurfaceFormat>

void detectOpenGLEarly()
{
}

void detectOpenGLLate()
{
    if (!QCoreApplication::testAttribute(Qt::AA_UseOpenGLES))
        return;

    // Workaround for broken QSGDefaultDistanceFieldGlyphCache::resizeTexture in ES 3 mode
    qputenv("QML_USE_GLYPHCACHE_WORKAROUND", "1");

    QList<int> versions = { 3, 2 };
    for (auto version : versions)
    {
        QSurfaceFormat fmt = QSurfaceFormat::defaultFormat();
        fmt.setMajorVersion(version);
#ifdef HAVE_OPTIMALORIENTATION
        fmt.setOption(QSurfaceFormat::UseOptimalOrientation);
#endif
        QOpenGLContext ctx;
        ctx.setFormat(fmt);
        if (ctx.create())
        {
            QSurfaceFormat::setDefaultFormat(fmt);
            break;
        }
    }
}
