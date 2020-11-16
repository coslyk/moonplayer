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

#pragma once

#include <QQuickItem>
#include <QtGui/qpa/qplatformdialoghelper.h>

// FontDialog without dependency on QtWidgets
class FontDialog : public QQuickItem
{
    Q_OBJECT

public:
    explicit FontDialog(QQuickItem *parent = 0);
    ~FontDialog();
    static bool hasNativeSupport(void);

    Q_PROPERTY(QString title   MEMBER title_   NOTIFY titleChanged)
    Q_PROPERTY(QString family  MEMBER family_  NOTIFY familyChanged)

    Q_INVOKABLE void open();
    Q_INVOKABLE void close();

signals:
    void familyChanged();
    void titleChanged();
    void accepted();
    void rejected();

private slots:
    void accept();
    void reject();

private:
    QPlatformFontDialogHelper *init_helper();

    QPlatformFontDialogHelper *m_dlgHelper;
    QSharedPointer<QFontDialogOptions> m_options;
    QString title_;
    QString family_;

    Q_DISABLE_COPY(FontDialog)
};
