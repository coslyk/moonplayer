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

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
    class QLocalServer;
class QLocalSocket;

/* MacOS handles file opening in a different way.
 * It doesn't use arguments, but FileOpenEvent.
 */

class Application : public QApplication
{
public:
    Application(int &argc, char **argv);
    virtual ~Application();
    bool parseArgs(void);
    
#ifdef Q_OS_MAC
protected:
    bool event(QEvent *e);
    
#else
private:
    void onNewConnection(void);
    int m_argc;
    char **m_argv;
    QLocalServer *m_server;
    QLocalSocket *m_client;
#endif
};

#endif // APPLICATION_H
