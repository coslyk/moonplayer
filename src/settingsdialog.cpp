#include "settingsdialog.h"
#include "settings_audio.h"
#include "settings_network.h"
#include "settings_player.h"
#include "settings_video.h"
#include "settings_danmaku.h"
#include "ui_settingsdialog.h"
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QDir>
#include <QSettings>
#include <QFileDialog>
#include <QFontDialog>
#include <QInputDialog>
#include <QMessageBox>
#include "accessmanager.h"
#include "parserbase.h"
#include "platform/paths.h"
#include "selectiondialog.h"
#include "utils.h"

QString Settings::aout;
QString Settings::hwdec;
QString Settings::proxy;
QString Settings::proxyType;
QString Settings::downloadDir;
QString Settings::danmakuFont;
int Settings::port;
int Settings::maxTasks;
int Settings::volume;
int Settings::danmakuSize;
int Settings::durationScrolling;
int Settings::durationStill;
bool Settings::copyMode;
bool Settings::rememberUnfinished;
bool Settings::autoCombine;
bool Settings::proxyOnlyForParsing;
bool Settings::classicUI;
double Settings::danmakuAlpha;
Settings::URLOpenMode Settings::urlOpenMode;

SettingsDialog *settingsDialog = NULL;

using namespace Settings;

//Show settings dialog
SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    connect(this, &SettingsDialog::accepted, this, &SettingsDialog::saveSettings);
    connect(this, &SettingsDialog::rejected, this, &SettingsDialog::loadSettings);
    connect(ui->dirButton, &QPushButton::clicked, this, &SettingsDialog::onDirButton);
    connect(ui->fontPushButton, &QPushButton::clicked, this, &SettingsDialog::onFontButton);
    connect(ui->qualityButton, &QPushButton::clicked, this, &SettingsDialog::onQualityButton);

#ifdef Q_OS_LINUX
    ui->aoComboBox->addItem("pulse");
    ui->aoComboBox->addItem("alsa");
    ui->aoComboBox->addItem("oss");
    if (getenv("FLATPAK_SANDBOX_DIR"))
        ui->dirButton->setEnabled(false);
#else
    ui->hwdecComboBox->setEnabled(false); // takes effect only on Linux
#endif

    loadSettings();
    settingsDialog = this;
}

//Load settings
void SettingsDialog::loadSettings()
{
    ui->hwdecComboBox->setCurrentIndex(ui->hwdecComboBox->findText(hwdec));
    ui->aoComboBox->setCurrentIndex(ui->aoComboBox->findText(aout));
    ui->proxyTypeComboBox->setCurrentIndex(ui->proxyTypeComboBox->findText(proxyType));
    ui->urlOpenModeComboBox->setCurrentIndex((int) urlOpenMode);
    ui->proxyEdit->setText(proxy);
    ui->portEdit->setText(QString::number(port));
    ui->maxTaskSpinBox->setValue(maxTasks);
    ui->dirButton->setText(downloadDir);
    ui->rememberCheckBox->setChecked(rememberUnfinished);
    ui->combineCheckBox->setChecked(autoCombine);
    ui->copyModeCheckBox->setChecked(copyMode);
    ui->proxyParsingOnlyCheckBox->setChecked(proxyOnlyForParsing);
    ui->classicUICheckBox->setChecked(classicUI);

    ui->alphaDoubleSpinBox->setValue(danmakuAlpha);
    ui->fontPushButton->setText(danmakuFont);
    ui->fontSizeSpinBox->setValue(danmakuSize);
    ui->dmSpinBox->setValue(durationScrolling);
    ui->dsSpinBox->setValue(durationStill);
}

void SettingsDialog::onDirButton()
{
    QString dir = QFileDialog::getExistingDirectory(this);
    if (!dir.isEmpty())
        ui->dirButton->setText(dir);
}

void SettingsDialog::onFontButton()
{
    if (ui->fontPushButton->text().isEmpty())
    {
        bool ok;
        QFont font = QFontDialog::getFont(&ok, this);
        if (ok)
            ui->fontPushButton->setText(font.key().section(',', 0, 0));
    }
    else
        ui->fontPushButton->setText("");
}

void SettingsDialog::onQualityButton()
{
    static SelectionDialog *dialog = NULL;
    if (dialog == NULL)
        dialog = new SelectionDialog(this);
    QStringList list;
    list << tr("Remove all");
    QHash<QString,QString>::const_iterator i = saved_qualities.constBegin();
    while (i != saved_qualities.constEnd())
    {
        list << (i.key() + " => " + i.value());
        i++;
    }
    int index = dialog->showDialog_Index(list, tr("Saved quality selections are shown below. Select to remove:"));
    if (index == 0)
        saved_qualities.clear();
    else if (index != -1)
    {
        QString selected = list[index].section(" => ", 0, 0);
        saved_qualities.remove(selected);
    }
}

//Save settings
void SettingsDialog::saveSettings()
{
    hwdec = ui->hwdecComboBox->currentText();
    proxyType = ui->proxyTypeComboBox->currentText();
    urlOpenMode = (Settings::URLOpenMode) ui->urlOpenModeComboBox->currentIndex();
    proxy = ui->proxyEdit->text().simplified();
    port = ui->portEdit->text().toInt();
    maxTasks = ui->maxTaskSpinBox->value();
    downloadDir = ui->dirButton->text();
    rememberUnfinished = ui->rememberCheckBox->isChecked();
    autoCombine = ui->combineCheckBox->isChecked();
    copyMode = ui->copyModeCheckBox->isChecked();
    proxyOnlyForParsing = ui->proxyParsingOnlyCheckBox->isChecked();
    classicUI = ui->classicUICheckBox->isChecked();

    danmakuAlpha = ui->alphaDoubleSpinBox->value();
    danmakuFont = ui->fontPushButton->text();
    danmakuSize = ui->fontSizeSpinBox->value();
    durationScrolling = ui->dmSpinBox->value();
    durationStill = ui->dsSpinBox->value();

    access_manager->setProxy(proxyType, proxy, port);
}


SettingsDialog::~SettingsDialog()
{
    //open file
    QSettings settings("moonsoft", "moonplayer");

    settings.setValue("Player/remember_unfinished", rememberUnfinished);
    settings.setValue("Player/url_open_mode", (int) urlOpenMode);
    settings.setValue("Player/classic_ui", classicUI);
    settings.setValue("Video/copy_mode", copyMode);
    settings.setValue("Video/hwdec", hwdec);
    settings.setValue("Audio/out", aout);
    settings.setValue("Audio/volume", volume);
    settings.setValue("Net/proxy_type", proxyType);
    settings.setValue("Net/proxy", proxy);
    settings.setValue("Net/proxy_only_for_parsing", proxyOnlyForParsing);
    settings.setValue("Net/port", port);
    settings.setValue("Net/max_tasks", maxTasks);
    settings.setValue("Net/download_dir", downloadDir);
    settings.setValue("Plugins/auto_combine", autoCombine);
    settings.setValue("Danmaku/alpha", danmakuAlpha);
    settings.setValue("Danmaku/font", danmakuFont);
    settings.setValue("Danmaku/size", danmakuSize);
    settings.setValue("Danmaku/dm", durationScrolling);
    settings.setValue("Danmaku/ds", durationStill);
    saveQHashToFile(saved_qualities, "saved_qualities.txt");
    delete ui;
}

//Init settings
void initSettings()
{
    QSettings settings("moonsoft", "moonplayer");

    //create user path
    createUserPath();

    //read settings
    hwdec = settings.value("Video/hwdec", "auto").toString();
    aout = settings.value("Audio/out", "auto").toString();
    volume = settings.value("Audio/volume", 10).toInt();
    classicUI = settings.value("Player/classic_ui", false).toBool();
    rememberUnfinished = settings.value("Player/remember_unfinished", true).toBool();
    urlOpenMode = (Settings::URLOpenMode) settings.value("Player/url_open_mode", 0).toInt();
    proxyType = settings.value("Net/proxy_type", "no").toString();
    proxy = settings.value("Net/proxy").toString();
    port = settings.value("Net/port").toInt();
    proxyOnlyForParsing = settings.value("Net/proxy_only_for_parsing", false).toBool();
    maxTasks = settings.value("Net/max_tasks", 3).toInt();
    autoCombine = settings.value("Plugins/auto_combine", true).toBool();
    copyMode = settings.value("Video/copy_mode", false).toBool();
    danmakuAlpha = settings.value("Danmaku/alpha", 0.9).toDouble();
    danmakuFont = settings.value("Danmaku/font", "").toString();
    danmakuSize = settings.value("Danmaku/size", 0).toInt();
    durationScrolling = settings.value("Danmaku/dm", 0).toInt();
    durationStill = settings.value("Danmaku/ds", 6).toInt();

#ifdef Q_OS_LINUX
    if (getenv("FLATPAK_SANDBOX_DIR")) // sandboxed by Flatpak, only has access to xdg-videos directory
        downloadDir = getVideosPath();
    else
        downloadDir = settings.value("Net/download_dir", getVideosPath()).toString();
#else
    downloadDir = settings.value("Net/download_dir", getVideosPath()).toString();
#endif

    //init proxy
    access_manager->setProxy(proxyType, proxy, port);

    // Read saved quality selections
    saved_qualities = loadQHashFromFile("saved_qualities.txt");
}

