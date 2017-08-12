#include "settingsdialog.h"
#include "settings_network.h"
#include "settings_player.h"
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
QString Settings::downloadDir;
QString Settings::danmakuFont;
QStringList Settings::skinList;
int Settings::currentSkin;
int Settings::port;
int Settings::cacheSize;
int Settings::maxTasks;
int Settings::volume;
int Settings::danmakuSize;
int Settings::durationScrolling;
int Settings::durationStill;
bool Settings::autoResize;
bool Settings::disableSkin;
bool Settings::rememberUnfinished;
bool Settings::autoCombine;
bool Settings::autoCloseWindow;
double Settings::danmakuAlpha;
double Settings::uiScale;

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
    connect(ui->fontPushButton, &QPushButton::clicked, this, &SettingsDialog::onFontButton);
    connect(ui->viewPluginsButton, SIGNAL(clicked()), this, SLOT(showPluginsMsg()));

    ui->skinComboBox->addItems(skinList);

    setMinimumSize(minimumSize() * uiScale);

#ifdef Q_OS_LINUX
    ui->aoComboBox->addItem("pulse");
    ui->aoComboBox->addItem("alsa");
    ui->aoComboBox->addItem("oss");
#else
    ui->hwdecComboBox->setEnabled(false); // takes effect only on Linux
#endif

    loadSettings();
}

//Load settings
void SettingsDialog::loadSettings()
{
    ui->hwdecComboBox->setCurrentIndex(ui->hwdecComboBox->findText(hwdec));
    ui->aoComboBox->setCurrentIndex(ui->aoComboBox->findText(aout));
    ui->skinComboBox->setCurrentIndex(currentSkin);
    ui->proxyEdit->setText(proxy);
    ui->portEdit->setText(QString::number(port));
    ui->cacheSpinBox->setValue(cacheSize);
    ui->maxTaskSpinBox->setValue(maxTasks);
    ui->dirButton->setText(downloadDir);
    ui->resizeCheckBox->setChecked(autoResize);
    ui->disableSkinCheckBox->setChecked(disableSkin);
    ui->rememberCheckBox->setChecked(rememberUnfinished);
    ui->combineCheckBox->setChecked(autoCombine);
    ui->autoCloseWindowCheckBox->setChecked(autoCloseWindow);

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
    proxy = ui->proxyEdit->text().simplified();
    port = ui->portEdit->text().toInt();
    cacheSize = ui->cacheSpinBox->value();
    maxTasks = ui->maxTaskSpinBox->value();
    downloadDir = ui->dirButton->text();
    currentSkin = ui->skinComboBox->currentIndex();
    autoResize = ui->resizeCheckBox->isChecked();
    disableSkin = ui->disableSkinCheckBox->isChecked();
    rememberUnfinished = ui->rememberCheckBox->isChecked();
    autoCombine = ui->combineCheckBox->isChecked();
    autoCloseWindow = ui->autoCloseWindowCheckBox->isChecked();

    danmakuAlpha = ui->alphaDoubleSpinBox->value();
    danmakuFont = ui->fontPushButton->text();
    danmakuSize = ui->fontSizeSpinBox->value();
    durationScrolling = ui->dmSpinBox->value();
    durationStill = ui->dsSpinBox->value();

    if (proxy.isEmpty())
        access_manager->setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
    else
        access_manager->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, proxy, port));
}


SettingsDialog::~SettingsDialog()
{
    //open file
    QSettings settings("moonsoft", "moonplayer");

    settings.setValue("Player/current_skin", currentSkin);
    settings.setValue("Player/auto_resize", autoResize);
    settings.setValue("Player/disable_skin", disableSkin);
    settings.setValue("Player/remember_unfinished", rememberUnfinished);
    settings.setValue("Video/hwdec", hwdec);
    settings.setValue("Audio/out", aout);
    settings.setValue("Audio/volume", volume);
    settings.setValue("Net/proxy", proxy);
    settings.setValue("Net/port", port);
    settings.setValue("Net/cache_size", cacheSize);
    settings.setValue("Net/max_tasks", maxTasks);
    settings.setValue("Net/download_dir", downloadDir);
    settings.setValue("Plugins/auto_combine", autoCombine);
    settings.setValue("Plugins/auto_close_window", autoCloseWindow);
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
    currentSkin = settings.value("Player/current_skin", 0).toInt();
    autoResize = settings.value("Player/auto_resize", true).toBool();
    disableSkin = settings.value("Player/disable_skin", false).toBool();
    rememberUnfinished = settings.value("Player/remember_unfinished", true).toBool();
    proxy = settings.value("Net/proxy").toString();
    port = settings.value("Net/port").toInt();
    cacheSize = settings.value("Net/cache_size", 4096).toInt();
    maxTasks = settings.value("Net/max_tasks", 3).toInt();
    downloadDir = settings.value("Net/download_dir", QDir::homePath()).toString();
    autoCombine = settings.value("Plugins/auto_combine", false).toBool();
    autoCloseWindow = settings.value("Plugins/auto_close_window", true).toBool();
    danmakuAlpha = settings.value("Danmaku/alpha", 0.9).toDouble();
    danmakuFont = settings.value("Danmaku/font", "").toString();
    danmakuSize = settings.value("Danmaku/size", 0).toInt();
    durationScrolling = settings.value("Danmaku/dm", 0).toInt();
    durationStill = settings.value("Danmaku/ds", 6).toInt();
#ifdef Q_OS_MAC
    uiScale = 1.0;
#else
    uiScale = qApp->desktop()->logicalDpiX() / 96.0;
#endif

    //init proxy
    if (proxy.isEmpty())
        access_manager->setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
    else
        access_manager->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, proxy, port));

    //init skins
    QDir skinDir(getAppPath());
    skinDir.cd("skins");
    skinList = skinDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    skinDir.cd(getUserPath() + "/skins");
    skinList.append(skinDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name));
    if (currentSkin >= skinList.size())
        currentSkin = 0;
}

void SettingsDialog::showPluginsMsg()
{
#ifdef Q_OS_WIN
    QDesktopServices::openUrl("file:///" + getUserPath() + "/plugins");
#else
    QDesktopServices::openUrl("file://" + getUserPath() + "/plugins");
#endif
}
