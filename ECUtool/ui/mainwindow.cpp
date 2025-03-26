#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "createconnectiondialog.h"
#include "commandmodel.h"
#include "commanddelegate.h"
#include "commandnew.h"
#include <QDebug>
#include <QFileDialog>
#include "../core/RawCommand.hpp"
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    statusLabel = new QLabel(this);
    ui->statusbar->insertWidget(0, statusLabel);

    // Setup callbacks
    auto statusChangeCb = std::bind(&MainWindow::onConnectionStatusChange, this, std::placeholders::_1, std::placeholders::_2);
    diagnosticSession = std::make_shared<DiagnosticSession>(statusChangeCb);

    auto messageCb = std::bind(&MainWindow::onMessage, this, std::placeholders::_1);
    logger.setMessageCallback(messageCb);

    CommandModel *commandModel = new CommandModel(diagnosticSession, this);
    CommandDelegate * delegate = new CommandDelegate(this);

    QRegularExpressionValidator *vecStreamValidator = new QRegularExpressionValidator(QRegularExpression("^([0-9A-Fa-f]{2}(\\s*[0-9A-Fa-f]{2})*)$"), this);
    ui->lineEdit->setValidator(vecStreamValidator);
    ui->listView->setModel(commandModel);
    ui->listView->setItemDelegate(delegate);
    ui->listView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->listView->setViewMode(QListView::ListMode);

    connect(ui->actionNewConnection, &QAction::triggered, this, &MainWindow::onNewConnection);
    connect(ui->actionOpenProject, &QAction::triggered, this, &MainWindow::onOpenProject);
    connect(ui->actionSaveProject, &QAction::triggered, this, &MainWindow::onSaveProject);
    connect(ui->actionConnect, &QAction::triggered, this, &MainWindow::onConnect);
    connect(ui->actionDisconnect, &QAction::triggered, this, &MainWindow::onDisconnect);
    connect(ui->addCommandButton, &QPushButton::pressed, this, &MainWindow::onAddCommand);
    connect(ui->listView, &QListView::activated, this, &MainWindow::onCommandDoubleClicked);
    connect(ui->listView, &CommandView::visibilityChanged, this, &MainWindow::onVisibilityChanged);
    connect(ui->clearButton, &QPushButton::pressed, this, &MainWindow::onClearLogs);
    connect(ui->systemLogsCheck, &QCheckBox::checkStateChanged, this, &MainWindow::onSystemLogsChecked);

    statusLabel->setStyleSheet("padding: 2px 5px;");
    statusLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

    onSystemLogsChecked(ui->systemLogsCheck->checkState());
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
    

    if (fileBrowser.exec() == QDialog::Accepted) {
        diagnosticSession->openProject(std::filesystem::path(fileBrowser.selectedFiles().first().toStdString()));
    }
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

void MainWindow::onClearLogs()
{
    logger.clearLogs();
    ui->terminalTextEdit->clear();
}

void MainWindow::onSystemLogsChecked(Qt::CheckState checked)
{
    logger.setSourceVisible("__SYSTEM__RESERVED__VISIBILITY", checked == 2);
    std::vector<std::shared_ptr<Message>> msgs = logger.getMessages(true);

    ui->terminalTextEdit->clear();

    for (std::shared_ptr<Message> &m : msgs)
    {
        addMessage(m);
    }
}

void MainWindow::onVisibilityChanged(std::shared_ptr<Command> command)
{
    logger.setSourceVisible(command->name, command->visible);
    std::vector<std::shared_ptr<Message>> msgs = logger.getMessages(true);

    ui->terminalTextEdit->clear();

    for (std::shared_ptr<Message> &m : msgs)
    {
        addMessage(m);
    }
}

void MainWindow::onMessage(std::shared_ptr<Message> m)
{
    // Handle logger message
    QMetaObject::invokeMethod(this, [=]() {
        addMessage(m);
        }, Qt::QueuedConnection);
}

void MainWindow::addMessage(std::shared_ptr<Message> m)
{
    ui->terminalTextEdit->appendPlainText("");

    QColor defaultColor = ui->terminalTextEdit->palette().color(QPalette::Text);

    QTextCharFormat format = ui->terminalTextEdit->currentCharFormat();
    QTextCursor cursor = ui->terminalTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);

    format.setForeground(Qt::gray);
    cursor.insertText(QString::fromStdString(std::format("[{:s}] [{:s}]\n", m->timeString, m->source)), format);

    QRegularExpression tagRegex("<#(default|[0-9A-Fa-f]{6})>");
    int lastIndex = 0;
    QString msg = QString::fromStdString(m->msg);
    QRegularExpressionMatchIterator it = tagRegex.globalMatch(msg);

    QColor currentCol = defaultColor;

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        int index = match.capturedStart();

        QString fragment = msg.mid(lastIndex, index - lastIndex);
        format.setForeground(currentCol);
        cursor.insertText(fragment, format);

        QString tagContent = match.captured(1);
        if (tagContent.toLower() == "default") {
            currentCol = defaultColor;
        }
        else {
            currentCol = QColor("#" + tagContent);
        }
        lastIndex = match.capturedEnd();
    }

    QString remainder = msg.mid(lastIndex);
    format.setForeground(currentCol);
    cursor.insertText(remainder, format);

    cursor.insertText("\n");
    ui->terminalTextEdit->setTextCursor(cursor);
}

void MainWindow::onCommandDoubleClicked(const QModelIndex &index)
{
    std::shared_ptr<Command> c = index.data().value<std::shared_ptr<Command>>();
    diagnosticSession->queueOrUnqueueCommand(c);
}

void MainWindow::onConnectionStatusChange(const Connection::ConnectionStatus status, const std::string message)
{
    QMetaObject::invokeMethod(this, [=]() {
        if (status == Connection::ConnectionStatus::Connected)
        {
            ui->actionConnect->setEnabled(false);
            ui->actionDisconnect->setEnabled(true);
        }
        else
        {
            ui->actionConnect->setEnabled(true);
            ui->actionDisconnect->setEnabled(false);
        }
        statusLabel->setText(QString::fromStdString(message));
        }, Qt::QueuedConnection);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Exit Application",
        "Do you want to save your project before exiting?",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
    );

    if (reply == QMessageBox::Save) {
        onSaveProject();
        event->accept();
    }
    else if (reply == QMessageBox::Discard) {
        event->accept();
    }
    else {
        event->ignore();
    }
}