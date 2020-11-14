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

#include "dialogs.h"

Dialogs *Dialogs::s_instance = nullptr;

Dialogs::Dialogs(QObject* parent) : QObject(parent)
{
    Q_ASSERT(s_instance == nullptr);
    s_instance = this;

    // Init QProcess for the console dialog
    m_process.setProcessChannelMode(QProcess::MergedChannels);
    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Dialogs::consoleFinished);
    connect(&m_process, &QProcess::errorOccurred, this, &Dialogs::consoleFinished);
    connect(&m_process, &QProcess::readyReadStandardOutput, [this]() {
        while (m_process.canReadLine())
        {
            m_consoleOutputs << QString::fromLocal8Bit(m_process.readLine());
        }
        emit consoleOutputsChanged();
    });
}

Dialogs::~Dialogs()
{
    s_instance = nullptr;
}


// Console dialog
void Dialogs::consoleDialog(const QString& title, const QString& program, const QStringList& args)
{
    if (m_process.state() == QProcess::Running)
    {
        return;
    }
    m_consoleOutputs.clear();
    emit consoleOutputsChanged();
    m_process.start(program, args, QIODevice::ReadOnly);
    emit consoleStarted(title);
}


// Selection dialog
void Dialogs::selectionDialog(const QString &title, const QStringList& items, std::function<void(int)> callback)
{
    m_selectionCb = callback;
    emit selectionStarted(title, items);
}

void Dialogs::selectionDialogCallback(int index)
{
    m_selectionCb(index);
}