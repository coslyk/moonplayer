#include "console.h"
#include <QProcess>
#include <QScrollBar>
#include <QTextCodec>

Console::Console(QWidget *parent) :
    QPlainTextEdit(parent),
    m_process(new QProcess(this))
{
    // Show as a window
    setWindowFlag(Qt::Window, true);
    resize(600, 450);
    
    document()->setMaximumBlockCount(100);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    
    // Display output
    connect(m_process, &QProcess::readyReadStandardOutput, [=]() {
        putData(m_process->readAllStandardOutput());
    });
    
    // Open window when script runs
    connect(m_process, &QProcess::started, [=]() {
        setWindowFlag(Qt::WindowCloseButtonHint, false);
        show();
    });
    
    // Close window when script ends
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=]() {
        setWindowFlag(Qt::WindowCloseButtonHint, true);
        show();
    });
}

void Console::putData(const QByteArray& data)
{
    QTextCodec *codec = QTextCodec::codecForLocale();
    insertPlainText(codec->toUnicode(data));
    QScrollBar *bar = verticalScrollBar();
    bar->setValue(bar->maximum());
}

void Console::launchScript ( const QString& filePath, const QStringList& args )
{
    clear();
    m_process->start(filePath, args, QProcess::ReadOnly);
}

