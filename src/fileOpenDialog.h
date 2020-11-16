/* Copyright 2013–2017 Kullo GmbH. 
 * Copyright 2020 Yikun Liu <cos.lyk@gmail.com>
 */

// File open dialog without dependency on QtWidgets
// Original code: https://github.com/kullo/qml-file-dialog-demo

#pragma once

#include <QQuickItem>
#include <QUrl>
#include <QtGui/qpa/qplatformdialoghelper.h>

class FileOpenDialog : public QQuickItem
{
    Q_OBJECT

public:
    explicit FileOpenDialog(QQuickItem *parent = 0);
    ~FileOpenDialog();
    static bool hasNativeSupport(void);

    Q_PROPERTY(QUrl fileUrl          MEMBER fileUrl_         NOTIFY fileUrlChanged)
    Q_PROPERTY(QList<QUrl> fileUrls  MEMBER fileUrls_        NOTIFY fileUrlsChanged)
    Q_PROPERTY(QString title         MEMBER title_           NOTIFY titleChanged)
    Q_PROPERTY(bool selectMultiple   MEMBER selectMultiple_  NOTIFY selectMultipleChanged)
    Q_PROPERTY(bool selectFolder     MEMBER selectFolder_    NOTIFY selectFolderChanged)

    Q_INVOKABLE void open();
    Q_INVOKABLE void close();

signals:
    void fileUrlChanged();
    void fileUrlsChanged();
    void titleChanged();
    void accepted();
    void rejected();
    void selectMultipleChanged();
    void selectFolderChanged();

private slots:
    void accept();
    void reject();

private:
    QPlatformFileDialogHelper *init_helper();

    QPlatformFileDialogHelper *m_dlgHelper;
    QSharedPointer<QFileDialogOptions> m_options;

    QUrl fileUrl_;
    QList<QUrl> fileUrls_;
    QString title_;
    bool selectMultiple_ = false;
    bool selectFolder_ = false;

    Q_DISABLE_COPY(FileOpenDialog)
};
