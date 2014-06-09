#include "transformer.h"
#include "ui_transformer.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QProcess>
#include <QDir>
#include <QTimer>
#include <QMessageBox>

TransformerItem::TransformerItem(QTreeWidget *view, const QString &file, const QString &outfile) :
    QTreeWidgetItem(view)
{
    setText(1, "(Transform) " + QFileInfo(file).fileName());
    this->file = file;
    this->outfile = outfile;
}

TransformerItem::TransformerItem(QTreeWidget *view, const QStringList &files, const QString &outfile) :
    QTreeWidgetItem(view)
{
    setText(1, "(Combine) " + QFileInfo(outfile).fileName());
    this->files = files;
    this->outfile = outfile;
}

Transformer::Transformer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Transformer)
{
    ui->setupUi(this);
    connect(ui->addPushButton, SIGNAL(clicked()), this, SLOT(onAddButton()));
    connect(ui->delPushButton, SIGNAL(clicked()), this, SLOT(onDelButton()));
    connect(ui->startPushButton, SIGNAL(clicked()), this, SLOT(onStartButton()));
    process = new QProcess(this);
    process->setProcessChannelMode(QProcess::MergedChannels);
    connect(process, SIGNAL(finished(int)), this, SLOT(onFinished()));
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(readOutput()));
}

Transformer::~Transformer()
{
    if (process->state() == QProcess::Running)
        process->kill();
    delete ui;
}

bool Transformer::hasTask()
{
    return ui->treeWidget->topLevelItemCount() != 0 && !ui->startPushButton->isEnabled();
}

void Transformer::onAddButton()
{
    QStringList files = QFileDialog::getOpenFileNames(this);
    if (files.isEmpty())
        return;
    if (files.size() > 1) //combine
    {
        if (QMessageBox::Yes == QMessageBox::question(this, "Combine", tr("Combine videos?"), QMessageBox::Yes, QMessageBox::No))
        {
            QString outfile = QFileDialog::getSaveFileName(this, tr("Set out file name"));
            if (outfile.isEmpty())
                return;
            if (!QFileInfo(outfile).fileName().contains('.'))
                outfile += ".mp4";
            new TransformerItem(ui->treeWidget, files, outfile);
            return;
        }
    }
    //transform
    QDir dir(QFileDialog::getExistingDirectory(this, tr("Choose save directory")));
    while (!files.isEmpty()) //transform
    {
        QString file = files.takeFirst();
        QString outfile = QFileInfo(file).baseName() + "_out.mp4";
        new TransformerItem(ui->treeWidget, file, dir.filePath(outfile));
    }
}

void Transformer::onDelButton()
{
    TransformerItem *item = static_cast<TransformerItem*>(ui->treeWidget->currentItem());
    if (item == NULL)
        return;
    if (!item->text(0).isEmpty()) //transforming
        return;
    delete item;
}

void Transformer::onStartButton()
{
    TransformerItem *item = static_cast<TransformerItem*>(ui->treeWidget->topLevelItem(0));
    if (item)
    {
        ui->startPushButton->setEnabled(false);

        //read audio settings
        args.clear();
        args << "-of" << "lavf" << "-oac";
        if (ui->acodecComboBox->currentText() == "ac3")
            args << "lavc" << "-lavcopts" << "acodec=ac3:abitrate=96";
        else
            args << "mp3lame" << "-lameopts" << "aq=7:abr:br=96";
        args << "-srate" << ui->srateComboBox->currentText();

        //read video settings
        char vfmsg[256];
        qsnprintf(vfmsg, 255, "scale=%d:%d,harddup", ui->widthSpinBox->value(), ui->heightSpinBox->value());
        args << "-vf" << vfmsg << "-ofps" << "15";

        if (ui->vcodecComboBox->currentText() == "xvid")
            args << "-ovc" << "xvid" << "-xvidencopts" <<
                    "fixed_quant=" + QString::number(ui->qualitySpinBox->value()) + ":threads=2";
        else
            args << "-ovc" << "lavc" << "-ffourcc" << "DX50" << "-lavcopts" <<
                    "vcodec=mpeg4:vqscale=" + QString::number(ui->qualitySpinBox->value());

        //start
        start(item);
    }
}

void Transformer::start(TransformerItem *item)
{
    QStringList a = args;
    prev_percentage = 0;
    if (item->files.size()) //combine
    {
        a << item->files;
        current_rest = item->files.size() - 1;
    }
    else //transform
    {
        a << item->file;
        current_rest = 0;
    }
    a << "-o" << item->outfile;
    current = item;
    process->start("mencoder", a, QProcess::ReadOnly);
    timer->start(1000);
}

void Transformer::onFinished()
{
    timer->stop();
    delete current;
    current = NULL;
    TransformerItem *item = static_cast<TransformerItem*>(ui->treeWidget->topLevelItem(0));
    if (item)
        start(item);
    else
        ui->startPushButton->setEnabled(true);
}

void Transformer::readOutput()
{
    QByteArray msg = process->readAllStandardOutput();
    int i = msg.lastIndexOf("%)");
    if (i >= 0)
    {
        int percentage = msg.mid(i - 2, 2).toInt();
        if (percentage < prev_percentage) //combine next file
            current_rest--;
        prev_percentage = percentage;
        QString s;
        s.sprintf("%d% (rest: %d)", percentage, current_rest);
        current->setText(0, s);
    }
}
