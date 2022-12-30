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
#include <QProcess>
#include <functional>

// Show dialogs in QML
class Dialogs : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList consoleOutputs MEMBER m_consoleOutputs NOTIFY consoleOutputsChanged)

public:
    Dialogs(QObject* parent = nullptr);
    virtual ~Dialogs();
    static inline Dialogs* instance() { return s_instance; }

    // Console dialog
    void consoleDialog(const QString& title, const QString& program, const QStringList& args);

    // Message dialog
    inline void messageDialog(const QString &title, const QString &message) {
        emit messageStarted(title, message);
    }

    // Open URL dialog
    inline void openUrlDialog(const QUrl& url) { emit openUrlStarted(url); }

    // Selection dialog
    void selectionDialog(const QString& title, const QStringList& items, std::function<void(int, bool)> callback,
        const QString& checkboxText);
    Q_INVOKABLE void selectionCallback(int index, bool checked);

    // Text input dialog
    void textInputDialog(const QString& title, std::function<void(const QString&)> callback, const QString& defaultValue);
    Q_INVOKABLE void textInputCallback(const QString& text);

signals:
    void consoleStarted(const QString& title);
    void consoleFinished(void);
    void consoleOutputsChanged(void);
    void messageStarted(const QString& title, const QString& message);
    void openUrlStarted(const QUrl& url);
    void selectionStarted(const QString& title, const QStringList& items, const QString& checkboxText);
    void textInputStarted(const QString& title, const QString& defaultValue);

private:
    QProcess m_process;
    QStringList m_consoleOutputs;
    std::function<void(int, bool)> m_selectionCb;
    std::function<void(const QString&)> m_textInputCb;

    static Dialogs* s_instance;
};

#endif