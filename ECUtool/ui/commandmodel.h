#pragma once

#include <QAbstractListModel>
#include <QAbstractItemModel>
#include "../core/DiagnosticSession.hpp"

class CommandModel : public QAbstractListModel
{
	Q_OBJECT
public:
	explicit CommandModel(std::shared_ptr<DiagnosticSession> &diagnosticSession, QObject *parent);
	
private:
	std::shared_ptr<DiagnosticSession> diagnosticSession{};

	Q_INVOKABLE int rowCount(const QModelIndex &parent) const override;
	Q_INVOKABLE QVariant data(const QModelIndex &index, int role) const override;
	Q_INVOKABLE bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
	Q_INVOKABLE bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
};