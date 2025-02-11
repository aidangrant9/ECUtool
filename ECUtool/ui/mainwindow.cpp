#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "createconnectiondialog.h"
#include <QDebug>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Make DiagnosticSession
    diagnosticSession = std::make_shared<DiagnosticSession>();

    connect(ui->actionNewConnection, &QAction::triggered, this, &MainWindow::onNewConnection);
    connect(ui->actionOpenProject, &QAction::triggered, this, &MainWindow::onOpenProject);
    connect(ui->actionNewProject, &QAction::triggered, this, &MainWindow::onNewProject);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onNewConnection()
{
    SerialConnection *newConnection = nullptr;
    CreateConnectionDialog dialog(&newConnection, std::filesystem::current_path(), nullptr);
    dialog.exec();

    if (newConnection != nullptr)
    {
        std::shared_ptr<SerialConnection> conPtr(newConnection);

        diagnosticSession->setConnection(conPtr);
    }
}

void MainWindow::onOpenProject()
{
    QFileDialog fileBrowser(this);
    fileBrowser.setFileMode(QFileDialog::FileMode::Directory);
    fileBrowser.exec();

    qDebug() << std::filesystem::current_path().native();

}

void MainWindow::onNewProject()
{
}
