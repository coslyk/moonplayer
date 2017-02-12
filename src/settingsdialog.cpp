#include "settingsdialog.h"
#include "settings_network.h"
#include "settings_player.h"
#include "settings_plugins.h"
#include "settings_video.h"
#include "settings_danmaku.h"
#include "ui_settingsdialog.h"
#include "videoqualities.h"
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

QString Settings::aout;
QString Settings::vout;
QString Settings::path;
QString Settings::userPath;
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
    connect(ui->fontPushButton, &QPushButton::clicked, this, &SettingsDialog::onFontButton);
    connect(ui->viewPluginsButton, SIGNAL(clicked()), this, SLOT(showPluginsMsg()));
    connect(ui->qualitiesButton, &QPushButton::clicked, this, &SettingsDialog::manageQualities);

    group = new QButtonGroup(this);
    group->addButton(ui->normalRadioButton, 0);
    group->addButton(ui->highRadioButton, 1);
    group->addButton(ui->superRadioButton, 2);
	group->addButton(ui->_1080pRadioButton, 3);
    ui->skinComboBox->addItems(skinList);

    setMinimumSize(minimumSize() * uiScale);

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

    switch (quality)
    {
    case NORMAL: ui->normalRadioButton->setChecked(true);break;
    case HIGH:   ui->highRadioButton->setChecked(true);  break;
    case SUPER:  ui->superRadioButton->setChecked(true); break;
	default:     ui->_1080pRadioButton->setChecked(true);break;
    }
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
    vout = ui->voComboBox->currentText();
    proxy = ui->proxyEdit->text().simplified();
    port = ui->portEdit->text().toInt();
    cacheSize = ui->cacheSpinBox->value();
    maxTasks = ui->maxTaskSpinBox->value();
    downloadDir = ui->dirButton->text();
    quality = (enum Quality) group->checkedId();
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
    settings.setValue("Video/out", vout);
    settings.setValue("Audio/out", aout);
    settings.setValue("Audio/volume", volume);
    settings.setValue("Net/proxy", proxy);
    settings.setValue("Net/port", port);
    settings.setValue("Net/cache_size", cacheSize);
    settings.setValue("Net/max_tasks", maxTasks);
    settings.setValue("Net/download_dir", downloadDir);
    settings.setValue("Plugins/quality", (int) quality);
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
#if defined(Q_OS_LINUX)
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

#elif defined(Q_OS_MAC)
    QDir dir = QDir::home();
    dir.cd("Library");
    dir.cd("Application Support");
    if (!dir.cd("MoonPlayer"))
    {
        dir.mkdir("MoonPlayer");
        dir.cd("MoonPlayer");
    }
    if (!dir.exists("plugins"))
        dir.mkdir("plugins");
    if (!dir.exists("skins"))
        dir.mkdir("skins");
#endif

    QSettings settings("moonsoft", "moonplayer");

    //read settings
#if defined(Q_OS_WIN)
    if (QSysInfo::WindowsVersion >= QSysInfo::WV_VISTA)
        vout = settings.value("Video/out", "direct3d").toString();
    else
        vout = settings.value("Video/out", "directx").toString();
#elif defined(Q_OS_LINUX)
    vout = settings.value("Video/out", "xv").toString();
#elif defined(Q_OS_MAC)
    vout = settings.value("Video/out", "opengl").toString();
#endif

    //set path
#if defined(Q_OS_LINUX)
    path = "/usr/share/moonplayer";
    userPath = QDir::homePath() + "/.moonplayer";
#elif defined(Q_OS_MAC)
    path = QCoreApplication::applicationDirPath().replace("/MacOS", "/Resources");
    userPath = QDir::homePath() + "/Library/Application Support/MoonPlayer";
#elif defined(Q_OS_WIN)
    path = QCoreApplication::applicationDirPath();
#else
#error ERROR: Unsupported system!
#endif

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
    quality = (Quality) settings.value("Plugins/quality", (int) SUPER).toInt();
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
    QDesktopServices::openUrl("file://" + userPath + "/plugins");
}

void SettingsDialog::manageQualities()
{
    QStringList list;
    QHash<QString, QString>::const_iterator i;
    for (i = qualities.constBegin(); i != qualities.constEnd(); i++)
        list << QString("%1 (%2)").arg(i.key(), i.value());
    bool ok;
    QString selected = QInputDialog::getItem(this, "Moon Player",
                                             tr("There are quality settings you have saved. Choose one you want to delete."),
                                             list,
                                             0,
                                             false,
                                             &ok);
    if (ok && !selected.isEmpty())
    {
        selected = selected.section(' ', 0, 0);
        qualities.remove(selected);
    }
}
