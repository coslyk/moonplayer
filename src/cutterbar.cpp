#include "cutterbar.h"
#include "ui_cutterbar.h"
#include "utils.h"
#include "transformer.h"
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QTimer>
#include <QTextCodec>

CutterBar::CutterBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CutterBar)
{
    ui->setupUi(this);
    slider_pressed = false;
    connect(ui->startSlider, SIGNAL(sliderPressed()), this, SLOT(onSliderPressed()));
    connect(ui->startSlider, SIGNAL(valueChanged(int)), this, SLOT(onStartSliderChanged()));
    connect(ui->startSlider, SIGNAL(sliderReleased()), this, SLOT(onSliderReleased()));
    connect(ui->endSlider, SIGNAL(sliderPressed()), this, SLOT(onSliderPressed()));
    connect(ui->endSlider, SIGNAL(valueChanged(int)), this, SLOT(onEndSliderChanged()));
    connect(ui->endSlider, SIGNAL(sliderReleased()), this, SLOT(onSliderReleased()));
    connect(ui->cancelButton, SIGNAL(clicked()), this, SIGNAL(finished()));
    connect(ui->okButton, SIGNAL(clicked()), this, SLOT(startTask()));

    process = new QProcess(this);
    connect(process, SIGNAL(finished(int)), this, SLOT(onFinished(int)));

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateState()));
}

CutterBar::~CutterBar()
{
    delete ui;
}

void CutterBar::init(QString filename, int length, int currentPos)
{
    this->filename = filename;
    ui->startSlider->setMaximum(length);
    ui->startSlider->setValue(currentPos);
    ui->endSlider->setMaximum(length);
    ui->endSlider->setValue(currentPos + 1);
    startPos = currentPos;
    endPos = currentPos + 1;
}

void CutterBar::onSliderPressed()
{
    slider_pressed = true;
}

void CutterBar::onStartSliderChanged()
{
    startPos = pos = ui->startSlider->value();
    ui->startPosLabel->setText(secToTime(pos));
    //Show preview when the progress is changed by keyboard
    if (isVisible() && !slider_pressed)
        emit newFrame(pos);
}

void CutterBar::onEndSliderChanged()
{
    endPos = pos = ui->endSlider->value();
    ui->endPosLabel->setText(secToTime(pos));
    if (isVisible() && !slider_pressed)
        emit newFrame(pos);
}

void CutterBar::onSliderReleased()
{
    slider_pressed = false;
    //Show preview when the progress is changed by mouse
    emit newFrame(pos);
}

void CutterBar::startTask()
{
#ifdef Q_OS_LINUX
    if (!QDir("/usr/bin").exists("mencoder")) {
        QMessageBox::warning(this, "Error",
                             tr("This function depends on mencoder, but mencoder is not installed in this computer."));
        return;
    }
#endif
    if (startPos >= endPos)
    {
        QMessageBox::warning(this, "Error", tr("Time position is not valid."));
        return;
    }

    int operation = QMessageBox::information(this, "Choose", tr("What do you want to do?"),
                                             tr("Cut"),
                                             tr("Losslessly cut"));
    switch (operation)
    {
    case 0:
        transformer->addCuttingTask(filename, startPos, endPos);
        transformer->show();
        emit finished();
        break;
    case 1:
    {
        QString new_name = QString("%1_clip.%2").arg(filename.section('.', 0, -2), filename.section('.', -1));
        QStringList args;
        args << "-oac" << "copy" << "-ovc" << "copy" <<
                "-ss" << QString::number(startPos) << "-endpos" << QString::number(endPos - startPos) <<
                filename << "-o" << new_name;
        ui->okButton->setEnabled(false);
        ui->cancelButton->setEnabled(false);
        process->start("mencoder", args, QProcess::ReadOnly);
        timer->start(1000);
        break;
    }
    default: break;
    }
}

void CutterBar::updateState()
{
    QByteArray msg = process->readAllStandardOutput();
    int i = msg.lastIndexOf("%)");
    if (i >= 0)
    {
        int percentage = msg.mid(i - 2, 2).toInt();
        QString s = QString::number(percentage) + '%';
        ui->okButton->setText(s);
    }
}

void CutterBar::onFinished(int status)
{
    timer->stop();
    if (status)
        QMessageBox::critical(this, "Mencoder ERROR", QTextCodec::codecForLocale()->toUnicode(process->readAllStandardError()));
    ui->okButton->setText(tr("OK"));
    ui->okButton->setEnabled(true);
    ui->cancelButton->setEnabled(true);
    emit finished();
}
