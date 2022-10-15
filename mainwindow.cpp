#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "player.h"

#include <QMimeData>
#include <QMouseEvent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_player(new Player(this))
{
    ui->setupUi(this);

    connect(m_player, &Player::fileLoaded, this, [this](){
        ui->horizontalSlider->setMaximum(m_player->totalOrders());
        ui->horizontalSlider->setValue(m_player->currentOrder());
        ui->label->setText(m_player->title());
        ui->label_3->setText(m_player->message());
    });

    connect(m_player, &Player::currentOrderChanged, this, [this](){
        ui->horizontalSlider->setValue(m_player->currentOrder());
    });

    connect(m_player, &Player::currentRowChanged, this, [this](){
        ui->label_2->setText(QString("%1/%2 %3/%4").arg(
                                 QString::number(m_player->currentOrder()), QString::number(m_player->totalOrders()),
                                 QString::number(m_player->currentRow()), QString::number(m_player->patternTotalRows(m_player->currentPattern()))));
    });

    connect(ui->horizontalSlider, &QSlider::sliderMoved, this, [this](){
        m_player->seek(ui->horizontalSlider->value());
    });

    connect(ui->pushButton, &QPushButton::clicked, this, [this](){
        qDebug() << "crashed" << 114514191981 / 0;
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
