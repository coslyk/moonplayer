#include "detailview.h"
#include "ui_detailview.h"
#include "accessmanager.h"
#include "parserbase.h"
#include "python_wrapper.h"
#include <QNetworkRequest>
#include <QNetworkReply>

DetailView::DetailView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DetailView)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    connect(ui->playPushButton, SIGNAL(clicked()), this, SLOT(onPlay()));
    connect(ui->sourceListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onPlay()));
    connect(ui->downloadPushButton, SIGNAL(clicked()), this, SLOT(onDownload()));
}

DetailView::~DetailView()
{
    delete ui;
}


void DetailView::loadDetail(const QVariantHash &data)
{
    static QString nameFmt = "<span style=\" font-size:16pt; font-weight:600;\">%1</span> (rating: %2)";

    // name
    QString name = data["name"].toString();
    setWindowTitle(name + tr(" - Detail page"));

    //rating
    ui->nameLabel->setText(nameFmt.arg(name, data["rating"].toString()));

    //length
    ui->lengthLabel->setText(data["length"].toString());

    //summary
    ui->summaryLabel->setText(data["summary"].toString());

    //others
    struct Item {const char *item_name; QLabel *label;};
    struct Item items[] = {
        {"directors", ui->directorLabel},
        {"script_writers", ui->scriptwriterLabel},
        {"players", ui->playerLabel},
        {"types", ui->typeLabel},
        {"nations", ui->nationLabel},
        {"languages", ui->langLabel},
        {"dates", ui->dateLabel},
        {"alt_names", ui->alternameLabel},
        {nullptr, nullptr}
    };
    for (struct Item *i = items; i->item_name; i++) {
        QStringList list = data[i->item_name].toStringList();
        i->label->setText(list.join(" / ").simplified());
    }


    // Source
    ui->sourceListWidget->clear();
    urls.clear();
    QStringList list = data["source"].toStringList();
    int len = list.length();
    for (int i = 0; i < len; i += 2)
    {
        ui->sourceListWidget->addItem(list[i]); // name
        urls.append(list[i+1]);                 // url
    }

    // Image
    QString img = data["image"].toString();
    if (!img.isEmpty())
    {
        reply = access_manager->get(QNetworkRequest(img));
        connect(reply, SIGNAL(finished()), this, SLOT(onImageLoaded()));
    }
}


void DetailView::onImageLoaded()
{
    QPixmap pic;
    pic.loadFromData(reply->readAll());
    if (pic.height() > 300)
        pic = pic.scaledToHeight(300, Qt::SmoothTransformation);
    reply->deleteLater();
    reply = nullptr;
    ui->picLabel->setPixmap(pic);
    ui->picLabel->setFixedSize(pic.size());
}


void DetailView::onPlay()
{
    int current_row = ui->sourceListWidget->currentRow();
    if (current_row < 0)
        return;
    QString url = urls[current_row];
    if (url.startsWith("python:"))
        PyRun_SimpleString(url.toUtf8().mid(7).constData());
    else
        parseUrl(url, false);
}


void DetailView::onDownload()
{
    int current_row = ui->sourceListWidget->currentRow();
    if (current_row < 0)
        return;
    QString url = urls[current_row];
    if (url.startsWith("python:"))
        PyRun_SimpleString(url.toUtf8().mid(7).constData());
    else
        parseUrl(url, true);
}
