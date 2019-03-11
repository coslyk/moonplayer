#ifndef UPGRADERDIALOG_H
#define UPGRADERDIALOG_H

#include <QDialog>
class QProcess;

namespace Ui {
class UpgraderDialog;
}

class UpgraderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpgraderDialog(QWidget *parent = 0);
    ~UpgraderDialog();

public slots:
    void runUpgrader(void);

private:
    Ui::UpgraderDialog *ui;
    QProcess *process;

private slots:
    void onReadyRead(void);
    void onFinished(void);
};

extern UpgraderDialog *upgraderDialog;

#endif // UPGRADERDIALOG_H
