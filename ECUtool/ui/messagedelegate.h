#pragma once

#include <QStyledItemDelegate>
#include <QPainter>
#include <QApplication>
#include "../core/Message.hpp"

#define ICON_WIDTH 20
#define SPACING 5

class MessageDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	explicit MessageDelegate(QObject *parent)
		: QStyledItemDelegate(parent)
	{
	}

	Q_INVOKABLE void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
	{
		painter->save();

		Message m = index.data(Qt::UserRole).value<Message>();
		QApplication::style()->drawPrimitive(QStyle::PE_FrameGroupBox, &option, painter);


		painter->restore();
	}


	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
	{
		return QSize(2, 100);
	}

};