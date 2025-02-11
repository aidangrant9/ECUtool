#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "createconnectiondialog.h"
#include "commandview.h"
#include "commanddelegate.h"
#include "messageview.h"
#include "messagedelegate.h"
#include <QDebug>
#include <QFileDialog>
#include "../core/RawCommand.hpp"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Make DiagnosticSession
    diagnosticSession = std::make_shared<DiagnosticSession>();

    CommandView *commandView = new CommandView(diagnosticSession, this);
    CommandDelegate * delegate = new CommandDelegate(this);

    ui->listView->setModel(commandView);
    ui->listView->setItemDelegate(delegate);
    ui->listView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->listView->setViewMode(QListView::ListMode);

    ui->listView->setUniformItemSizes(false);
    ui->listView->setResizeMode(QListView::Adjust);

    MessageView *messageView = new MessageView(diagnosticSession, this);
    MessageDelegate *messageDelegate = new MessageDelegate(this);

    ui->messageView->setModel(messageView);
    ui->messageView->setItemDelegate(messageDelegate);

    Message* m = new Message(Message::MessageType::Data, "ExampleExampleExample", "Script1.lua");

    diagnosticSession->addMessage(*m);

    //qDebug() << ui->listView->itemDelegate();

    std::shared_ptr<Command> c = std::make_shared<RawCommand>("Example", 1, std::vector<uint8_t>{});
    diagnosticSession->addCommand(c);
    std::shared_ptr<Command> c1 = std::make_shared<RawCommand>("e2", 1, std::vector<uint8_t>{});
    diagnosticSession->addCommand(c1);
    std::shared_ptr<Command> c2 = std::make_shared<RawCommand>("e3", 1, std::vector<uint8_t>{});
    diagnosticSession->addCommand(c2);
    std::shared_ptr<Command> c3 = std::make_shared<RawCommand>("Example", 1, std::vector<uint8_t>{});
    diagnosticSession->addCommand(c3);
    std::shared_ptr<Command> c4 = std::make_shared<RawCommand>("Example", 1, std::vector<uint8_t>{});
    diagnosticSession->addCommand(c4);
    std::shared_ptr<Command> c5 = std::make_shared<RawCommand>("Example", 1, std::vector<uint8_t>{});
    diagnosticSession->addCommand(c5);
    std::shared_ptr<Command> c6 = std::make_shared<RawCommand>("e2", 1, std::vector<uint8_t>{});
    diagnosticSession->addCommand(c6);
    std::shared_ptr<Command> c7 = std::make_shared<RawCommand>("e3", 1, std::vector<uint8_t>{});
    diagnosticSession->addCommand(c7);
    std::shared_ptr<Command> c8 = std::make_shared<RawCommand>("Example", 1, std::vector<uint8_t>{});
    diagnosticSession->addCommand(c8);
    std::shared_ptr<Command> c9 = std::make_shared<RawCommand>("Example", 1, std::vector<uint8_t>{});
    diagnosticSession->addCommand(c9);

    //qDebug() << ui->listView->model()->rowCount();

    diagnosticSession->notifyCommandsView();

    ui->listView->reset();
    ui->listView->update();
    ui->listView->repaint();

    ui->listView->setCurrentIndex(ui->listView->model()->index(0, 0));


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
