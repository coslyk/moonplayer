#include "transformer.h"
#include "ui_transformer.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QProcess>
#include <QDir>
#include <QTimer>
#include <QMessageBox>
#include <QTextCodec>
#include <QShowEvent>
#include <iostream>
#include "sortingdialog.h"

Transformer *transformer;

TransformerItem::TransformerItem(QTreeWidget *view, const QString &file, const QString &outfile,
                                 int startPos, int endPos) :
    QTreeWidgetItem(view)
{
    setText(1, (startPos == -1 ? "(Transform) " : "(Cutting) ") + QFileInfo(file).fileName());
    this->file = file;
    this->outfile = outfile;
    this->startPos = startPos;
    this->endPos = endPos;
}

TransformerItem::TransformerItem(QTreeWidget *view, const QStringList &files, const QString &outfile) :
    QTreeWidgetItem(view)
{
    setText(1, "(Link) " + QFileInfo(outfile).fileName());
    this->files = files;
    this->outfile = outfile;
}

Transformer::Transformer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Transformer)
{
    std::cout << "Initialize transformer..." << std::endl;
    ui->setupUi(this);
    connect(ui->addPushButton, SIGNAL(clicked()), this, SLOT(onAddButton()));
    connect(ui->delPushButton, SIGNAL(clicked()), this, SLOT(onDelButton()));
    connect(ui->linkPushButton, SIGNAL(clicked()), this, SLOT(onLinkButton()));
    connect(ui->startPushButton, SIGNAL(clicked()), this, SLOT(onStartButton()));
    connect(ui->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(onListDoubleClicked(QTreeWidgetItem*)));
    process = new QProcess(this);
    sortingDialog = new SortingDialog(this);
    connect(process, SIGNAL(finished(int)), this, SLOT(onFinished(int)));
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(readOutput()));

#ifdef Q_OS_LINUX
    // Check whether mencoder is installed
    mencoder_installed = QDir("/usr/bin").exists("mencoder");
#endif
    transformer = this;
}

#ifdef Q_OS_LINUX
void Transformer::showEvent(QShowEvent *e)
{
    if (!mencoder_installed)
        QMessageBox::warning(this, "Warning",
                             tr("This function depends on mencoder, but mencoder is not installed in this computer."));
    e->accept();
}
#endif

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
    QDir dir(QFileDialog::getExistingDirectory(this, tr("Choose save directory")));
    while (!files.isEmpty()) //transform
    {
        QString file = files.takeFirst();
        QString outfile = QFileInfo(file).baseName() + "_out.mp4";
        new TransformerItem(ui->treeWidget, file, dir.filePath(outfile));
    }
}

void Transformer::addCuttingTask(const QString &filename, int startPos, int endPos) {
    QDir dir(QFileDialog::getExistingDirectory(this, tr("Choose save directory")));
    QString outfile = QFileInfo(filename).baseName() + "_clip.mp4";
    new TransformerItem(ui->treeWidget, filename, dir.filePath(outfile), startPos, endPos);
}

void Transformer::onLinkButton()
{
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Please select videos you want to link"));
    if (files.isEmpty())
        return;
    // Sort files
    sortingDialog->execDialog(files);
    if (files.isEmpty())
        return;

    QString outfile = sortingDialog->saveTo();
    new TransformerItem(ui->treeWidget, files, outfile);
}

void Transformer::onListDoubleClicked(QTreeWidgetItem *item)
{
    TransformerItem *it = static_cast<TransformerItem*>(item);
    if (hasTask() || it->files.isEmpty()) // Transforming or it is a single-video task
        return;
    sortingDialog->execDialog(it->files, it->outfile);
    if (it->files.isEmpty())
    {
        delete it;
        return;
    }
    it->outfile = sortingDialog->saveTo();
    it->setText(1, "(Link) " + QFileInfo(it->outfile).fileName());
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
    QStringList lavcopts;
    if (item)
    {
        ui->startPushButton->setEnabled(false);
        ui->addPushButton->setEnabled(false);
        ui->linkPushButton->setEnabled(false);
        ui->tabWidget->setEnabled(false);

        //read audio settings
        args.clear();
        args << "-of" << "lavf" << "-oac";
        if (ui->acodecComboBox->currentText() == "faac")
        {
            args << "lavc" << "-af" << "channels=1,lavcresample=" + ui->srateComboBox->currentText();
            lavcopts << "acodec=libfaac" << "abitrate=128";
        }
        else if (ui->acodecComboBox->currentText() == "ac3")
        {
            args << "lavc" << "-af" << "channels=1,lavcresample=" + ui->srateComboBox->currentText();
            lavcopts << "acodec=ac3" << "abitrate=96";
        }
        else
            args << "mp3lame" << "-lameopts" << "aq=7:abr:br=96";
        args << "-srate" << ui->srateComboBox->currentText();

        //read video settings
        char vfmsg[256];
        qsnprintf(vfmsg, 255, "scale=%d:%d,harddup", ui->widthSpinBox->value(), ui->heightSpinBox->value());
        args << "-vf" << vfmsg << "-ofps" << "15";

        if (ui->vcodecComboBox->currentText() == "mpeg4")
        {
            args << "-ovc" << "lavc";
            lavcopts << "vcodec=mpeg4" << ("vqscale=" + QString::number(ui->qualitySpinBox->value()));
        }
        else
            args << "-ovc" << "xvid" << "-xvidencopts" <<
                    "fixed_quant=" + QString::number(ui->qualitySpinBox->value()) + ":threads=2";

        // Set -lavcopts
        if (!lavcopts.isEmpty())
            args << "-lavcopts" << lavcopts.join(":");

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
        if (item->startPos != -1)
            a << "-ss" << QString::number(item->startPos) << "-endpos" << QString::number(item->endPos - item->startPos);
        a << item->file;
        current_rest = 0;
    }
    a << "-o" << item->outfile;
    current = item;
    process->start("mencoder", a, QProcess::ReadOnly);
    timer->start(1000);
}

void Transformer::onFinished(int status)
{
    timer->stop();
    delete current;
    current = NULL;
    if (status)
        QMessageBox::critical(this, "Mencoder ERROR", QTextCodec::codecForLocale()->toUnicode(process->readAllStandardError()));
    TransformerItem *item = static_cast<TransformerItem*>(ui->treeWidget->topLevelItem(0));
    if (item)
        start(item);
    else
    {
        ui->startPushButton->setEnabled(true);
        ui->addPushButton->setEnabled(true);
        ui->linkPushButton->setEnabled(true);
        ui->tabWidget->setEnabled(true);
    }
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
