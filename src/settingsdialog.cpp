#include "settingsdialog.h"
#include "settings_network.h"
#include "settings_player.h"
#include "settings_plugins.h"
#include "settings_video.h"
#include "ui_settingsdialog.h"
#include "accessmanager.h"
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QDir>
#include <QSettings>
#include <QButtonGroup>
#include <QFileDialog>
#include <QMessageBox>
#include "settings_audio.h"
#include "plugins.h"

QString Settings::aout;
QString Settings::vout;
QString Settings::path;
QString Settings::proxy;
QString Settings::downloadDir;
QStringList Settings::skinList;
int Settings::currentSkin;
int Settings::port;
int Settings::cacheSize;
int Settings::cacheMin;
int Settings::maxTasks;
int Settings::volume;
bool Settings::framedrop;
bool Settings::doubleBuffer;
bool Settings::fixLastFrame;
bool Settings::ffodivxvdpau;
bool Settings::autoResize;
bool Settings::enableScreenshot;
bool Settings::softvol;
bool Settings::useSkin;
bool Settings::rememberUnfinished;
bool Settings::autoCombine;
enum Settings::Quality Settings::quality;

using namespace Settings;

//Show settings dialog
SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    connect(this, SIGNAL(accepted()), this, SLOT(saveSettings()));
    connect(this, SIGNAL(rejected()), this, SLOT(loadSettings()));
    connect(ui->dirButton, SIGNAL(clicked()), this, SLOT(onDirButton()));
    connect(ui->viewPluginsButton, SIGNAL(clicked()), this, SLOT(showPluginsMsg()));
    connect(ui->combineCheckBox, SIGNAL(toggled(bool)), this, SLOT(checkFFMPEG(bool)));

    group = new QButtonGroup(this);
    group->addButton(ui->normalRadioButton, 0);
    group->addButton(ui->highRadioButton, 1);
    group->addButton(ui->superRadioButton, 2);
    ui->skinComboBox->addItems(skinList);

#ifdef Q_OS_LINUX
    ui->aoComboBox->addItem("pulse");
    ui->aoComboBox->addItem("alsa");
    ui->aoComboBox->addItem("oss");
#endif

    loadSettings();
}

//Load settings
void SettingsDialog::loadSettings()
{
    ui->voComboBox->setCurrentIndex(ui->voComboBox->findText(vout));
    ui->aoComboBox->setCurrentIndex(ui->aoComboBox->findText(aout));
    ui->skinComboBox->setCurrentIndex(currentSkin);
    ui->skinCheckBox->setChecked(useSkin);
    ui->proxyEdit->setText(proxy);
    ui->portEdit->setText(QString::number(port));
    ui->cacheSpinBox->setValue(cacheSize);
    ui->cacheMinSpinBox->setValue(cacheMin);
    ui->maxTaskSpinBox->setValue(maxTasks);
    ui->dirButton->setText(downloadDir);
    ui->dropCheckBox->setChecked(framedrop);
    ui->doubleCheckBox->setChecked(doubleBuffer);
    ui->ffodivxvdpauCheckBox->setChecked(ffodivxvdpau);
    ui->fixCheckBox->setChecked(fixLastFrame);
    ui->resizeCheckBox->setChecked(autoResize);
    ui->screenshotCheckBox->setChecked(enableScreenshot);
    ui->softvolCheckBox->setChecked(softvol);
    ui->rememberCheckBox->setChecked(rememberUnfinished);
    ui->combineCheckBox->setChecked(autoCombine);
    switch (quality)
    {
    case NORMAL: ui->normalRadioButton->setChecked(true);break;
    case HIGH:   ui->highRadioButton->setChecked(true);  break;
    default:     ui->superRadioButton->setChecked(true); break;
    }
}

void SettingsDialog::onDirButton()
{
    QString dir = QFileDialog::getExistingDirectory(this);
    if (!dir.isEmpty())
        ui->dirButton->setText(dir);
}

//Save settings
void SettingsDialog::saveSettings()
{
    vout = ui->voComboBox->currentText();
    proxy = ui->proxyEdit->text().simplified();
    port = ui->portEdit->text().toInt();
    cacheSize = ui->cacheSpinBox->value();
    cacheMin = ui->cacheMinSpinBox->value();
    maxTasks = ui->maxTaskSpinBox->value();
    downloadDir = ui->dirButton->text();
    framedrop = ui->dropCheckBox->isChecked();
    doubleBuffer = ui->doubleCheckBox->isChecked();
    ffodivxvdpau = ui->ffodivxvdpauCheckBox->isChecked();
    fixLastFrame = ui->fixCheckBox->isChecked();
    softvol = ui->softvolCheckBox->isChecked();
    quality = (enum Quality) group->checkedId();
    currentSkin = ui->skinComboBox->currentIndex();
    useSkin = ui->skinCheckBox->isChecked();
    autoResize = ui->resizeCheckBox->isChecked();
    enableScreenshot = ui->screenshotCheckBox->isChecked();
    rememberUnfinished = ui->rememberCheckBox->isChecked();
    autoCombine = ui->combineCheckBox->isChecked();

    if (proxy.isEmpty())
        access_manager->setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
    else
        access_manager->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, proxy, port));
}


SettingsDialog::~SettingsDialog()
{
    //open file
#ifdef Q_OS_WIN
    QSettings settings("HKEY_CURRENT_USER\\Software\\moonplayer", QSettings::NativeFormat);
#else
    QSettings settings(QDir::homePath() + "/.config/moonplayer.ini", QSettings::IniFormat);
#endif
    settings.setValue("Player/current_skin", currentSkin);
    settings.setValue("Player/use_skin", useSkin);
    settings.setValue("Player/auto_resize", autoResize);
    settings.setValue("Player/screenshot", enableScreenshot);
    settings.setValue("Player/remember_unfinished", rememberUnfinished);
    settings.setValue("Video/out", vout);
    settings.setValue("Video/framedrop", framedrop);
    settings.setValue("Video/double", doubleBuffer);
    settings.setValue("Video/fixlastframe", fixLastFrame);
    settings.setValue("Video/ffodivxvdpau", ffodivxvdpau);
    settings.setValue("Audio/out", aout);
    settings.setValue("Audio/softvol", softvol);
    settings.setValue("Audio/volume", volume);
    settings.setValue("Net/proxy", proxy);
    settings.setValue("Net/port", port);
    settings.setValue("Net/cache_size", cacheSize);
    settings.setValue("Net/cache_min", cacheMin);
    settings.setValue("Net/max_tasks", maxTasks);
    settings.setValue("Net/download_dir", downloadDir);
    settings.setValue("Plugins/quality", (int) quality);
    settings.setValue("Plugins/auto_combine", autoCombine);
    delete ui;
}

//Init settings
void initSettings()
{
    //open file
#ifdef Q_OS_WIN
    QSettings settings("HKEY_CURRENT_USER\\Software\\moonplayer", QSettings::NativeFormat);
#else
    QDir dir = QDir::home();
    if (!dir.cd(".moonplayer"))
    {
        dir.mkdir(".moonplayer");
        dir.cd(".moonplayer");
    }
    if (!dir.exists("plugins"))
        dir.mkdir("plugins");
    if (!dir.exists("skins"))
        dir.mkdir("skins");

    QSettings settings(QDir::homePath() + "/.config/moonplayer.ini", QSettings::IniFormat);
#endif

    //read settings
#ifdef Q_OS_WIN
    if (QSysInfo::WindowsVersion >= QSysInfo::WV_VISTA)
        vout = settings.value("Video/out", "direct3d").toString();
    else
        vout = settings.value("Video/out", "directx").toString();
#else
    vout = settings.value("Video/out", "xv").toString();
    path = "/usr/share/moonplayer";
#endif
    framedrop = settings.value("Video/framedrop", true).toBool();
    doubleBuffer = settings.value("Video/double", true).toBool();
    fixLastFrame = settings.value("Video/fixlastframe", false).toBool();
    ffodivxvdpau = settings.value("Video/ffodivxvdpau", true).toBool();
    aout = settings.value("Audio/out", "auto").toString();
    softvol = settings.value("Audio/softvol", false).toBool();
    volume = settings.value("Audio/volume", 10).toInt();
    currentSkin = settings.value("Player/current_skin", 0).toInt();
    useSkin = settings.value("Player/use_skin", true).toBool();
    autoResize = settings.value("Player/auto_resize", true).toBool();
    enableScreenshot = settings.value("Player/screenshot", true).toBool();
    rememberUnfinished = settings.value("Player/remember_unfinished", true).toBool();
    proxy = settings.value("Net/proxy").toString();
    port = settings.value("Net/port").toInt();
    cacheSize = settings.value("Net/cache_size", 4096).toInt();
    cacheMin = settings.value("Net/cache_min", 50).toInt();
    maxTasks = settings.value("Net/max_tasks", 3).toInt();
    downloadDir = settings.value("Net/download_dir", QDir::homePath()).toString();
    quality = (Quality) settings.value("Plugins/quality", (int) SUPER).toInt();
    autoCombine = settings.value("Plugins/auto_combine", false).toBool();

    //init proxy
    if (proxy.isEmpty())
        access_manager->setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
    else
        access_manager->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, proxy, port));

    //init skins
    QDir skinDir(path);
    skinDir.cd("skins");
    skinList = skinDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
#ifdef Q_OS_LINUX
    dir.cd("skins");
    skinList.append(dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name));
#endif
    if (currentSkin >= skinList.size())
        currentSkin = 0;
}

void SettingsDialog::showPluginsMsg()
{
    QMessageBox::information(this, "Plugins", plugins_msg);
}

void SettingsDialog::checkFFMPEG(bool toggled)
{
    if (toggled)
    {
        QDir dir = QDir::home();
        dir.cd(".moonplayer");
        if (!dir.exists("ffmpeg"))
        {
            QMessageBox::warning(this, "Error", tr("FFMPEG is not installed. Please download it from") +
                                 "\n    http://johnvansickle.com/ffmpeg/\n" +
                                tr("and place file \"ffmpeg\" into ~/.moonplayer/"));
            ui->combineCheckBox->setChecked(false);
        }
    }
}
