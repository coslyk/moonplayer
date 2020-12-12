/* Copyright 2013–2017 Kullo GmbH. 
 * Copyright 2020 Yikun Liu <cos.lyk@gmail.com>
 */

// File open dialog without dependency on QtWidgets
// Original code: https://github.com/kullo/qml-file-dialog-demo

#include "fileOpenDialog.h"

#include <QDebug>
#include <QQuickWindow>
#include <private/qguiapplication_p.h>
#include <QtGui/qpa/qplatformtheme.h>

FileOpenDialog::FileOpenDialog(QQuickItem *parent) :
    QQuickItem(parent),
    m_dlgHelper(init_helper()),
    m_options(QFileDialogOptions::create())
{
    Q_ASSERT(m_dlgHelper != nullptr);
    connect(m_dlgHelper, &QPlatformFileDialogHelper::accept,
            this, &FileOpenDialog::accept);
    connect(m_dlgHelper, &QPlatformFileDialogHelper::reject,
            this, &FileOpenDialog::reject);
}


FileOpenDialog::~FileOpenDialog()
{
    if (m_dlgHelper)
        m_dlgHelper->hide();
    delete m_dlgHelper;
}


bool FileOpenDialog::hasNativeSupport()
{
    return QGuiApplicationPrivate::platformTheme()->usePlatformNativeDialog(QPlatformTheme::FileDialog);
}


QPlatformFileDialogHelper *FileOpenDialog::init_helper()
{
    return static_cast<QPlatformFileDialogHelper *>(
        QGuiApplicationPrivate::platformTheme()->createPlatformDialogHelper(QPlatformTheme::FileDialog));
}


void FileOpenDialog::open()
{
    QQuickItem *parent = this->parentItem();
    Q_ASSERT(parent);

    QQuickWindow *window = parent->window();
    Q_ASSERT(window);
    
    if (selectFolder_)
        m_options->setFileMode(QFileDialogOptions::DirectoryOnly);
    else if (selectMultiple_)
        m_options->setFileMode(QFileDialogOptions::ExistingFiles);
    else
        m_options->setFileMode(QFileDialogOptions::ExistingFile);
    m_options->setAcceptMode(QFileDialogOptions::AcceptOpen);
    m_options->setWindowTitle(title_);

    m_dlgHelper->setDirectory(QUrl::fromLocalFile(QDir::homePath()));
    m_dlgHelper->setOptions(m_options);

    Qt::WindowFlags flags = Qt::Dialog;
    if (!title_.isEmpty())
        flags |= Qt::WindowTitleHint;

    m_dlgHelper->show(flags, Qt::WindowModal, window);
}


void FileOpenDialog::close()
{
    m_dlgHelper->hide();
}


void FileOpenDialog::accept()
{
    m_dlgHelper->hide();

    QList<QUrl> selectedUrls = m_dlgHelper->selectedFiles();
    if (!selectedUrls.empty())
    {
        if (selectedUrls.size() == 1)
            fileUrl_ = selectedUrls.at(0);
        else
            fileUrl_ = QUrl();
        emit fileUrlChanged();

        fileUrls_ = selectedUrls;
        emit fileUrlsChanged();
    }

    emit accepted();
}


void FileOpenDialog::reject()
{
    m_dlgHelper->hide();
    emit rejected();
}
