#include "commandnew.h"
#include "ui_commandnew.h"
#include <QRegularExpressionValidator>
#include <QIntValidator>
#include "../core/ScriptCommand.hpp"
#include "../core/RawCommand.hpp"
#include <QMessageBox>

CommandNew::CommandNew(Command *command, Command **toReturn, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CommandNew)
    , providedCommand(command)
    , toReturn(toReturn)
{
    ui->setupUi(this);

    populateFields();

    connect(ui->comboBox, &QComboBox::currentIndexChanged, this, &CommandNew::onUpdateCommandType);
    connect(ui->pushButton, &QPushButton::clicked, this, &CommandNew::onSaveCommand);

    if (providedCommand != nullptr)
    {
        fillExistingCommand();
    }
}

CommandNew::~CommandNew()
{
    delete ui;
}

void CommandNew::populateFields()
{
    ui->comboBox->addItem("Raw", QVariant::fromValue(Command::Type::Raw));
    ui->comboBox->addItem("Script", QVariant::fromValue(Command::Type::Script));

    QRegularExpressionValidator *vecStreamValidator = new QRegularExpressionValidator(QRegularExpression("^([0-9A-Fa-f]{2}(\\s*[0-9A-Fa-f]{2})*)$"), this);
    QIntValidator *intervalValidator = new QIntValidator(0, 60000, this);

    ui->dataEdit->setValidator(vecStreamValidator);
    ui->intervalEdit->setValidator(intervalValidator);
}

void CommandNew::fillExistingCommand()
{
    ui->nameEdit->setText(providedCommand->name.c_str());
    ui->intervalEdit->setText(std::to_string(providedCommand->repeatInMilliseconds).c_str());

    if (auto cm = dynamic_cast<RawCommand *>(providedCommand))
    {
        ui->comboBox->setCurrentIndex(ui->comboBox->findData(QVariant::fromValue(Command::Type::Raw)));
        ui->dataEdit->setText(stringFromDataVec(cm->msg));
    }
    if (auto cm = dynamic_cast<ScriptCommand *>(providedCommand))
    {
        ui->comboBox->setCurrentIndex(ui->comboBox->findData(QVariant::fromValue(Command::Type::Script)));
    }
}

void CommandNew::onUpdateCommandType()
{
    switch (ui->comboBox->currentData().value<Command::Type>())
    {
    case Command::Type::Raw:
        ui->stackedWidget->setCurrentIndex(0);
        break;
    case Command::Type::Script:
        ui->stackedWidget->setCurrentIndex(1);
        break;
    }
}

void CommandNew::onSaveCommand()
{
    std::string name = ui->nameEdit->text().toStdString();
    size_t repeatInterval = ui->intervalEdit->text().toInt();
    std::vector data = dataVecFromString(ui->dataEdit->text());

    if (name.empty())
    {
        QMessageBox alert;
        alert.setText("Name cannot be empty");
        alert.exec();
        return;
    }

    switch (ui->comboBox->currentData().value<Command::Type>())
    {
    case Command::Type::Raw:
        *toReturn = new RawCommand(name, repeatInterval, data);
        break;
    case Command::Type::Script:
        *toReturn = new ScriptCommand(name, repeatInterval);
        break;
    }

    QDialog::done(QDialog::DialogCode::Accepted);
}

QString CommandNew::stringFromDataVec(std::vector<uint8_t> &data)
{
    std::ostringstream o;
    o << std::hex << std::uppercase << std::setfill('0');

    for (size_t i = 0; i < data.size(); ++i)
    {
        o << std::setw(2) << static_cast<int>(data[i]);

        if (i < data.size() - 1)
            o << " ";
    }
    return QString::fromStdString(o.str());
}

std::vector<uint8_t> CommandNew::dataVecFromString(QString input)
{
    std::string inputStr = input.toStdString();
    inputStr.erase(std::remove(inputStr.begin(), inputStr.end(), ' '), inputStr.end());
    std::vector<uint8_t> ret{};
    if (inputStr.length() % 2 != 0)
    {
        inputStr = inputStr;
    }
    for (int i = 0; i < inputStr.length(); i += 2)
    {
        std::string byte = inputStr.substr(i, 2);
        ret.push_back(std::stoi(byte, nullptr, 16));
    }

    return ret;
}
