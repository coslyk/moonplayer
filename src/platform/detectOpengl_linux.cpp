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
#include <QApplication>
#include <QSurfaceFormat>
#include <QSettings>
#include <cstdio>
#include "../mpvObject.h"

    // Attempt to reuse mpv's code for detecting whether we want GLX or EGL (which
    // is tricky to do because of hardware decoding concerns). This is not pretty,
    // but quite effective and without having to duplicate too much GLX/EGL code.
    static QByteArray
    probeHwdecInterop()
{
    QByteArray result;
    mpv_handle *mpv = mpv_create();
    if (!mpv)
        return QByteArray();
    mpv_set_option_string(mpv, "hwdec-preload", "auto");
    mpv_set_option_string(mpv, "opengl-hwdec-interop", "auto");
    // Actually creating a window is required. There is currently no way to keep
    // this window hidden or invisible.
    mpv_set_option_string(mpv, "force-window", "yes");
    // As a mitigation, put the window in the top/right corner, and make it as
    // small as possible by forcing 1x1 size and removing window borders.
    mpv_set_option_string(mpv, "geometry", "1x1+0+0");
    mpv_set_option_string(mpv, "border", "no");
    if (mpv_initialize(mpv) < 0)
        return QByteArray();
    char *str = mpv_get_property_string(mpv, "hwdec-interop");
    if (str)
    {
        printf("Detected OpenGL backend: %s\n", str);
        result = str;
        mpv_free(str);
    }
    mpv_terminate_destroy(mpv);
    return result;
}

void detectOpenGLEarly()
{
    MpvObject::Hwdec hwdec = (MpvObject::Hwdec) QSettings(QStringLiteral("coslyk"), QStringLiteral("MoonPlayer")).value(QStringLiteral("video/hwdec")).toInt();
    if (hwdec == MpvObject::VAAPI)
    {
        qputenv("QT_XCB_GL_INTEGRATION", QByteArrayLiteral("xcb_egl"));
    }
    else if (hwdec == MpvObject::AUTO)
    {
        if (probeHwdecInterop() == "vaapi-egl")
        {
            qputenv("QT_XCB_GL_INTEGRATION", QByteArrayLiteral("xcb_egl"));
        }
    }
}

void detectOpenGLLate()
{
}
