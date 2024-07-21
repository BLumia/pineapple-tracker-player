#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "player.h"
#include "util.h"
#include "playlistmanager.h"

#include "instrumentsmodel.h"

#include <QFileDialog>
#include <QMimeData>
#include <QMouseEvent>
#include <QToolTip>
#include <QListView>
#include <QTime>
#include <QMessageBox>

#include <portaudio.h>
#include <libopenmpt/libopenmpt.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_player(new Player(this))
    , m_playlistManager(new PlaylistManager(this))
    , m_instrumentsModel(new InstrumentsModel(m_player, this))
{
    ui->setupUi(this);
    ui->plainTextEdit->setFont(Util::defaultMonoFont());
    ui->instrumentsListView->setFont(Util::defaultMonoFont());
    ui->instrumentsListView->setModel(m_instrumentsModel);
    ui->playlistView->setModel(m_playlistManager->model());
    setWindowIcon(QIcon(":/icons/dist/pineapple-tracker-player.svg"));

    m_playlistManager->setAutoLoadFilterSuffixes({"*.xm", "*.it", "*.mod", "*.s3m", "*.mptm"});

    connect(ui->instrumentsListView, &QListView::clicked, this, [this](const QModelIndex &index){
        m_instrumentsModel->setData(index,
                                    index.data(Qt::CheckStateRole) == Qt::Unchecked
                                        ? Qt::Checked
                                        : Qt::Unchecked,
                                    Qt::CheckStateRole);
    });

    connect(m_player, &Player::fileLoaded, this, [this](){
        ui->horizontalSlider->setMaximum(m_player->totalOrders() - 1);
        ui->horizontalSlider->setValue(m_player->currentOrder());
        ui->songTitle->setText(m_player->title());
        const QString & artist = m_player->artist();
        ui->label_4->setText(artist.isEmpty() ? m_player->tracker() : (m_player->artist() + " (" + m_player->tracker() + ")") );
        ui->plainTextEdit->setPlainText(m_player->message());

        m_instrumentsModel->setStringList(m_player->instrumentNames());
    });

    connect(m_player, &Player::playbackStatusChanged, this, [this](){
        ui->playButton->setText(m_player->isPlaying() ? "Pause" : "Play");
    });

    connect(m_player, &Player::currentOrderChanged, this, [this](){
        QSignalBlocker sb(ui->horizontalSlider);
        ui->horizontalSlider->setValue(m_player->currentOrder());
    });

    connect(m_player, &Player::currentRowChanged, this, [this](){
        ui->playbackStatus->setText(QString::asprintf("Order: %3d/%3d Row: %3d/%3d Time: %s",
                                 m_player->currentOrder(), m_player->totalOrders(),
                                 m_player->currentRow(), m_player->patternTotalRows(m_player->currentPattern()),
                                 QTime::fromMSecsSinceStartOfDay(0).addSecs(m_player->positionSec()).toString("h:mm:ss").toLatin1().data()
                             ));
    });

    connect(ui->horizontalSlider, &QSlider::valueChanged, this, [this](int value){
        m_player->seek(ui->horizontalSlider->value());
        QToolTip::showText(QCursor::pos(), QString::number(value), nullptr);
    });

    connect(ui->horizontalSlider_2, &QSlider::sliderMoved, this, [this](int value){
        m_player->setGain(ui->horizontalSlider_2->value());
        QToolTip::showText(QCursor::pos(), QString("%1 dB").arg(value / 100.f), nullptr);
    });

    connect(ui->actionOpen, &QAction::triggered, this, [this](){
        const QUrl & url = QFileDialog::getOpenFileUrl(
                    this, "Select module file", {},
                    "Module Files (*.xm *.it *.mod *.s3m *.mptm)");
        if (url.isValid()) {
            m_player->load(url);
            m_player->play();
        }
    });

    connect(ui->playButton, &QPushButton::clicked, this, [this](){
        m_player->isPlaying() ? m_player->pause() : m_player->play();
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::playFiles(const QList<QUrl> &urls)
{
    if (urls.size() <= 0) return;

    m_playlistManager->loadPlaylist(urls);
    m_player->load(urls.first());
    m_player->play();
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
        playFiles(urls);
    }
}

void MainWindow::on_instrumentsBtn_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->instrumentsPage);
}

void MainWindow::on_messageBtn_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->messagePage);
}

void MainWindow::on_playlistBtn_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->playlistPage);
}

void MainWindow::on_playlistView_activated(const QModelIndex &index)
{
    m_playlistManager->setCurrentIndex(index);
    playFiles({m_playlistManager->urlByIndex(index)});
}


void MainWindow::on_actionAbout_triggered()
{
    QMessageBox infoBox(this);
    infoBox.setIcon(QMessageBox::Information);
    infoBox.setWindowTitle(tr("About"));
    infoBox.setStandardButtons(QMessageBox::Ok);
    std::uint32_t libopenmptver = openmpt::get_library_version();
    infoBox.setText(
        "Pineapple Tracker Player " PTPLAY_VERSION_STRING
        "\n\n" %
        tr("Based on the following free software libraries:") %
        "\n\n" %
        QStringLiteral("- [Qt](https://www.qt.io/) %1\n").arg(QT_VERSION_STR) %
        QStringLiteral("- [PortAudio](https://www.portaudio.com/) %1.%2.%3\n").arg(Pa_GetVersionInfo()->versionMajor)
            .arg(Pa_GetVersionInfo()->versionMinor)
            .arg(Pa_GetVersionInfo()->versionSubMinor) %
        QStringLiteral("- [libopenmpt](https://lib.openmpt.org/libopenmpt/) %1.%2.%3\n").arg(libopenmptver >> 24)
            .arg((libopenmptver >> 16) & 0xFFFF)
            .arg(libopenmptver & 0xFFFF) %
        "\n"
        "[Source Code](https://github.com/BLumia/pineapple-tracker-player)\n"
        "\n"
        "Copyright &copy; 2024 [BLumia](https://github.com/BLumia/)"
        );
    infoBox.setTextFormat(Qt::MarkdownText);
    infoBox.exec();
}

