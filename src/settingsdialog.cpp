#include "settingsdialog.h"
#include "settings_network.h"
#include "settings_plugins.h"
#include "settings_video.h"
#include "settings_danmaku.h"
#include "ui_settingsdialog.h"
#include "accessmanager.h"
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QDir>
#include <QSettings>
#include <QButtonGroup>
#include <QFileDialog>
#include <QFontDialog>
#include <QInputDialog>
#include <QMessageBox>
#include "settings_audio.h"
#include "plugin.h"
#include "utils.h"
#include "platforms.h"

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
double Settings::danmakuAlpha;
Settings::VideoParser Settings::parser;

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
    connect(ui->viewPluginsButton, &QPushButton::clicked, this, &SettingsDialog::openPluginsFolder);

#ifdef Q_OS_LINUX
    ui->aoComboBox->addItem("pulse");
    ui->aoComboBox->addItem("alsa");
    ui->aoComboBox->addItem("oss");
    if (getenv("FLATPAK_SANDBOX_DIR"))
        ui->dirButton->setEnabled(false);
#else
    ui->hwdecComboBox->setEnabled(false); // takes effect only on Linux
#endif

#ifndef MP_ENABLE_WEBKIT
    ui->parserComboBox->removeItem(2);
#endif

    loadSettings();
    settingsDialog = this;
}

//Load settings
void SettingsDialog::loadSettings()
{
    ui->hwdecComboBox->setCurrentIndex(ui->hwdecComboBox->findText(hwdec));
    ui->aoComboBox->setCurrentIndex(ui->aoComboBox->findText(aout));
    ui->parserComboBox->setCurrentIndex(parser);
    ui->proxyTypeComboBox->setCurrentIndex(ui->proxyTypeComboBox->findText(proxyType));
    ui->proxyEdit->setText(proxy);
    ui->portEdit->setText(QString::number(port));
    ui->maxTaskSpinBox->setValue(maxTasks);
    ui->dirButton->setText(downloadDir);
    ui->rememberCheckBox->setChecked(rememberUnfinished);
    ui->combineCheckBox->setChecked(autoCombine);
    ui->copyModeCheckBox->setChecked(copyMode);

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

//Save settings
void SettingsDialog::saveSettings()
{
    hwdec = ui->hwdecComboBox->currentText();
    proxyType = ui->proxyTypeComboBox->currentText();
    proxy = ui->proxyEdit->text().simplified();
    port = ui->portEdit->text().toInt();
    maxTasks = ui->maxTaskSpinBox->value();
    downloadDir = ui->dirButton->text();
    rememberUnfinished = ui->rememberCheckBox->isChecked();
    parser = (VideoParser) ui->parserComboBox->currentIndex();
    autoCombine = ui->combineCheckBox->isChecked();
    copyMode = ui->copyModeCheckBox->isChecked();

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
    settings.setValue("Video/copy_mode", copyMode);
    settings.setValue("Video/hwdec", hwdec);
    settings.setValue("Audio/out", aout);
    settings.setValue("Audio/volume", volume);
    settings.setValue("Net/proxy_type", proxyType);
    settings.setValue("Net/proxy", proxy);
    settings.setValue("Net/port", port);
    settings.setValue("Net/max_tasks", maxTasks);
    settings.setValue("Net/download_dir", downloadDir);
    settings.setValue("Plugins/auto_combine", autoCombine);
    settings.setValue("Plugins/parser", (int) parser);
    settings.setValue("Danmaku/alpha", danmakuAlpha);
    settings.setValue("Danmaku/font", danmakuFont);
    settings.setValue("Danmaku/size", danmakuSize);
    settings.setValue("Danmaku/dm", durationScrolling);
    settings.setValue("Danmaku/ds", durationStill);
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
    rememberUnfinished = settings.value("Player/remember_unfinished", true).toBool();
    proxyType = settings.value("Net/proxy_type", "no").toString();
    proxy = settings.value("Net/proxy").toString();
    port = settings.value("Net/port").toInt();
    maxTasks = settings.value("Net/max_tasks", 3).toInt();
    autoCombine = settings.value("Plugins/auto_combine", true).toBool();
    parser = (VideoParser) settings.value("Plugins/parser", 0).toInt();
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
}



void SettingsDialog::openPluginsFolder()
{
#ifdef Q_OS_WIN
    QDesktopServices::openUrl("file:///" + getUserPath() + "/plugins");
#else
    QDesktopServices::openUrl("file://" + getUserPath() + "/plugins");
#endif
}

void SettingsDialog::switchToPluginsTab()
{
    ui->tabWidget->setCurrentIndex(4);
}
