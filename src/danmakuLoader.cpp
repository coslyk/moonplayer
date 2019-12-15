#include "danmakuLoader.h"
#include "accessManager.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QDir>
#include <QFont>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QSettings>
#include "platform/paths.h"
#include "mpvObject.h"


DanmakuLoader DanmakuLoader::s_instance;


DanmakuLoader::DanmakuLoader(QObject *parent) :
    QObject(parent)
, m_reply(nullptr), m_process(new QProcess(this))
{
    connect(m_process, SIGNAL(finished(int)), this, SLOT(onProcessFinished()));
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
        QSettings settings;
        
        QStringList args;
        args << appResourcesPath() + "/danmaku2ass.py";

        // Output file
        m_outputFile = QDir::temp().filePath("moonplayer_danmaku.ass").toUtf8();
        args << "-o" << m_outputFile;

        // Size
        args << "-s" << QString().sprintf("%dx%d", m_width, m_height);

        // Font
        QString fontName = settings.value("danmaku/font").value<QFont>().family();
#ifdef Q_OS_MAC
        args << "-fn" << (fontName.isEmpty() ? "PingFang SC" : fontName);
#else
        args << "-fn" << (fontName.isEmpty() ? "sans-serif" : fontName);
#endif
        
        // Font size
        args << "-fs";
        int fontSize = settings.value("danmaku/font_size").toInt();
        if (fontSize)
            args << QString::number(fontSize);
        else
        {
            if (m_width > 960)
                args << "36";
            else if (m_width > 640)
                args << "32";
            else
                args << "28";
        }

        // Duration of comment display
        args << "-dm";
        int dm = settings.value("danmaku/dm").toInt();
        if (dm)
            args << QString::number(dm);
        else
        {
            if (m_width > 960)
                args << "10";
            else if (m_width > 640)
                args << "8";
            else
                args << "6";
        }

        // Duration of still danmaku
        args << "-ds" << settings.value("danmaku/ds").toString();

        // text opacity
        args << "-a" << settings.value("danmaku/alpha").toString();

        // input
        args << "/dev/stdin";

        // run
        m_process->start("python", args);
        m_process->waitForStarted(-1);
        m_process->write(m_reply->readAll());
        m_process->closeWriteChannel();
    }
    m_reply->deleteLater();
    m_reply = nullptr;
}

void DanmakuLoader::onProcessFinished()
{
    qDebug("%s", m_process->readAllStandardError().constData());
    MpvObject::instance()->addSubtitle(QUrl::fromLocalFile(m_outputFile));
}
