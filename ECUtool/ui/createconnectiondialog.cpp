#include "createconnectiondialog.h"
#include "ui_createconnectiondialog.h"
#include "PortNames.hpp"
#include <QRegularExpressionValidator>
#include "../communication/KLineWindows.hpp"
#include <nlohmann/json.hpp>

CreateConnectionDialog::CreateConnectionDialog(SerialConnection **toConstruct, std::filesystem::path workDir, QWidget *parent)
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
    connect(ui->initModeCombo, &QComboBox::currentIndexChanged, this, &CreateConnectionDialog::updateInitModeState);
    connect(ui->applyButton, &QPushButton::pressed, this, &CreateConnectionDialog::onApply);
    connect(ui->saveButton, &QPushButton::pressed, this, &CreateConnectionDialog::onSave);

    updateConnectionTypeState();
    updateInitModeState();
}

CreateConnectionDialog::~CreateConnectionDialog()
{
    delete ui;
}

void CreateConnectionDialog::populateKLine()
{
    std::vector<std::string> portNames = getPortNames();
    for (auto &s : portNames)
    {
        ui->portCombo->addItem(s.c_str());
    }

    ui->parityCombo->addItem("None", QVariant::fromValue(SerialConnection::Parity::None));
    ui->parityCombo->addItem("Even", QVariant::fromValue(SerialConnection::Parity::Even));
    ui->parityCombo->addItem("Odd", QVariant::fromValue(SerialConnection::Parity::Odd));
    ui->parityCombo->addItem("Mark", QVariant::fromValue(SerialConnection::Parity::Mark));
    ui->parityCombo->addItem("Space", QVariant::fromValue(SerialConnection::Parity::Space));


    QRegularExpressionValidator *oneByteValidator = new QRegularExpressionValidator(QRegularExpression("^[A-Fa-f0-9]{1,2}$"), this);
    QRegularExpressionValidator *u64Validator = new QRegularExpressionValidator(QRegularExpression("^(0|[1-9][0-9]{0,19})$"), this);

    // need to make this non-platform specific
    ui->initModeCombo->addItem("None", QVariant::fromValue(KLine::InitMode::None));
    ui->initModeCombo->addItem("Fast Init", QVariant::fromValue(KLine::InitMode::FastInit));
    ui->initModeCombo->addItem("5 Baud", QVariant::fromValue(KLine::InitMode::FiveBaud));

    ui->addressModeCombo->addItem("Functional", QVariant::fromValue(KLine::AddressingMode::Functional));
    ui->addressModeCombo->addItem("Physical", QVariant::fromValue(KLine::AddressingMode::Physical));

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

void CreateConnectionDialog::onSave()
{
    switch (ui->connectionTypeCombo->currentData().value<ConnectionTypes>())
    {
    case ConnectionTypes::KLine:
        saveKLine();
        break;
    default:
        break;
    }
}

void CreateConnectionDialog::connectKLine()
{
    std::string portName = ui->portCombo->currentText().toStdString();
    size_t baudRate = ui->baudRateEdit->text().toInt(nullptr, 10);
    size_t byteSize = ui->byteSizeEdit->text().toInt(nullptr, 10);
    SerialConnection::Parity parity = ui->parityCombo->currentData().value<SerialConnection::Parity>();
    KLine::AddressingMode addressingMode = ui->addressModeCombo->currentData().value<KLine::AddressingMode>();
    KLine::InitMode initMode = ui->initModeCombo->currentData().value<KLine::InitMode>();
    uint8_t sourceAddress = ui->sourceAddressEdit->text().toInt(nullptr, 16);
    uint8_t targetAddress = ui->targetAddressEdit->text().toInt(nullptr, 16);

    if (initMode == KLine::InitMode::None)
    {
        // Implement platform specific initialisation here
        *toConstruct = new KLine(portName, baudRate, byteSize, parity, KLine::StopBits::OneStopBit, true);
    }
    else
    {
        *toConstruct = new KLine(portName, baudRate, byteSize, parity, KLine::StopBits::OneStopBit, true, initMode, addressingMode, sourceAddress, targetAddress);
    }

    this->close();
}

void CreateConnectionDialog::saveKLine()
{
}

void CreateConnectionDialog::updateConnectionTypeState()
{
    switch (ui->connectionTypeCombo->currentData().value<ConnectionTypes>())
    {
    case ConnectionTypes::KLine:
        ui->stackedWidget->setCurrentIndex(0);
        populateKLine();
        break;
    case ConnectionTypes::Example:
        ui->stackedWidget->setCurrentIndex(1);
        break;
    default:
        break;
    }
}

void CreateConnectionDialog::updateInitModeState()
{
    bool needsAddress = ui->initModeCombo->currentData().value<KLine::InitMode>() != KLine::InitMode::None;

    ui->sourceAddressEdit->setEnabled(needsAddress);
    ui->targetAddressEdit->setEnabled(needsAddress);
    ui->addressModeCombo->setEnabled(needsAddress);
}
