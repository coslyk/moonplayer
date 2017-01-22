#ifndef SETTINGS_PLAYER_H
#define SETTINGS_PLAYER_H

#include <QStringList>

namespace Settings
{
extern QString path;
extern QString userPath;
extern int currentSkin;
extern QStringList skinList;
extern bool autoResize;
extern bool disableSkin;
extern double uiScale;
}

#endif // SETTINGS_PLAYER_H
