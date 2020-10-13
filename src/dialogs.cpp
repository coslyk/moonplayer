#include "dialogs.h"

Dialogs* Dialogs::s_instance = nullptr;

Dialogs::Dialogs(QObject* parent) : QObject(parent)
{
    Q_ASSERT(s_instance == nullptr);
    s_instance = this;
}

Dialogs::~Dialogs()
{
    s_instance = nullptr;
}

void Dialogs::selectionDialog(const QString &title, const QStringList& items, std::function<void(int)> callback)
{
    m_selectionCb = callback;
    emit selectionDialogRequested(title, items);
}

void Dialogs::selectionDialogCallback(int index)
{
    m_selectionCb(index);
}