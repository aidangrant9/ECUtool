#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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
    std::shared_ptr<DiagnosticSession> diagnosticSession {};
    void onNewConnection();
    void onOpenProject();
    void onNewProject();
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
