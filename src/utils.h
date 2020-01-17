#ifndef UTILS_H
#define UTILS_H

#include <QObject>

class Utils: public QObject
{
    Q_OBJECT
    
public:
    // Check MoonPlayer's update
    Q_INVOKABLE static void checkUpdate(void);
    
    // Update video parsers
    Q_INVOKABLE static void updateParser(void);
    
    // Get environment variable
    Q_INVOKABLE static QString environmentVariable(const QString& env);
};

#endif  // UTILS_H
