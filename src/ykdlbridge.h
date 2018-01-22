#ifndef YKDLBRIDGE_H
#define YKDLBRIDGE_H


#include <QObject>
class QProcess;
class SelectionDialog;

class YkdlBridge : public QObject
{
    Q_OBJECT
public:
    explicit YkdlBridge(QObject *parent = 0);
    ~YkdlBridge();
    void parse(const QString &url, bool download);

public slots:
    void updateYkdl(void);

private:
    SelectionDialog *selectionDialog;
    QProcess *process;
    QString url;
    bool download;

private slots:
    void onFinished(void);
    void onError(void);
};

extern YkdlBridge ykdl_bridge;
#endif // YKDLBRIDGE_H
