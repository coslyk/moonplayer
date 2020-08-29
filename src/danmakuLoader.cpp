#include "danmakuLoader.h"
#include "accessManager.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QDir>
#include <QFont>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <Danmaku2ASS/CommentParser.h>
#include <sstream>
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
        double alpha = settings.value(QStringLiteral("danmaku/alpha")).toDouble() / 100.0;

        // Create parser
        QByteArray source = m_reply->readAll();
        std::stringstream input(source.toStdString());

        Danmaku2ASS::CommentParser parser(input);
        parser.setResolution(m_width, m_height);
        parser.setFont(fontName.toUtf8().toStdString(), fontSize);
        parser.setDuration(dm, ds);
        parser.setAlpha(alpha);

        // Convert
        auto assBuilder = parser.convert();
        if (assBuilder != nullptr)
        {
            assBuilder->exportAssToFile(outputFile.toUtf8().toStdString());
            MpvObject::instance()->addSubtitle(QUrl::fromLocalFile(outputFile));
        }
    }
    m_reply->deleteLater();
    m_reply = nullptr;
}
