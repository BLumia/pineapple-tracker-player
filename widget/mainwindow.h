#pragma once

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QDragEnterEvent;
class QDropEvent;
QT_END_NAMESPACE

class Player;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void playFiles(const QList<QUrl> &urls);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void on_instrumentsBtn_clicked();

    void on_messageBtn_clicked();

private:
    Ui::MainWindow *ui;
    Player * m_player = nullptr;
};
