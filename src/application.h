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

#include <QGuiApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <memory>

/* MacOS handles file opening in a different way.
 * It doesn't use arguments, but FileOpenEvent.
 */

class Application : public QGuiApplication
{
public:
    Application(int &argc, char **argv);

    bool connectAnotherInstance(void);
    void sendFileLists(void);
    void createServer(void);
    void processFileLists(void);
    void processFileLists(const QByteArrayList& fileList);

protected:
    bool event(QEvent *e);

private slots:
    void onNewConnection(void);

private:
    QByteArrayList m_fileList;
    std::unique_ptr<QLocalServer> m_server;
    std::unique_ptr<QLocalSocket> m_client;
};

#endif // APPLICATION_H
