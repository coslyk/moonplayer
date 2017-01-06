#ifndef VIDEOQUALITIES
#define VIDEOQUALITIES

#include <QHash>

extern QHash<QString, QString> qualities;

void saveQualities(void);
void loadQualities(void);

#endif // VIDEOQUALITIES

