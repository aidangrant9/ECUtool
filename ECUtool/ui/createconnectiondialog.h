#ifndef CREATECONNECTIONDIALOG_H
#define CREATECONNECTIONDIALOG_H

#include <QDialog>
#include "../communication/Connection.hpp"
#include <filesystem>

namespace Ui {
class CreateConnectionDialog;
}

class CreateConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CreateConnectionDialog(Connection **toConstruct = nullptr, std::filesystem::path workDir = std::filesystem::current_path(), QWidget *parent = nullptr);
    ~CreateConnectionDialog();

    enum class ConnectionTypes
    {
        KLine,
        Example
    };

private:
    void populateKLine();
    void updateConnectionTypeState();
    void updateInitModeState();
    void onApply();
    void onSave();
    void connectKLine();
    void saveKLine();

    std::filesystem::path workDir{};
    Connection **toConstruct = nullptr;
    Ui::CreateConnectionDialog *ui;
};

#endif // CREATECONNECTIONDIALOG_H
