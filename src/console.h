#ifndef CONSOLE_H
#define CONSOLE_H

#include <QPlainTextEdit>
#include <QProcess>


/* Open a console window and launch scripts */
class Console : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit Console(QWidget *parent = nullptr);
    void launchScript(const QString& filePath, const QStringList& args = QStringList());
    
private:
    void putData(const QByteArray& _data);
    QProcess m_process;
};

#endif // CONSOLE_H
