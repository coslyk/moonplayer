#include "terminal.h"
#include <QDialog>
#include <QGridLayout>
#include <qtermwidget.h>

void execShell(const QString &shellFile)
{
    QDialog *dialog = new QDialog;
    dialog->setFixedSize(QSize(600, 480));
    QGridLayout *layout = new QGridLayout(dialog);
    QTermWidget *term = new QTermWidget(0);
    layout->addWidget(term, 0, 0);
    QObject::connect(term, &QTermWidget::finished, dialog, &QDialog::accept);
    QStringList args;
    args << shellFile;
    term->setShellProgram("/bin/sh");
    term->setArgs(args);
    term->startShellProgram();
    dialog->exec();
    dialog->deleteLater();
}
