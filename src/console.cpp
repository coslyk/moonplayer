#include "console.h"
#include <QScrollBar>
#include <QTextCodec>

Console::Console(QWidget *parent) :
    QPlainTextEdit(parent)
{
    // Show as a window
    setWindowFlag(Qt::Window, true);
    resize(600, 450);
    
    Q_ASSERT(document() != nullptr);
    document()->setMaximumBlockCount(100);
    m_process.setProcessChannelMode(QProcess::MergedChannels);
    
    // Display output
    connect(&m_process, &QProcess::readyReadStandardOutput, [=]() {
        putData(m_process.readAllStandardOutput());
    });
    
    // Open window when script runs
    connect(&m_process, &QProcess::started, [=]() {
        setWindowFlag(Qt::WindowCloseButtonHint, false);
        show();
    });
    
    // Close window when script ends
    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [=]() {
        setWindowFlag(Qt::WindowCloseButtonHint, true);
        show();
    });
}

void Console::putData(const QByteArray& _data)
{
    QTextCodec *codec = QTextCodec::codecForLocale();
    Q_ASSERT(codec != nullptr);
    insertPlainText(codec->toUnicode(_data));

    QScrollBar *bar = verticalScrollBar();
    Q_ASSERT(bar != nullptr);
    bar->setValue(bar->maximum());
}

void Console::launchScript ( const QString& filePath, const QStringList& args )
{
    clear();
    m_process.start(filePath, args, QProcess::ReadOnly);
}

