#include "webvideo.h"
#include <QMessageBox>
#include <QDir>
#include <QGridLayout>
#include <QPushButton>
#include <QListWidget>
#include <QSpacerItem>
#include <QComboBox>
#include <QInputDialog>
#include <QShowEvent>
#include <QDesktopWidget>
#include <QApplication>
#include <QLabel>
#include <QUrl>
#include "searcher.h"
#include "plugin.h"
#include "pyapi.h"
#include "utils.h"
#include "yougetbridge.h"

WebVideo *webvideo = NULL;

WebVideo::WebVideo(QWidget *parent) :
    QTabWidget(parent)
{
    printf("Initialize webview...\n");
    webvideo = this;
    setObjectName("WebVideo");

    initPlugins();
    initSearchers();
    if (n_searchers == 0)
    {
        QLabel *label = new QLabel(tr("You have not install any plugins yet ~_~"));
        addTab(label, tr("Web videos"));
        return;
    }

    QPushButton *playButton = new QPushButton(tr("Play"));
    QPushButton *downButton = new QPushButton(tr("Down"));
    QPushButton *searchButton = new QPushButton(tr("Search"));
    lineEdit = new QLineEdit;
    prevButton = new QPushButton(tr("Prev"));
    nextButton = new QPushButton(tr("Next"));
    listWidget = new QListWidget;
    comboBox = new QComboBox;
    QWidget *page = new QWidget;
    QGridLayout *grid = new QGridLayout(page);
    addTab(page, tr("Web videos"));
    grid->addWidget(comboBox, 0, 0, 1, 1);
    grid->addWidget(lineEdit, 0, 1, 1, 3);
    grid->addWidget(searchButton, 0, 4, 1, 1);
    grid->addWidget(listWidget, 1, 0, 1, 5);
    grid->addWidget(playButton, 2, 0, 1, 1);
    grid->addWidget(downButton, 2, 1, 1, 1);
    grid->addWidget(prevButton, 2, 3, 1, 1);
    grid->addWidget(nextButton, 2, 4, 1, 1);
    grid->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding), 2, 2);
    setMinimumSize(950, 650);

    //down search page and parse
    connect(nextButton, SIGNAL(clicked()), this, SLOT(nextSearchPage()));
    connect(prevButton, SIGNAL(clicked()), this, SLOT(prevSearchPage()));
    connect(lineEdit, SIGNAL(returnPressed()), this, SLOT(searchVideo()));
    connect(searchButton, SIGNAL(clicked()), this, SLOT(searchVideo()));

    //Download video file
    connect(playButton, SIGNAL(clicked()), this, SLOT(onPlayButton()));
    connect(downButton, SIGNAL(clicked()), this, SLOT(onDownButton()));
    connect(listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onDoubleClicked(QListWidgetItem*)));

    //plugins support
    provider = 0;
    for (int i = 0; i < n_searchers; i++)
        comboBox->addItem(searchers[i]->getName());
}

void WebVideo::showEvent(QShowEvent *event)
{
    QDesktopWidget *desktop = qApp->desktop();
    move((desktop->width() - width())/2,
         (desktop->height() - height())/2);
    event->accept();
}

void WebVideo::warnHavingTask()
{
    QMessageBox::warning(this, "warning", tr("Another file is parsing. Please wait."));
}


//Search videos
void WebVideo::searchVideo()
{
    if (geturl_obj->hasTask())
    {
        warnHavingTask();
        return;
    }
    QString key = lineEdit->text().simplified();
    if (key.isEmpty())
        return;
    provider = comboBox->currentIndex();
    keyword = key;
    downSearchPage();
}

void WebVideo::downSearchPage()
{
    // no plugins, return
    if (geturl_obj->hasTask() || keyword.isEmpty() || !n_searchers)
        return;
    // set ui
    page_n = 1;
    prevButton->setEnabled(false);
    nextButton->setEnabled(true);
    // call plugin
    searchers[provider]->search(keyword, 1);
}

void WebVideo::nextSearchPage()
{
    if (geturl_obj->hasTask())
    {
        warnHavingTask();
        return;
    }
    if (keyword.isEmpty())
        return;
    // set ui
    page_n++;
    prevButton->setEnabled(true);
    nextButton->setEnabled(true);
    //call plugin
    searchers[provider]->search(keyword, page_n);
}

void WebVideo::prevSearchPage()
{
    if (geturl_obj->hasTask())
    {
        warnHavingTask();
        return;
    }
    if (keyword.isEmpty())
        return;
    //set ui
    page_n--;
    if (page_n == 1)
        prevButton->setEnabled(false);
    else
        prevButton->setEnabled(true);
    nextButton->setEnabled(true);
    //call plugin
    searchers[provider]->search(keyword, page_n);
}

PyObject* WebVideo::showList(PyObject *list)
{
    listWidget->clear();
    result.clear();
    int size = PyList_Size(list);
    if (size < 0)
        return NULL;
    PyObject *item;
    const char *str;
    for (int i = 0; i < size; i+=2)
    {
         if ((item = PyList_GetItem(list, i)) == NULL)
             return NULL;
        listWidget->addItem(PyString_AsQString(item));
        if ((item = PyList_GetItem(list, i+1)) == NULL)
            return NULL;
        if ((str = PyString_AsString(item)) == NULL)
            return NULL;
        result.append(str);
    }
    show();
    activateWindow();
    setCurrentIndex(1);
    Py_IncRef(Py_None);
    return Py_None;
}

void WebVideo::setListItemColor(int n, const QColor &color)
{
    QListWidgetItem *item = listWidget->item(n);
    if (item)
        item->setForeground(QBrush(color));
}

// Parse video url and play or download
void WebVideo::onDoubleClicked(QListWidgetItem *item)
{
    if (geturl_obj->hasTask())
    {
        warnHavingTask();
        return;
    }
    int i = listWidget->row(item);
    QByteArray url = result[i];
    you_get_bridge.parse(QString::fromUtf8(url), false);
}

void WebVideo::onPlayButton()
{
    QListWidgetItem *item = listWidget->currentItem();
    if (item)
        onDoubleClicked(item);
}

void WebVideo::onDownButton()
{
    if (geturl_obj->hasTask())
    {
        warnHavingTask();
        return;
    }
    int i = listWidget->currentRow();
    if (i == -1)
        return;
    QByteArray url = result[i];
    you_get_bridge.parse(QString::fromUtf8(url), true);
}
