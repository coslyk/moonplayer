#include "settings.h"
#include "ui_settings.h"
#include <QDir>
#include <QSettings>
#include <QButtonGroup>
#include <QFileDialog>

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
bool Settings::framedrop;
bool Settings::doubleBuffer;
bool Settings::fixLastFrame;
bool Settings::ffodivxvdpau;
bool Settings::autoResize;
bool Settings::enableScreenshot;
bool Settings::useSkin;
bool Settings::rememberUnfinished;
enum Settings::Quality Settings::quality;

//Show settings dialog
Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);
    connect(this, SIGNAL(accepted()), this, SLOT(saveSettings()));
    connect(this, SIGNAL(rejected()), this, SLOT(loadSettings()));
    connect(ui->dirButton, SIGNAL(clicked()), this, SLOT(onDirButton()));

    group = new QButtonGroup(this);
    group->addButton(ui->normalRadioButton, 0);
    group->addButton(ui->highRadioButton, 1);
    group->addButton(ui->superRadioButton, 2);
    ui->skinComboBox->addItems(skinList);
    loadSettings();
}

//Load settings
void Settings::loadSettings()
{
    ui->voComboBox->setCurrentIndex(ui->voComboBox->findText(vout));
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
    ui->rememberCheckBox->setChecked(rememberUnfinished);
    switch (quality)
    {
    case NORMAL: ui->normalRadioButton->setChecked(true);break;
    case HIGH:   ui->highRadioButton->setChecked(true);  break;
    default:     ui->superRadioButton->setChecked(true); break;
    }
}

void Settings::onDirButton()
{
    QString dir = QFileDialog::getExistingDirectory(this);
    if (!dir.isEmpty())
        ui->dirButton->setText(dir);
}

//Save settings
void Settings::saveSettings()
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
    quality = (enum Quality) group->checkedId();
    currentSkin = ui->skinComboBox->currentIndex();
    useSkin = ui->skinCheckBox->isChecked();
    autoResize = ui->resizeCheckBox->isChecked();
    enableScreenshot = ui->screenshotCheckBox->isChecked();
    rememberUnfinished = ui->rememberCheckBox->isChecked();

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
    settings.setValue("Net/proxy", proxy);
    settings.setValue("Net/port", port);
    settings.setValue("Net/cache_size", cacheSize);
    settings.setValue("Net/cache_min", cacheMin);
    settings.setValue("Net/max_tasks", maxTasks);
    settings.setValue("Net/quality", (int) quality);
    settings.setValue("Net/download_dir", downloadDir);
}


Settings::~Settings()
{
    delete ui;
}

//Init settings
void Settings::initSettings()
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
    quality = (Quality) settings.value("Net/quality", 0).toInt();

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
