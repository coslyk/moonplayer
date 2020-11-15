/* Copyright 2013-2020 Yikun Liu <cos.lyk@gmail.com>
 *
 * This program is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see http://www.gnu.org/licenses/.
 */

#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QUrl>

class Utils : public QObject
{
    Q_OBJECT
    
public:
    // Check MoonPlayer's update
    Q_INVOKABLE static void checkUpdate(void);
    
    // Update video parsers
    Q_INVOKABLE static void updateParser(void);
    
    // Get environment variable
    Q_INVOKABLE static QString environmentVariable(const QString& env);

    // Paths
    Q_INVOKABLE static QUrl homeLocation(void);
    Q_INVOKABLE static QUrl movieLocation(void);
    Q_INVOKABLE static QUrl musicLocation(void);
    Q_INVOKABLE static QUrl desktopLocation(void);
    Q_INVOKABLE static QUrl downloadLocation(void);
};

#endif  // UTILS_H
