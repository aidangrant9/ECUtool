#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include "../core/DiagnosticSession.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    std::shared_ptr<DiagnosticSession> diagnosticSession{};
    void onNewConnection();
    void onOpenProject();
    void onSaveProject();
    void onConnect();
    void onDisconnect();
    void onManualEnter();
    void onConnectionStatusChange(std::optional<SerialConnection::ConnectionStatus>);
    QLabel *statusLabel;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
