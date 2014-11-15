#ifndef SETTINGS_VIDEO_H
#define SETTINGS_VIDEO_H

#include <QString>

namespace Settings {
extern QString vout;
extern bool framedrop;
extern bool doubleBuffer;
extern bool fixLastFrame;
extern bool ffodivxvdpau;
extern bool enableScreenshot;
extern bool rememberUnfinished;
extern int cacheSize;
extern int cacheMin;
}

#endif // SETTINGS_VIDEO_H
