#ifndef PLUGIN_H
#define PLUGIN_H

#include <QObject>
#include <QJSValue>
#include <QJSEngine>

class Plugin : public QObject {
    
    Q_OBJECT
    
    Q_PROPERTY(QString name             READ name                       NOTIFY nameChanged)
    Q_PROPERTY(QString description      READ description                NOTIFY descriptionChanged)
    Q_PROPERTY(QString keyword          READ keyword  WRITE setKeyword  NOTIFY keywordChanged)
    Q_PROPERTY(int page                 READ page     WRITE setPage     NOTIFY pageChanged)
    Q_PROPERTY(QStringList resultModel  READ resultModel                NOTIFY resultModelChanged)
    
public:
    Plugin(const QString& filepath, QObject* parent = nullptr);
    static QObjectList loadPlugins(void);
    
    Q_INVOKABLE void openItem(int index);
    
    // Access properties
    inline QString name() { return m_name; }
    inline QString description() { return m_description; }
    inline QString keyword() { return m_keyword; }
    inline int page() { return m_page; }
    inline QStringList resultModel() { return m_titles; }
    
    void setKeyword(const QString& keyword);
    void setPage(int page);
    
signals:
    void nameChanged(void);
    void descriptionChanged(void);
    void keywordChanged(void);
    void pageChanged(void);
    void resultModelChanged(void);
    
private slots:
    void updateResult(const QVariant& result);
    void printJSError(const QJSValue &errValue);
    
private:
    QJSEngine m_engine;
    QByteArray m_script;
    QJSValue m_searchFunc;
    QString m_name;
    QString m_description;
    QString m_keyword;
    QString m_id;
    int m_page;
    QStringList m_titles;
    QList<QUrl> m_urls;
};

#endif // PLUGIN_H
