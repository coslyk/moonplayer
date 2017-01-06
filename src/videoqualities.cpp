#include "videoqualities.h"
#include <QDir>
#include <QFile>

QHash<QString, QString> qualities;

void loadQualities()
{
#if defined(Q_OS_LINUX)
    QFile file(QDir::homePath() + "/.moonplayer/qualities");
#elif defined(Q_OS_MAC)
    QFile file(QDir::homePath() + "/Library/Application Support/MoonPlayer/qualities");
#else
#error ERROR: Unsupported system!
#endif

    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        QStringList list = QString(file.readAll()).split('\n');
        foreach (QString line, list)
        {
            if (!line.isEmpty())
            {
                QString host = line.section(' ', 0, 0);
                QString quality = line.section(' ', 1);
                qualities[host] = quality;
            }
        }
        file.close();
    }
}

void saveQualities()
{
#if defined(Q_OS_LINUX)
    QFile file(QDir::homePath() + "/.moonplayer/qualities");
#elif defined(Q_OS_MAC)
    QFile file(QDir::homePath() + "/Library/Application Support/MoonPlayer/qualities");
#else
#error ERROR: Unsupported system!
#endif

    if (file.open(QFile::WriteOnly | QFile::Text))
    {
        QHash<QString, QString>::const_iterator i;
        for (i = qualities.constBegin(); i != qualities.constEnd(); i++)
            file.write(QString("%1 %2\n").arg(i.key(), i.value()).toUtf8());
        file.close();
    }
}
