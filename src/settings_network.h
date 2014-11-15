#ifndef SETTINGS_NETWORK_H
#define SETTINGS_NETWORK_H

#include <QString>

namespace Settings {
extern QString proxy;
extern QString downloadDir;
extern int port;
extern int maxTasks;
extern enum Quality {NORMAL, HIGH, SUPER} quality;
}

#endif // SETTINGS_NETWORK_H
