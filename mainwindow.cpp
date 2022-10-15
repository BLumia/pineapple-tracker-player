#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "player.h"

#include <QMimeData>
#include <QMouseEvent>
#include <QTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_player(new Player(this))
{
    ui->setupUi(this);

    connect(m_player, &Player::fileLoaded, this, [this](){
        ui->horizontalSlider->setMaximum(m_player->totalOrders() - 1);
        ui->horizontalSlider->setValue(m_player->currentOrder());
        ui->label->setText(m_player->title());
        ui->label_4->setText(m_player->artist());
        ui->label_3->setText(m_player->message());
    });

    connect(m_player, &Player::currentOrderChanged, this, [this](){
        ui->horizontalSlider->setValue(m_player->currentOrder());
    });

    connect(m_player, &Player::currentRowChanged, this, [this](){
        ui->label_2->setText(QString::asprintf("Order: %3d/%3d Row: %3d/%3d Time: %s",
                                 m_player->currentOrder(), m_player->totalOrders(),
                                 m_player->currentRow(), m_player->patternTotalRows(m_player->currentPattern()),
                                 QTime::fromMSecsSinceStartOfDay(0).addSecs(m_player->positionSec()).toString("h:mm:ss").toLatin1().data()
                             ));
    });

    connect(ui->horizontalSlider, &QSlider::sliderMoved, this, [this](){
        m_player->seek(ui->horizontalSlider->value());
    });

    connect(ui->pushButton, &QPushButton::clicked, this, [this](){
        qDebug() << "crashed" << 114514191981 / 0;
    });

    connect(ui->pushButton_2, &QPushButton::clicked, this, [this](){
        m_player->play();
    });

    connect(ui->pushButton_3, &QPushButton::clicked, this, [this](){
        m_player->pause();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }

    return QMainWindow::dragEnterEvent(event);
}

void MainWindow::dropEvent(QDropEvent *event)
{
    event->acceptProposedAction();

    const QMimeData * mimeData = event->mimeData();

    if (mimeData->hasUrls()) {
        const QList<QUrl> &urls = mimeData->urls();
        if (!urls.isEmpty()) {
            m_player->load(urls.first());
            m_player->play();
        }
    }
}
