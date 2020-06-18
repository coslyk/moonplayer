#include "plugin.h"
#include "jsapiObject.h"
#include "platform/paths.h"
#include "playlistModel.h"
#include <QDir>
#include <QFile>
#include <QJSEngine>

// Load all plugins
QList<QObject *> Plugin::loadPlugins()
{
    QList<QObject*> result;
    QDir pluginsDir = QDir(userResourcesPath() + "/plugins");
    QStringList list = pluginsDir.entryList(QDir::Files, QDir::Name);

    while (!list.isEmpty())
    {
        QString filename = list.takeFirst();
        if (filename.endsWith(".js"))
        {
            result << new Plugin(pluginsDir.filePath(filename));
        }
    }
    return result;
}



Plugin::Plugin(const QString& filepath, QObject* parent) :
    QObject(parent),
    m_engine(new QJSEngine(this)),
    m_page(1)
{
    // install api
    JSAPIObject *apiObject = new JSAPIObject(this);
    connect(apiObject, &JSAPIObject::showResultRequested, this, &Plugin::updateResult);
    connect(apiObject, &JSAPIObject::jsError, this, &Plugin::printJSError);
    QJSValue api = m_engine->newQObject(apiObject);
    m_engine->globalObject().setProperty("moonplayer", api);
    m_engine->installExtensions(QJSEngine::ConsoleExtension);
    
    // open file
    QFile file(filepath);
    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        throw std::runtime_error("Cannot init plugin!");
    }
    
    // evaluate
    m_script = file.readAll();
    file.close();
    QString content = QString::fromUtf8(m_script);
    QJSValue result = m_engine->evaluate(content, filepath);
    if (result.isError())
    {
        printJSError(result);
        throw std::runtime_error("Cannot init plugin!");
    }
    
    // get name
    m_name = m_engine->globalObject().property("website_name").toString();

    // get description
    QJSValue descValue = m_engine->globalObject().property("website_description");
    if (!descValue.isUndefined())
        m_description = descValue.toString();
    
    // get search() function
    m_searchFunc = m_engine->globalObject().property("search");
}

// Set keyword
void Plugin::setKeyword(const QString& keyword)
{
    if (m_keyword == keyword)
        return;
    m_keyword = keyword;
    emit keywordChanged();
    
    if (!m_keyword.isEmpty())
    {
        m_page = 1;
        emit pageChanged();
        
        // Call search()
        QJSValueList args;
        args << m_keyword << m_page;
        QJSValue retVal = m_searchFunc.call(args);
        if (retVal.isError())
        {
            printJSError(retVal);
        }
    }
}

// Set page
void Plugin::setPage(int page)
{
    if (m_page == page)
        return;
    m_page = page;
    emit pageChanged();
    if (!m_keyword.isEmpty())
    {
        // Call search()
        QJSValueList args;
        args << m_keyword << m_page;
        QJSValue retVal = m_searchFunc.call(args);
        if (retVal.isError())
        {
            printJSError(retVal);
        }
    }
}


// Show result
void Plugin::updateResult(const QVariant& result)
{
    m_titles.clear();
    m_urls.clear();
    QVariantList list = result.toList();
    foreach (QVariant item, list)
    {
        m_titles << item.toHash()["title"].toString();
        m_urls << item.toHash()["url"].toUrl();
    }
    emit resultModelChanged();
}

// Open item
void Plugin::openItem(int index)
{
    PlaylistModel::instance()->addUrl(m_urls[index]);
}


void Plugin::printJSError(const QJSValue& errValue)
{
    QString filename = errValue.property("fileName").toString();
    int lineNumber = errValue.property("lineNumber").toInt();
    QByteArray line = m_script.split('\n')[lineNumber - 1];
    qDebug("In file \"%s\", line %i:\n  %s\n%s",
           filename.toUtf8().constData(),
           lineNumber,
           line.constData(),
           errValue.toString().toUtf8().constData());
}
