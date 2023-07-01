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

#ifndef DOWNLOADERABSTRACTITEM_H
#define DOWNLOADERABSTRACTITEM_H

#include <QObject>
#include <QUrl>
#include <QtQml/qqmlregistration.h>

class DownloaderAbstractItem : public QObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(DownloaderItem)
    QML_UNCREATABLE("Access to enums & flags only")
    Q_PROPERTY(QString name      READ name        NOTIFY nameChanged)
    Q_PROPERTY(QString filePath  READ filePath    NOTIFY filePathChanged)
    Q_PROPERTY(QUrl danmakuUrl   READ danmakuUrl  NOTIFY danmakuUrlChanged)
    Q_PROPERTY(State state       READ state       NOTIFY stateChanged)
    Q_PROPERTY(int progress      READ progress    NOTIFY progressChanged)
    
    
public:
    enum State {WAITING, PAUSED, DOWNLOADING, FINISHED, ERROR, CANCELED};
    Q_ENUM(State);
    
    DownloaderAbstractItem(const QString &filepath, const QUrl &danmakuUrl = QUrl(), QObject *parent = nullptr);
    virtual ~DownloaderAbstractItem();
    Q_INVOKABLE virtual void pause(void) = 0;
    Q_INVOKABLE virtual void start(void) = 0;
    Q_INVOKABLE virtual void stop(void) = 0;
    
    inline QString name() { return m_name; }
    inline QString filePath() { return m_filePath; }
    inline QUrl danmakuUrl() { return m_danmakuUrl; }
    inline State state() { return m_state; }
    inline int progress() { return m_progress; }
    
protected:
    void setName(const QString& name);
    void setFilePath(const QString& filePath);
    void setState(State state);
    void setProgress(int progress);
    
signals:
    void danmakuUrlChanged(void);
    void nameChanged(void);
    void filePathChanged(void);
    void stateChanged(void);
    void progressChanged(void);

private:
    QString m_name;
    QString m_filePath;
    QUrl m_danmakuUrl;
    State m_state;
    int m_progress;
};

#endif // DOWNLOADERABSTRACTITEM_H
