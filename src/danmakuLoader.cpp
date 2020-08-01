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
        // write source to tempfile
        QFile f(QDir::temp().filePath(QStringLiteral("danmaku_source")));
        if (!f.open(QFile::WriteOnly))
            return;
        f.write(m_reply->readAll());
        f.close();

        // load settings
        QSettings settings;
        
        QStringList args;
#ifndef Q_OS_WIN
        args << appResourcesPath() + QStringLiteral("/danmaku2ass.py");
#endif

        // Output file
        m_outputFile = QDir::temp().filePath(QStringLiteral("moonplayer_danmaku.ass"));
        args << QStringLiteral("-o") << m_outputFile;

        // Size
        args << QStringLiteral("-s") << QString().sprintf("%dx%d", m_width, m_height);

        // Font
        QString fontName = settings.value(QStringLiteral("danmaku/font")).value<QFont>().family();
#ifdef Q_OS_MAC
        args << QStringLiteral("-fn") << (fontName.isEmpty() ? QStringLiteral("PingFang SC") : fontName);
#else
        args << QStringLiteral("-fn") << (fontName.isEmpty() ? QStringLiteral("sans-serif") : fontName);
#endif
        
        // Font size
        args << QStringLiteral("-fs");
        int fontSize = settings.value(QStringLiteral("danmaku/font_size")).toInt();
        if (fontSize)
            args << QString::number(fontSize);
        else
        {
            if (m_width > 960)
                args << QStringLiteral("36");
            else if (m_width > 640)
                args << QStringLiteral("32");
            else
                args << QStringLiteral("28");
        }

        // Duration of comment display
        args << QStringLiteral("-dm");
        int dm = settings.value(QStringLiteral("danmaku/dm")).toInt();
        if (dm)
            args << QString::number(dm);
        else
        {
            if (m_width > 960)
                args << QStringLiteral("10");
            else if (m_width > 640)
                args << QStringLiteral("8");
            else
                args << QStringLiteral("6");
        }

        // Duration of still danmaku
        args << QStringLiteral("-ds") << settings.value(QStringLiteral("danmaku/ds")).toString();

        // text opacity
        args << QStringLiteral("-a") << QString::number(settings.value(QStringLiteral("danmaku/alpha")).toFloat() / 100.0);

        // input
        args << QDir::temp().filePath(QStringLiteral("danmaku_source"));

        // run
#ifdef Q_OS_WIN
        m_process->start(appResourcesPath() + QStringLiteral("/danmaku2ass.exe"), args);
#else
        m_process->start(QStringLiteral("python"), args);
#endif
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
