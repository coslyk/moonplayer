#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>

namespace Ui {
class Settings;
}
class QButtonGroup;

class Settings : public QDialog
{
    Q_OBJECT
    
public:
    explicit Settings(QWidget *parent = 0);
    static void initSettings(void);

    static QString vout;
    static QString path;
    static QString proxy;
    static QString downloadDir;
    static QStringList skinList;
    static int currentSkin;
    static int port;
    static int maxTasks;
    static int cacheSize;
    static int cacheMin;
    static bool framedrop;
    static bool doubleBuffer;
    static bool fixLastFrame;
    static bool ffodivxvdpau;
    static bool autoResize;
    static bool enableScreenshot;
    static enum Quality {NORMAL, HIGH, SUPER} quality;

    ~Settings();
    
private:
    Ui::Settings *ui;
    QButtonGroup *group;

private slots:
    void loadSettings(void);
    void saveSettings(void);
    void onDirButton(void);
};

#endif // SETTINGS_H
