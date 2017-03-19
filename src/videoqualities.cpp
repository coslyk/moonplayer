#include "videoqualities.h"
#include <QDir>
#include <QFile>
#include "settings_player.h"

QHash<QString, QString> qualities;

void loadQualities()
{
    QFile file(Settings::userPath + "/qualities");
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
    QFile file(Settings::userPath + "/qualities");
    if (file.open(QFile::WriteOnly | QFile::Text))
    {
        QHash<QString, QString>::const_iterator i;
        for (i = qualities.constBegin(); i != qualities.constEnd(); i++)
            file.write(QString("%1 %2\n").arg(i.key(), i.value()).toUtf8());
        file.close();
    }
}
