#include "commandview.h"
#include <iterator>

CommandView::CommandView(std::shared_ptr<DiagnosticSession> &diagnosticSession, QObject *parent)
    : QAbstractListModel(parent), diagnosticSession(diagnosticSession)
{
    diagnosticSession->setCommandViewCallback([this]() {beginResetModel(); endResetModel();});
}

Q_INVOKABLE int CommandView::rowCount(const QModelIndex &parent) const
{
    return diagnosticSession->getCommands().size();
}

Q_INVOKABLE QVariant CommandView::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= diagnosticSession->getCommands().size())
    {
        return QVariant();
    }

    const std::vector<std::shared_ptr<Command>> &cs = diagnosticSession->getCommands();
    return QVariant::fromValue(cs.at(index.row()));
}
