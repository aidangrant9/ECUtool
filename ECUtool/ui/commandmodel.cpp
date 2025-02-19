#include "commandmodel.h"
#include "commandnew.h"
#include <iterator>

CommandModel::CommandModel(std::shared_ptr<DiagnosticSession> &diagnosticSession, QObject *parent)
    : QAbstractListModel(parent), diagnosticSession(diagnosticSession)
{
    diagnosticSession->setCommandsResetStart([this]() {beginResetModel();});
    diagnosticSession->setCommandsResetEnd([this]() {endResetModel();});
}

Q_INVOKABLE int CommandModel::rowCount(const QModelIndex &parent) const
{
    return diagnosticSession->getCommands().size();
}

Q_INVOKABLE QVariant CommandModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= diagnosticSession->getCommands().size())
    {
        return QVariant();
    }

    const std::vector<std::shared_ptr<Command>> &cs = diagnosticSession->getCommands();
    return QVariant::fromValue(cs.at(index.row()));
}

Q_INVOKABLE bool CommandModel::removeRows(int row, int count, const QModelIndex &parent)
{
    bool r = true;
    for (int i = row; i < row + count; i++)
    {
        r = r && diagnosticSession->removeCommand(i);
    }
    return r;
}

Q_INVOKABLE bool CommandModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;
    
    std::shared_ptr<Command> m = value.value<std::shared_ptr<Command>>();
    return diagnosticSession->editCommand(index.row(), m);
}
