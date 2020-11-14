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

#include "application.h"
#include "../playlistModel.h"
#include <QFileOpenEvent>

Application::Application(int &argc, char **argv) : QGuiApplication(argc, argv)
{
}

Application::~Application()
{
}

// MacOS don't use arguments to open files. Always return true here
bool Application::parseArgs()
{
     return true;
}

bool Application::event(QEvent *e)
{
    if (e->type() == QEvent::FileOpen) {
        Q_ASSERT(PlaylistModel::instance() != nullptr);
        QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(e);
        QString file = openEvent->file();
        if (file.isEmpty()) //url
        {
            QUrl url = openEvent->url();
            if (url.scheme() == QStringLiteral("moonplayer"))
                url.setScheme(QStringLiteral("http"));
            else if (url.scheme() == QStringLiteral("moonplayers"))
                url.setScheme(QStringLiteral("https"));
            PlaylistModel::instance()->addUrl(url);
        }
        else
        {
            QList<QUrl> files;
            files << QUrl::fromLocalFile(file);
            PlaylistModel::instance()->addLocalFiles(files);
        }
    }
    return QApplication::event(e);
}

