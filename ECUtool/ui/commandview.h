#pragma once

#include <QAbstractListModel>
#include "../core/DiagnosticSession.hpp"

class CommandView : public QAbstractListModel
{
	Q_OBJECT
public:
	explicit CommandView(std::shared_ptr<DiagnosticSession> &diagnosticSession, QObject *parent);

	
private:
	std::shared_ptr<DiagnosticSession> diagnosticSession{};

	// Inherited via QAbstractListModel
	Q_INVOKABLE int rowCount(const QModelIndex &parent) const override;
	Q_INVOKABLE QVariant data(const QModelIndex &index, int role) const override;
};