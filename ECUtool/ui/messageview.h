#pragma once

#include <QAbstractListModel>
#include "../core/DiagnosticSession.hpp"

class MessageView : public QAbstractListModel
{
	Q_OBJECT
public:
	explicit MessageView(std::shared_ptr<DiagnosticSession> &diagnosticSession, QObject *parent)
		: diagnosticSession(diagnosticSession)
	{
		diagnosticSession->setMessageViewCallback([this]() {beginResetModel(); endResetModel();});
	}


private:
	std::shared_ptr<DiagnosticSession> diagnosticSession{};

	Q_INVOKABLE int rowCount(const QModelIndex &parent) const
	{
		return diagnosticSession->getMessages().size();
	}

	Q_INVOKABLE QVariant data(const QModelIndex &index, int role) const override
	{
		if (!index.isValid() || index.row() >= diagnosticSession->getCommands().size())
		{
			return QVariant();
		}

		const std::vector<Message> &ms = diagnosticSession->getMessages();
		return QVariant::fromValue(ms.at(index.row()));
	}
};