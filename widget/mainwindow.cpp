#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "player.h"
#include "util.h"
#include "playlistmanager.h"
#include "settings.h"

#include "instrumentsmodel.h"

#include <QFileDialog>
#include <QMimeData>
#include <QMouseEvent>
#include <QToolTip>
#include <QListView>
#include <QTime>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QStringBuilder>
#include <QStyleFactory>

#include <portaudio.h>
#include <libopenmpt/libopenmpt.hpp>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_player(new Player(this))
    , m_playlistManager(new PlaylistManager(this))
    , m_instrumentsModel(new InstrumentsModel(m_player, this))
    , m_playlistFilterModel(new QSortFilterProxyModel(this))
{
    ui->setupUi(this);
    ui->plainTextEdit->setFont(Util::defaultMonoFont());
    ui->playbackStatus->setFont(Util::defaultMonoFont());
    ui->instrumentsListView->setFont(Util::defaultMonoFont());
    ui->instrumentsListView->setModel(m_instrumentsModel);
    m_playlistFilterModel->setSourceModel(m_playlistManager->model());
    ui->playlistView->setModel(m_playlistFilterModel);
    setWindowIcon(QIcon(":/icons/dist/pineapple-tracker-player.svg"));
    generateThemeMenu();

    m_playlistManager->setAutoLoadFilterSuffixes({"*.xm", "*.it", "*.mod", "*.s3m", "*.mptm"});

    connect(ui->instrumentsListView, &QListView::clicked, this, [this](const QModelIndex &index){
        m_instrumentsModel->setData(index,
                                    index.data(Qt::CheckStateRole) == Qt::Unchecked
                                        ? Qt::Checked
                                        : Qt::Unchecked,
                                    Qt::CheckStateRole);
    });

    connect(m_player, &Player::fileLoaded, this, [this](){
        ui->playbackSlider->setMaximum(m_player->totalOrders() - 1);
        ui->playbackSlider->setValue(m_player->currentOrder());
        ui->songTitle->setText(m_player->title());
        const QString & artist = m_player->artist();
        ui->label_4->setText(artist.isEmpty() ? m_player->tracker() : (m_player->artist() + " (" + m_player->tracker() + ")") );
        ui->plainTextEdit->setPlainText(m_player->message());

        m_instrumentsModel->setStringList(m_player->instrumentNames());
    });

    connect(m_player, &Player::playbackStatusChanged, this, [this](){
        ui->playButton->setText(m_player->isPlaying() ? tr("Pause") : tr("Play"));
    });

    connect(m_player, &Player::currentOrderChanged, this, [this](){
        QSignalBlocker sb(ui->playbackSlider);
        ui->playbackSlider->setValue(m_player->currentOrder());
    });

    connect(m_player, &Player::currentRowChanged, this, [this](){
        ui->playbackStatus->setText(QString::asprintf("Order: %3d/%3d Row: %3d/%3d Time: %s",
                                 m_player->currentOrder(), m_player->totalOrders(),
                                 m_player->currentRow(), m_player->patternTotalRows(m_player->currentPattern()),
                                 QTime::fromMSecsSinceStartOfDay(0).addSecs(m_player->positionSec()).toString("h:mm:ss").toLatin1().data()
                             ));
    });

    connect(m_player, &Player::gainChanged, this, [this](){
        // only update the icon of the mute button here.
        int32_t gain = m_player->gain();
        QString iconText;
        if (gain < ui->volumeSlider->minimum()) {
            // muted
            iconText = "audio-volume-muted";
        } else if (gain <= -1000) {
            iconText = "audio-volume-low";
        } else if (gain <= 0) {
            iconText = "audio-volume-medium";
        } else {
            iconText = "audio-volume-high";
        }
        ui->muteButton->setIcon(QIcon::fromTheme(iconText));
    });

    connect(this, &MainWindow::repeatModeChanged, this, [this](RepeatMode mode){
        switch (mode) {
        case DisableRepeat:
            m_player->setRepeatCount(1);
            m_player->setProperty("restartAfterFinished", false);
            ui->playbackModeButton->setText(tr("Single", "Repeat is disabled, song will only play "
                                                         "a single time and will then stopped."));
            break;
        case Repeat:
            m_player->setRepeatCount(0);
            ui->playbackModeButton->setText(tr("Repeat"));
            break;
        case Replay:
            m_player->setRepeatCount(1);
            m_player->setProperty("restartAfterFinished", true);
            ui->playbackModeButton->setText(tr("Replay", "Similar to Repeat mode, but the playback "
                                                         "is restarted from beginning instead of "
                                                         "using the loop point in the module file."));
            break;
        }
    });

    connect(ui->playbackSlider, &QSlider::valueChanged, this, [this](int value){
        m_player->seek(ui->playbackSlider->value());
        QToolTip::showText(QCursor::pos(), QString::number(value), nullptr);
    });

    connect(ui->volumeSlider, &QSlider::sliderMoved, this, [this](int value){
        m_player->setGain(ui->volumeSlider->value());
        QToolTip::showText(QCursor::pos(), QString("%1 dB").arg(value / 100.f), nullptr);
    });

    connect(ui->actionOpen, &QAction::triggered, this, [this](){
        const QUrl & url = QFileDialog::getOpenFileUrl(
                    this, "Select module file", {},
                    "Module Files (*.xm *.it *.mod *.s3m *.mptm)");
        if (url.isValid()) {
            playFiles({url});
        }
    });

    connect(ui->playButton, &QPushButton::clicked, this, [this](){
        m_player->isPlaying() ? m_player->pause() : m_player->play();
    });

    ui->trackerWidget->bindablePatternContent().setBinding([&](){
        // qDebug() << m_player->currentPattern();
        return m_player->patternContent(QBindable<int>(m_player, "currentPattern").value());
    });
    ui->trackerWidget->bindableCurrentRow().setBinding([&](){
        return QBindable<int>(m_player, "currentRow").value();
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

void MainWindow::generateThemeMenu()
{
    m_themes = new QMenu();
    ui->actionSetTheme->setMenu(m_themes);

    const QStringList & themes = QStyleFactory::keys();
    for (const QString & theme : themes) {
        QAction * entry = m_themes->addAction(theme);
        entry->setData(theme);
        connect(entry, &QAction::triggered, this, [this](){
            QAction * action = qobject_cast<QAction*>(QObject::sender());
            qApp->setStyle(action->data().toString());
            Settings::instance()->setApplicationStyle(action->data().toString());
        });
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

void MainWindow::on_trackerBtn_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->trackerPage);
}

void MainWindow::on_playlistBtn_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->playlistPage);
}

void MainWindow::on_playlistView_activated(const QModelIndex &index)
{
    QModelIndex sourceIndex(m_playlistFilterModel->mapToSource(index));
    m_playlistManager->setCurrentIndex(sourceIndex);
    playFiles({m_playlistManager->urlByIndex(sourceIndex)});
}


void MainWindow::on_actionAbout_triggered()
{
    QMessageBox infoBox(this);
    infoBox.setIcon(QMessageBox::Information);
    infoBox.setWindowTitle(tr("About"));
    infoBox.setStandardButtons(QMessageBox::Ok);
#if defined(Q_OS_MACOS) && QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
    infoBox.setOption(QMessageBox::Option::DontUseNativeDialog);
#endif // Q_OS_MACOS
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
        "Copyright &copy; 2025 [BLumia](https://github.com/BLumia/)"
        );
    infoBox.setTextFormat(Qt::MarkdownText);
    infoBox.exec();
}


void MainWindow::on_filterEdit_textChanged(const QString &arg1)
{
    m_playlistFilterModel->setFilterFixedString(arg1);
}


void MainWindow::on_muteButton_clicked()
{
    if (QGuiApplication::queryKeyboardModifiers().testFlag(Qt::ShiftModifier)) {
        m_player->setGain(0);
        ui->volumeSlider->setValue(0);
        return;
    }

    bool isMuted = m_player->gain() < ui->volumeSlider->minimum();
    m_player->setGain(isMuted ? ui->volumeSlider->value() : std::numeric_limits<std::int32_t>::min());
}


void MainWindow::on_playbackModeButton_clicked()
{
    RepeatMode targetMode = FirstRepeatMode;
    if (m_repeatMode != LastRepeatMode) {
        targetMode = static_cast<RepeatMode>(m_repeatMode + 1);
    }
    setProperty("repeatMode", targetMode);
}
