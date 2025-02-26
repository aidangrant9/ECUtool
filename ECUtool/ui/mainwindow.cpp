#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "createconnectiondialog.h"
#include "commandmodel.h"
#include "commanddelegate.h"
#include "messagemodel.h"
#include "messagedelegate.h"
#include "commandnew.h"
#include <QDebug>
#include <QFileDialog>
#include "../core/RawCommand.hpp"
#include <QRegularExpressionValidator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Make DiagnosticSession
    diagnosticSession = std::make_shared<DiagnosticSession>();

    statusLabel = new QLabel(this);
    ui->statusbar->insertWidget(0, statusLabel);

    // Set connection change callback
    diagnosticSession->setStatusChanged(std::bind(&MainWindow::onConnectionStatusChange, this, std::placeholders::_1));

    CommandModel *commandView = new CommandModel(diagnosticSession, this);
    CommandDelegate * delegate = new CommandDelegate(this);

    QRegularExpressionValidator *vecStreamValidator = new QRegularExpressionValidator(QRegularExpression("^([0-9A-Fa-f]{2}(\\s*[0-9A-Fa-f]{2})*)$"), this);
    ui->lineEdit->setValidator(vecStreamValidator);

    ui->listView->setModel(commandView);
    ui->listView->setItemDelegate(delegate);
    ui->listView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->listView->setViewMode(QListView::ListMode);


    MessageModel *messageView = new MessageModel(diagnosticSession, this);
    MessageDelegate *messageDelegate = new MessageDelegate(this);

    ui->messageView->setModel(messageView);
    ui->messageView->setItemDelegate(messageDelegate);
    ui->messageView->setResizeMode(QListView::Adjust);

    connect(ui->actionNewConnection, &QAction::triggered, this, &MainWindow::onNewConnection);
    connect(ui->actionOpenProject, &QAction::triggered, this, &MainWindow::onOpenProject);
    connect(ui->actionSaveProject, &QAction::triggered, this, &MainWindow::onSaveProject);
    connect(ui->actionConnect, &QAction::triggered, this, &MainWindow::onConnect);
    connect(ui->actionDisconnect, &QAction::triggered, this, &MainWindow::onDisconnect);
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &MainWindow::onManualEnter);
    connect(ui->addCommandButton, &QPushButton::pressed, this, &MainWindow::onAddCommand);
    connect(ui->listView, &QListView::activated, this, &MainWindow::onCommandDoubleClicked);
    connect(messageView, &MessageModel::messagesUpdated, this, [this]() {
        QScrollBar *scrollBar = ui->messageView->verticalScrollBar();
        if (scrollBar->value() == scrollBar->maximum()) {
            ui->messageView->scrollToBottom();
        }});
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onNewConnection()
{
    Connection *newConnection = nullptr;
    CreateConnectionDialog dialog(&newConnection, std::filesystem::current_path(), nullptr);
    dialog.exec();

    if (newConnection != nullptr)
    {
        std::shared_ptr<Connection> conPtr(newConnection);

        diagnosticSession->setConnection(conPtr);
    }
}

void MainWindow::onOpenProject()
{
    QFileDialog fileBrowser(this);
    fileBrowser.setFileMode(QFileDialog::FileMode::Directory);
    fileBrowser.setAcceptMode(QFileDialog::AcceptMode::AcceptOpen);
    fileBrowser.exec();

    diagnosticSession->openProject(std::filesystem::path(fileBrowser.selectedFiles().first().toStdString()));
}

void MainWindow::onSaveProject()
{
    diagnosticSession->saveProject();
}

void MainWindow::onConnect()
{
    diagnosticSession->connect();
}

void MainWindow::onDisconnect()
{
    diagnosticSession->disconnect();
}

void MainWindow::onManualEnter()
{
    std::vector<uint8_t> toSend = diagnosticSession->dataVecFromString(ui->lineEdit->text().toStdString());
    if (!toSend.empty())
        diagnosticSession->queueCommand(std::shared_ptr<Command>(new RawCommand("Command Line", 0, toSend)));
    ui->lineEdit->clear();
}

void MainWindow::onAddCommand()
{
    Command *editedCommand;
    CommandNew editWindow = CommandNew(nullptr, &editedCommand, nullptr);
    editWindow.exec();

    if (editWindow.result())
    {
        std::shared_ptr<Command> newCommand{ editedCommand };
        diagnosticSession->addCommand(newCommand);
    }
}

void MainWindow::onCommandDoubleClicked(const QModelIndex &index)
{
    std::shared_ptr<Command> c = index.data().value<std::shared_ptr<Command>>();
    diagnosticSession->queueCommand(c);
}

void MainWindow::onConnectionStatusChange(std::optional<Connection::ConnectionStatus> status)
{
    QMetaObject::invokeMethod(this, [=]() {
        if (status == std::nullopt)
        {
            ui->actionConnect->setEnabled(false);
            ui->actionDisconnect->setEnabled(false);
        }
        else
        {
            switch (status.value())
            {
            case Connection::ConnectionStatus::Connected:
                ui->actionConnect->setEnabled(false);
                ui->actionDisconnect->setEnabled(true);
                break;
            case Connection::ConnectionStatus::Disconnected:
                ui->actionConnect->setEnabled(true);
                ui->actionDisconnect->setEnabled(false);
                break;
            }
            static std::array<std::string, 3> statusStrings{ "Disconnected", "Connected", "Error" };
            statusLabel->setText(QString::fromStdString(statusStrings[static_cast<int>(status.value())]));
        }
        }, Qt::QueuedConnection);
}
