#ifndef TRANSFORMER_H
#define TRANSFORMER_H

#include <QWidget>
#include <QTreeWidgetItem>
class QProcess;
class QTreeWidgetItem;
class SortingDialog;
class QShowEvent;

namespace Ui {
class Transformer;
}

class TransformerItem : public QTreeWidgetItem
{
public:
    TransformerItem(QTreeWidget *view, const QString &file, const QString &outfile, int startPos = -1, int endPos = -1);
    TransformerItem(QTreeWidget *view, const QStringList &files, const QString &outfile);
    QStringList files;
    QString file;
    QString outfile;
    int startPos;
    int endPos;
};

class Transformer : public QWidget
{
    Q_OBJECT
    
public:
    explicit Transformer(QWidget *parent = 0);
    bool hasTask(void);
    void addCuttingTask(const QString &filename, int startPos, int endPos);
    ~Transformer();

#ifdef Q_OS_LINUX
protected:
    void showEvent(QShowEvent *e);
#endif
    
private:
    Ui::Transformer *ui;
    SortingDialog *sortingDialog;
    QTimer *timer;
    QProcess *process;
    QStringList args;
    TransformerItem *current;
    int current_rest;
    int prev_percentage;
#ifdef Q_OS_LINUX
    bool mencoder_installed;
#endif

    void start(TransformerItem *item);

private slots:
    void onAddButton(void);
    void onLinkButton(void);
    void onDelButton(void);
    void onStartButton(void);
    void onFinished(int status);
    void onListDoubleClicked(QTreeWidgetItem *item);
    void readOutput(void);
};

extern Transformer *transformer;

#endif // TRANSFORMER_H
