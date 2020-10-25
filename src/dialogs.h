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

#ifndef DIALOGS_H
#define DIALOGS_H

#include <QObject>
#include <functional>

    // Show dialogs in QML
    class Dialogs : public QObject
{
    Q_OBJECT

signals:
    void selectionDialogRequested(const QString& title, const QStringList& items);

public:
    Dialogs(QObject* parent = nullptr);
    virtual ~Dialogs();
    static inline Dialogs* instance() { return s_instance; }

    // Selection dialog
    void selectionDialog(const QString& title, const QStringList& items, std::function<void(int)> callback);
    Q_INVOKABLE void selectionDialogCallback(int index);

private:
    std::function<void(int)> m_selectionCb;

    static Dialogs* s_instance;
};

#endif