#include "detailview.h"
#include "ui_detailview.h"
#include "utils.h"
#include "accessmanager.h"
#include "plugins.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>

DetailView::DetailView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DetailView)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose);
    connect(ui->playPushButton, SIGNAL(clicked()), this, SLOT(onPlay()));
    connect(ui->downloadPushButton, SIGNAL(clicked()), this, SLOT(onDownload()));
}

DetailView::~DetailView()
{
    delete ui;
}

PyObject* DetailView::loadDetail(PyObject *dict)
{
    static QString nameFmt = "<span style=\" font-size:16pt; font-weight:600;\">%1</span> (rating: %2)";
    if (!PyDict_Check(dict))
    {
        PyErr_SetString(PyExc_TypeError, "The argument is not a dict.");
        return NULL;
    }

    PyObject *item;
    QString name;
    item = PyDict_GetItemString(dict, "name");
    if (item)
    {
        name = PyString_AsQString(item);
        setWindowTitle(name + tr(" - Detail page"));
    }
    item = PyDict_GetItemString(dict, "rating");
    if (item)
        ui->nameLabel->setText(nameFmt.arg(name, QString::number(PyFloat_AsDouble(item))));
    else
        ui->nameLabel->setText(nameFmt.arg(name, tr("Unknown")));

    item = PyDict_GetItemString(dict, "director");
    if (item)
    {
        QStringList list = PyList_AsQStringList(item);
        ui->directorLabel->setText(list.join(" / "));
    }
    item = PyDict_GetItemString(dict, "script_writer");
    if (item)
    {
        QStringList list = PyList_AsQStringList(item);
        ui->scriptwriterLabel->setText(list.join(" / "));
    }
    item = PyDict_GetItemString(dict, "player");
    if (item)
    {
        QStringList list = PyList_AsQStringList(item);
        ui->playerLabel->setText(list.join(" / "));
    }
    item = PyDict_GetItemString(dict, "type");
    if (item)
    {
        QStringList list = PyList_AsQStringList(item);
        ui->typeLabel->setText(list.join(" / "));
    }
    item = PyDict_GetItemString(dict, "nation");
    if (item)
    {
        QStringList list = PyList_AsQStringList(item);
        ui->nationLabel->setText(list.join(" / "));
    }
    item = PyDict_GetItemString(dict, "language");
    if (item)
    {
        QStringList list = PyList_AsQStringList(item);
        ui->langLabel->setText(list.join(" / "));
    }
    item = PyDict_GetItemString(dict, "date");
    if (item)
    {
        QStringList list = PyList_AsQStringList(item);
        ui->dateLabel->setText(list.join(" / "));
    }
    item = PyDict_GetItemString(dict, "length");
    if (item)
        ui->lengthLabel->setText(PyString_AsQString(item));

    item = PyDict_GetItemString(dict, "alternate_name");
    if (item)
    {
        QStringList list = PyList_AsQStringList(item);
        ui->alternameLabel->setText(list.join(" / "));
    }
    item = PyDict_GetItemString(dict, "summary");
    if (item)
        ui->summaryLabel->setText(PyString_AsQString(item));

    // Source
    item = PyDict_GetItemString(dict, "source");
    if (item)
    {
        int n = PyList_Size(item);
        for (int i = 0; i < n; i += 2)
        {
            QString name = PyString_AsQString(PyList_GetItem(item, i));
            const char *url = PyString_AsString(PyList_GetItem(item, i+1));
            ui->sourceListWidget->addItem(name);
            urls.append(url);
        }
    }

    // Image
    item = PyDict_GetItemString(dict, "image");
    if (item)
    {
        QNetworkRequest request(QUrl(PyString_AsQString(item)));
        request.setRawHeader("User-Agent", "moonplayer");
        reply = access_manager->get(request);
        connect(reply, SIGNAL(finished()), this, SLOT(onImageLoaded()));
    }
    Py_IncRef(Py_None);
    return Py_None;
}

void DetailView::onImageLoaded()
{
    QPixmap pic;
    pic.loadFromData(reply->readAll());
    if (pic.height() > 300)
        pic = pic.scaledToHeight(300, Qt::SmoothTransformation);
    reply->deleteLater();
    reply = NULL;
    ui->picLabel->setPixmap(pic);
    ui->picLabel->setFixedSize(pic.size());
}

void DetailView::onPlay()
{
    int current_row = ui->sourceListWidget->currentRow();
    if (current_row < 0)
        return;
    QString host = QUrl(QString::fromUtf8(urls[current_row])).host();
    Plugin *plugin = getPluginByHost(host);
    if (plugin)
    {
        plugin->parse(urls[current_row].constData(), false);
        close();
    }
    else
        QMessageBox::warning(this, "warning", tr("Cannot find plugin which can parse this source."));
}

void DetailView::onDownload()
{
    int current_row = ui->sourceListWidget->currentRow();
    if (current_row < 0)
        return;
    QString host = QUrl(QString::fromUtf8(urls[current_row])).host();
    Plugin *plugin = getPluginByHost(host);
    if (plugin)
    {
        plugin->parse(urls[current_row].constData(), true);
        close();
    }
    else
        QMessageBox::warning(this, "warning", tr("Cannot find plugin which can parse this source."));
}
