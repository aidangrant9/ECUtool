#pragma once

#include <QAbstractListModel>
#include "../core/DiagnosticSession.hpp"

class MessageModel : public QAbstractListModel
{
	Q_OBJECT
public:
	explicit MessageModel(std::shared_ptr<DiagnosticSession> &diagnosticSession, QObject *parent)
		: diagnosticSession(diagnosticSession)
	{
		diagnosticSession->setMessageResetStart([this]() {beginResetModel();});
		diagnosticSession->setMessageResetEnd([this]() {endResetModel();});
	}


private:
	std::shared_ptr<DiagnosticSession> diagnosticSession{};

	Q_INVOKABLE int rowCount(const QModelIndex &parent) const
	{
		return diagnosticSession->getMessages().size();
	}

	Q_INVOKABLE QVariant data(const QModelIndex &index, int role) const override
	{
		if (!index.isValid() || index.row() >= diagnosticSession->getMessages().size())
		{
			return QVariant();
		}

		const std::vector<std::shared_ptr<Message>> &ms = diagnosticSession->getMessages();
		return QVariant::fromValue(ms.at(index.row()));
	}
};