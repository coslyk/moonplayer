#ifndef SETTINGS_PLUGINS_H
#define SETTINGS_PLUGINS_H


namespace Settings {

enum VideoParser {
    YKDL,
    YOUTUBE_DL,
    SIMULATION
};

extern bool autoCombine;
extern VideoParser parser;
}

#endif // SETTINGS_PLUGINS_H
