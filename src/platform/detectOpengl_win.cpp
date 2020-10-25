/* Copyright 2013-2020 Yikun Liu <cos.lyk@gmail.com>
 *
 * This program is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see http://www.gnu.org/licenses/.
 */

#include "detectOpengl.h"
#include <QCoreApplication>
#include <QOpenGLContext>
#include <QSurfaceFormat>

    void
    detectOpenGLEarly()
{
}

void detectOpenGLLate()
{
    if (!QCoreApplication::testAttribute(Qt::AA_UseOpenGLES))
        return;

    // Workaround for broken QSGDefaultDistanceFieldGlyphCache::resizeTexture in ES 3 mode
    qputenv("QML_USE_GLYPHCACHE_WORKAROUND", QByteArrayLiteral("1"));

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
