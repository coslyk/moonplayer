#ifndef PATHS_H
#define PATHS_H

#include <QString>

QString getAppPath(void);
QString getUserPath(void);
void createUserPath(void);

// videos path
QString getVideosPath(void);
QString getPicturesPath(void);

// Get FFmpeg's file path
QString ffmpegFilePath(void);

// parser upgrader
QString parserUpgraderPath(void);

#endif // PATHS_H
