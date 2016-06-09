#ifndef CUTTERBAR_H
#define CUTTERBAR_H

#include <QWidget>

namespace Ui {
class CutterBar;
}
class QProcess;
class QTimer;

class CutterBar : public QWidget
{
    Q_OBJECT

public:
    explicit CutterBar(QWidget *parent = 0);
    ~CutterBar();
    void init(QString filename, int length, int currentPos);

signals:
    void newFrame(int pos);
    void finished(void);

private:
    Ui::CutterBar *ui;
    QString filename;
    int pos;
    int startPos;
    int endPos;
    bool slider_pressed;
    QProcess *process;

private slots:
    void onStartSliderChanged(void);
    void onEndSliderChanged(void);
    void onSliderPressed(void);
    void onSliderReleased(void);
    void startTask(void);
    void onFinished(int status);
};

#endif // CUTTERBAR_H
