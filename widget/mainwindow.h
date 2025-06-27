#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QMenu;
class QDragEnterEvent;
class QDropEvent;
class QSortFilterProxyModel;
QT_END_NAMESPACE

class Player;
class InstrumentsModel;
class PlaylistManager;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum RepeatMode {
        DisableRepeat,
        Repeat,
        Replay,
        Playlist,
        FirstRepeatMode = DisableRepeat,
        LastRepeatMode = Playlist,
    };
    Q_ENUM(RepeatMode)

    Q_PROPERTY(RepeatMode repeatMode MEMBER m_repeatMode NOTIFY repeatModeChanged)

    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void playFiles(const QList<QUrl> &urls);
    void playFile(const QUrl &url);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

signals:
    void repeatModeChanged(RepeatMode mode);

private slots:
    void on_instrumentsBtn_clicked();
    void on_messageBtn_clicked();
    void on_trackerBtn_clicked();
    void on_playlistBtn_clicked();
    void on_playlistView_activated(const QModelIndex &index);
    void on_actionAbout_triggered();
    void on_filterEdit_textChanged(const QString &arg1);
    void on_muteButton_clicked();
    void on_playbackModeButton_clicked();

private:
    void generateThemeMenu();

    Ui::MainWindow *ui;
    QMenu *m_themes;
    Player * m_player = nullptr;
    PlaylistManager * m_playlistManager = nullptr;
    InstrumentsModel * m_instrumentsModel = nullptr;
    QSortFilterProxyModel * m_playlistFilterModel = nullptr;
    RepeatMode m_repeatMode = Repeat;
};
