#include "createconnectiondialog.h"
#include "ui_createconnectiondialog.h"
#include "PortNames.hpp"
#include <QRegularExpressionValidator>
#include <nlohmann/json.hpp>
#include "../communication/KWP2000DL.hpp"
#include "../../serial/include/serial/serial.h"

CreateConnectionDialog::CreateConnectionDialog(Connection **toConstruct, std::filesystem::path workDir, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CreateConnectionDialog)
    , toConstruct(toConstruct)
    , workDir(workDir)
{
    ui->setupUi(this);

    // Set up types combo
    {
        ui->connectionTypeCombo->addItem("K-Line", QVariant::fromValue(ConnectionTypes::KLine));
        // How to add additional connection types
        //ui->connectionTypeCombo->addItem("newtype", QVariant::fromValue(ConnectionTypes::Example));
    }

    connect(ui->connectionTypeCombo, &QComboBox::currentIndexChanged, this, &CreateConnectionDialog::updateConnectionTypeState);
    connect(ui->applyButton, &QPushButton::pressed, this, &CreateConnectionDialog::onApply);

    updateConnectionTypeState();
    populateKLine();
}

CreateConnectionDialog::~CreateConnectionDialog()
{
    delete ui;
}

void CreateConnectionDialog::populateKLine()
{
    std::vector<serial::PortInfo> devices_found = serial::list_ports();
    for (auto &s : devices_found)
    {
        ui->portCombo->addItem(s.port.c_str());
    }

    ui->parityCombo->addItem("None", QVariant::fromValue(serial::parity_none));
    ui->parityCombo->addItem("Even", QVariant::fromValue(serial::parity_even));
    ui->parityCombo->addItem("Odd", QVariant::fromValue(serial::parity_odd));
    ui->parityCombo->addItem("Mark", QVariant::fromValue(serial::parity_mark));
    ui->parityCombo->addItem("Space", QVariant::fromValue(serial::parity_space));

    ui->stopBitsCombo->addItem("One", QVariant::fromValue(serial::stopbits_one));
    ui->stopBitsCombo->addItem("One-Five", QVariant::fromValue(serial::stopbits_one_point_five));
    ui->stopBitsCombo->addItem("Two", QVariant::fromValue(serial::stopbits_two));

    QRegularExpressionValidator *oneByteValidator = new QRegularExpressionValidator(QRegularExpression("^[A-Fa-f0-9]{1,2}$"), this);
    QRegularExpressionValidator *u64Validator = new QRegularExpressionValidator(QRegularExpression("^(0|[1-9][0-9]{0,19})$"), this);

    ui->addressModeCombo->addItem("Physical", QVariant::fromValue(KLine::AddressingMode::Physical));
    ui->addressModeCombo->addItem("Functional", QVariant::fromValue(KLine::AddressingMode::Functional));

    ui->baudRateEdit->setValidator(u64Validator);
    ui->byteSizeEdit->setValidator(u64Validator);
    ui->targetAddressEdit->setValidator(oneByteValidator);
    ui->sourceAddressEdit->setValidator(oneByteValidator);
}

void CreateConnectionDialog::onApply()
{
    switch (ui->connectionTypeCombo->currentData().value<ConnectionTypes>())
    {
    case ConnectionTypes::KLine:
        connectKLine();
        break;
    default:
        break;
    }
}

void CreateConnectionDialog::connectKLine()
{
    std::string portName = ui->portCombo->currentText().toStdString();
    uint32_t baudRate = ui->baudRateEdit->text().toInt(nullptr, 10);
    bytesize_t byteSize = static_cast<bytesize_t>(ui->byteSizeEdit->text().toInt(nullptr, 10));
    serial::parity_t parity = ui->parityCombo->currentData().value<serial::parity_t>();
    KLine::AddressingMode addressingMode = ui->addressModeCombo->currentData().value<KLine::AddressingMode>();
    uint8_t sourceAddress = ui->sourceAddressEdit->text().toInt(nullptr, 16);
    uint8_t targetAddress = ui->targetAddressEdit->text().toInt(nullptr, 16);
    stopbits_t stopBits = ui->stopBitsCombo->currentData().value<serial::stopbits_t>();

    *toConstruct = new KLine(portName, baudRate, byteSize, parity, stopBits, serial::flowcontrol_none, ui->oneWireCheck->isChecked(), addressingMode, sourceAddress, targetAddress);

    this->close();
}

void CreateConnectionDialog::updateConnectionTypeState()
{
    switch (ui->connectionTypeCombo->currentData().value<ConnectionTypes>())
    {
    case ConnectionTypes::KLine:
        ui->stackedWidget->setCurrentIndex(0);
        break;
    case ConnectionTypes::Example:
        ui->stackedWidget->setCurrentIndex(1);
        break;
    default:
        break;
    }
}
