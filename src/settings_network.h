#ifndef SETTINGS_NETWORK_H
#define SETTINGS_NETWORK_H

#include <QString>

namespace Settings {
extern QString proxyType;
extern QString proxy;
extern QString downloadDir;
extern int port;
extern int maxTasks;
extern bool proxyOnlyForParsing;
extern bool autoCombine;
}

#endif // SETTINGS_NETWORK_H
