#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include "../core/DiagnosticSession.hpp"
#include "../core/Logger.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    std::shared_ptr<DiagnosticSession> diagnosticSession{};
    void onNewConnection();
    void onOpenProject();
    void onSaveProject();
    void onConnect();
    void onDisconnect();
    void onAddCommand();
    void onClearLogs();
    void onManualEnter();
    void onSystemLogsChecked(Qt::CheckState checked);
    void onVisibilityChanged(std::shared_ptr<Command> command);
    void onMessage(std::shared_ptr<Message> m);
    void addMessage(std::shared_ptr<Message> m);
    void onCommandDoubleClicked(const QModelIndex &index);
    void onConnectionStatusChange(const Connection::ConnectionStatus status, const std::string message);

    QLabel *statusLabel;
    Logger &logger = Logger::instance();
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
