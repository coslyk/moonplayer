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

#include "fontDialog.h"

#include <QQuickWindow>
#include <private/qguiapplication_p.h>
#include <QtGui/qpa/qplatformtheme.h>
#include "mpvObject.h"

FontDialog::FontDialog(QQuickItem *parent) :
    QQuickItem(parent),
    m_dlgHelper(init_helper()),
    m_options(QFontDialogOptions::create())
{
    Q_ASSERT(m_dlgHelper != nullptr);
    connect(m_dlgHelper, &QPlatformFontDialogHelper::accept,
            this, &FontDialog::accept);
    connect(m_dlgHelper, &QPlatformFontDialogHelper::reject,
            this, &FontDialog::reject);
}


FontDialog::~FontDialog()
{
    if (m_dlgHelper)
        m_dlgHelper->hide();
    delete m_dlgHelper;
}


bool FontDialog::hasNativeSupport()
{
    return QGuiApplicationPrivate::platformTheme()->usePlatformNativeDialog(QPlatformTheme::FontDialog);
}


QPlatformFontDialogHelper *FontDialog::init_helper()
{
    return static_cast<QPlatformFontDialogHelper *>(
        QGuiApplicationPrivate::platformTheme()->createPlatformDialogHelper(QPlatformTheme::FontDialog));
}


void FontDialog::open()
{
    // Pause video before showing dialog
    Q_ASSERT(MpvObject::instance() != nullptr);
    MpvObject::instance()->pause();
    
    // Get parent window
    QQuickItem *parent = this->parentItem();
    Q_ASSERT(parent);

    QQuickWindow *window = parent->window();
    Q_ASSERT(window);

    // Set window title
    m_options->setWindowTitle(title_);
    m_dlgHelper->setOptions(m_options);

    Qt::WindowFlags flags = Qt::Dialog;
    if (!title_.isEmpty())
        flags |= Qt::WindowTitleHint;

    // Set font
    if (!family_.isEmpty())
    {
        m_dlgHelper->setCurrentFont(QFont(family_, 12));
    }

    m_dlgHelper->show(flags, Qt::WindowModal, window);
}


void FontDialog::close()
{
    m_dlgHelper->hide();
}


void FontDialog::accept()
{
    m_dlgHelper->hide();

    QFont font = m_dlgHelper->currentFont();
    family_ = font.family();
    emit familyChanged();
    emit accepted();
}

void FontDialog::reject()
{
    m_dlgHelper->hide();
    emit rejected();
}
