#ifndef DIALOGS_H
#define DIALOGS_H

#include <QObject>
#include <functional>

// Show dialogs in QML
class Dialogs : public QObject
{
    Q_OBJECT

signals:
    void selectionDialogRequested(const QString& title, const QStringList& items);

public:
    Dialogs(QObject* parent = nullptr);
    virtual ~Dialogs();
    static inline Dialogs* instance() { return s_instance; }

    // Selection dialog
    void selectionDialog(const QString& title, const QStringList& items, std::function<void(int)> callback);
    Q_INVOKABLE void selectionDialogCallback(int index);

private:
    std::function<void(int)> m_selectionCb;

    static Dialogs* s_instance;
};

#endif