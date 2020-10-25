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
