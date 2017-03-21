#ifndef PLATFORMS_H
#define PLATFORMS_H

#include <QString>

QString getAppPath(void);
QString createUserPath(void);

//Get FFmpeg's file name
QString ffmpegFilePath(void);

#endif // PLATFORMS_H
