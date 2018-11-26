#ifndef SETTINGS_PLUGINS_H
#define SETTINGS_PLUGINS_H


namespace Settings {

enum VideoParser {
    YKDL,
    YOU_GET,
    SIMULATION
};

extern bool autoCombine;
extern VideoParser parser;
}

#endif // SETTINGS_PLUGINS_H
