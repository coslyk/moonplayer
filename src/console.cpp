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

#include "console.h"
#include <QScrollBar>
#include <QTextCodec>

    Console::Console(QWidget *parent) : QPlainTextEdit(parent)
{
    // Show as a window
    setWindowFlag(Qt::Window, true);
    resize(600, 450);
    
    Q_ASSERT(document() != nullptr);
    document()->setMaximumBlockCount(100);
    m_process.setProcessChannelMode(QProcess::MergedChannels);
    
    // Display output
    connect(&m_process, &QProcess::readyReadStandardOutput, [=]() {
        putData(m_process.readAllStandardOutput());
    });
    
    // Open window when script runs
    connect(&m_process, &QProcess::started, [=]() {
        setWindowFlag(Qt::WindowCloseButtonHint, false);
        show();
    });
    
    // Close window when script ends
    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=]() {
        setWindowFlag(Qt::WindowCloseButtonHint, true);
        show();
    });
}

void Console::putData(const QByteArray& _data)
{
    QTextCodec *codec = QTextCodec::codecForLocale();
    Q_ASSERT(codec != nullptr);
    insertPlainText(codec->toUnicode(_data));

    QScrollBar *bar = verticalScrollBar();
    Q_ASSERT(bar != nullptr);
    bar->setValue(bar->maximum());
}

void Console::launchScript ( const QString& filePath, const QStringList& args )
{
    clear();
    m_process.start(filePath, args, QProcess::ReadOnly);
}

