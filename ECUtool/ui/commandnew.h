#ifndef COMMANDNEW_H
#define COMMANDNEW_H

#include <QDialog>
#include "../core/Command.hpp"

namespace Ui {
class CommandNew;
}

class CommandNew : public QDialog
{
    Q_OBJECT

public:
    explicit CommandNew(Command *command = nullptr, Command **toReturn = nullptr, QWidget *parent = nullptr);
    ~CommandNew();

private:
    void populateFields();
    void fillExistingCommand();
    void onUpdateCommandType();
    void onSaveCommand();
    QString stringFromDataVec(std::vector<uint8_t> &data);
    std::vector<uint8_t> dataVecFromString(QString input);
    Command *providedCommand;
    Command **toReturn;
    Ui::CommandNew *ui;
};

#endif // COMMANDNEW_H
