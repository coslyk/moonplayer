#include "upgraderdialog.h"
#include "ui_upgraderdialog.h"
#include <QProcess>
#include "python_wrapper.h"
#include "platform/paths.h"

UpgraderDialog *upgraderDialog = NULL;

UpgraderDialog::UpgraderDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UpgraderDialog)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowCloseButtonHint, false);
    process = new QProcess(this);
    process->setProcessChannelMode(QProcess::MergedChannels);
    connect(process, &QProcess::readyReadStandardOutput, this, &UpgraderDialog::onReadyRead);
    connect(process, SIGNAL(finished(int)), this, SLOT(onFinished()));
    upgraderDialog = this;
}

UpgraderDialog::~UpgraderDialog()
{
    delete ui;
}

void UpgraderDialog::runUpgrader()
{
    if (process->state() == QProcess::Running)
        return;
    QStringList args;
    args << (getAppPath() + "/plugins/upgrade_extras.py");
    ui->buttonBox->setEnabled(false);
    ui->textEdit->clear();
    process->start(PYTHON_BIN, args, QProcess::ReadOnly);
    exec();
}

void UpgraderDialog::onReadyRead()
{
    while (process->canReadLine())
        ui->textEdit->append(QString::fromUtf8(process->readLine()).simplified());
}

void UpgraderDialog::onFinished()
{
    ui->buttonBox->setEnabled(true);
}
