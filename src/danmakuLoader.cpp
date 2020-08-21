#include "danmakuLoader.h"
#include "accessManager.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QDir>
#include <QFont>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <danmaku2ass.h>
#include "mpvObject.h"


DanmakuLoader DanmakuLoader::s_instance;


DanmakuLoader::DanmakuLoader(QObject *parent) :
    QObject(parent)
, m_reply(nullptr)
{
}



// Start
void DanmakuLoader::start(const QUrl& srcUrl, int width, int height)
{
    // Set video size
    if (height > QApplication::desktop()->height())
    {
        m_height = QApplication::desktop()->height();
        m_width = width * QApplication::desktop()->height() / height;
    }
    else if (width > QApplication::desktop()->width())
    {
        m_height = height * QApplication::desktop()->width() / width;
        m_width = QApplication::desktop()->width();
    }
    else
    {
        m_width = width;
        m_height = height;
    }
    
    //another task is running?
    if (m_reply) 
    {
        m_reply->disconnect();
        m_reply->abort();
        m_reply->deleteLater();
    }
    m_reply = NetworkAccessManager::instance()->get(QNetworkRequest(srcUrl));
    connect(m_reply, &QNetworkReply::finished, this, &DanmakuLoader::onXmlDownloaded);
}



void DanmakuLoader::onXmlDownloaded()
{
    if (m_reply->error() == QNetworkReply::NoError)
    {
        // write source to tempfile
        QString inputFile = QDir::temp().filePath(QStringLiteral("danmaku_source"));
        QFile f(inputFile);
        if (!f.open(QFile::WriteOnly))
            return;
        f.write(m_reply->readAll());
        f.close();

        // load settings
        QSettings settings;

        // Output file
        QString outputFile = QDir::temp().filePath(QStringLiteral("moonplayer_danmaku.ass"));

        // Font
        QString fontName = settings.value(QStringLiteral("danmaku/font")).value<QFont>().family();
        if (fontName.isEmpty())
        {
#ifdef Q_OS_MAC
            fontName = QStringLiteral("PingFang SC");
#else
            fontName = QStringLiteral("sans-serif");
#endif
        }
        
        // Font size
        int fontSize = settings.value(QStringLiteral("danmaku/font_size")).toInt();
        if (fontSize == 0)
        {
            if (m_width > 960)
                fontSize = 36;
            else if (m_width > 640)
                fontSize = 32;
            else
                fontSize = 28;
        }

        // Duration of comment display
        int dm = settings.value(QStringLiteral("danmaku/dm")).toInt();
        if (dm == 0)
        {
            if (m_width > 960)
                dm = 10;
            else if (m_width > 640)
                dm = 8;
            else
                dm = 6;
        }

        // Duration of still danmaku
        int ds = settings.value(QStringLiteral("danmaku/ds")).toInt();

        // text opacity
        float alpha = settings.value(QStringLiteral("danmaku/alpha")).toFloat() / 100.0;

        // run
        danmaku2ass(
            inputFile.toLocal8Bit().constData(),  // infile
            outputFile.toLocal8Bit().constData(), // outfile
            m_width,                              // width
            m_height,                             // height
            fontName.toUtf8().constData(),        // font
            fontSize,                             // fontsize
            alpha,                                // alpha
            dm,                                   // duration_marquee
            ds                                    // duration_still
        );

        // success?
        if (QFile::exists(outputFile))
        {
            MpvObject::instance()->addSubtitle(QUrl::fromLocalFile(outputFile));
        } else {
            qDebug("Fails to parse danmaku!");
        }
    }
    m_reply->deleteLater();
    m_reply = nullptr;
}
