#ifndef SETTINGS_PLAYER_H
#define SETTINGS_PLAYER_H

namespace Settings {
enum URLOpenMode
{
    QUESTION,
    PLAY,
    DOWNLOAD
};
extern URLOpenMode urlOpenMode;
extern bool rememberUnfinished;
}

#endif // SETTINGS_PLAYER_H
