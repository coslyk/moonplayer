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

#ifndef CONSOLE_H
#define CONSOLE_H

#include <QPlainTextEdit>
#include <QProcess>

/* Open a console window and launch scripts */
class Console : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit Console(QWidget *parent = nullptr);
    void launchScript(const QString& filePath, const QStringList& args = QStringList());
    
private:
    void putData(const QByteArray& _data);
    QProcess m_process;
};

#endif // CONSOLE_H
