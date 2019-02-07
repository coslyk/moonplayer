#include "danmakuloader.h"
#include "accessmanager.h"
#include "settings_danmaku.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QDir>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>

#if PY_MAJOR_VERSION >=3
#define DANMAKU2ASS "danmaku2ass_py3"
#else
#define DANMAKU2ASS "danmaku2ass_py2"
#endif

DanmakuLoader::DanmakuLoader(QObject *parent) : QObject(parent)
{
    module = danmaku2assFunc = nullptr;
    reply = nullptr;
}

void DanmakuLoader::reload()
{
    load(xmlFile, width, height);
}

void DanmakuLoader::load(const QString &xmlFile, int width, int height)
{
    this->xmlFile = xmlFile;
    if (height > QApplication::desktop()->height())
    {
        this->height = QApplication::desktop()->height();
        this->width = width * QApplication::desktop()->height() / height;
    }
    else if (width > QApplication::desktop()->width())
    {
        this->height = height * QApplication::desktop()->width() / width;
        this->width = QApplication::desktop()->width();
    }
    else
    {
        this->width = width;
        this->height = height;
    }
    if (reply) //another task is running
    {
        reply->abort();
        QTimer::singleShot(0, this, SLOT(reload())); //after event loop
        return;
    }
    reply = access_manager->get(QNetworkRequest(xmlFile));
    connect(reply, &QNetworkReply::finished, this, &DanmakuLoader::onXmlDownloaded);
}



void DanmakuLoader::onXmlDownloaded()
{
    if (reply->error() == QNetworkReply::NoError)
    {
        if (danmaku2assFunc == nullptr)
        {
            if ((module = PyImport_ImportModule(DANMAKU2ASS)) == nullptr)
            {
                printPythonException();
                exit(EXIT_FAILURE);
            }
            if ((danmaku2assFunc = PyObject_GetAttrString(module, "Danmaku2ASS")) == nullptr)
            {
                printPythonException();
                exit(EXIT_FAILURE);
            }
        }

        // Output file
        QByteArray output_file = QDir::temp().filePath("moonplayer_danmaku.ass").toUtf8();

        // Font
#ifdef Q_OS_MAC
        QByteArray font = Settings::danmakuFont.isEmpty() ? "PingFang SC" : Settings::danmakuFont.toUtf8();
#else
        QByteArray font = Settings::danmakuFont.isEmpty() ? "sans-serif" : Settings::danmakuFont.toUtf8();
#endif

        // Font size
        int fs;
        if (Settings::danmakuSize)
            fs = Settings::danmakuSize;
        else
        {
            if (width > 960)
                fs = 36;
            else if (width > 640)
                fs = 32;
            else
                fs = 28;
        }

        // Duration of comment display
        int dm;
        if (Settings::durationScrolling)
            dm = Settings::durationScrolling;
        else
        {
            if (width > 960)
                dm = 9;
            else if (width > 640)
                dm = 7;
            else
                dm = 6;
        }

        // Run parser
        /* API definition:
         * def Danmaku2ASS(input_files,
         *                 input_format,
         *                 output_file,
         *                 stage_width,
         *                 stage_height,
         *                 reserve_blank=0,
         *                 font_face=_('(FONT) sans-serif')[7:],
         *                 font_size=25.0,
         *                 text_opacity=1.0,
         *                 duration_marquee=5.0,
         *                 duration_still=5.0,
         *                 comment_filter=None,
         *                 is_reduce_comments=False,
         *                 progress_callback=None)
         */
        PyObject *result = PyObject_CallFunction(danmaku2assFunc, "sssiiisdddd",
                                                 reply->readAll().constData(),
                                                 "autodetect",
                                                 output_file.constData(),
                                                 width,
                                                 height,
                                                 0,
                                                 font.constData(),
                                                 (double) fs,
                                                 Settings::danmakuAlpha,
                                                 (double) dm,
                                                 (double) Settings::durationStill
                                                 );
        if (result) // success
        {
            Py_DecRef(result);
            emit finished(output_file);
        }
        else
            printPythonException();
    }
    reply->deleteLater();
    reply = nullptr;
}
