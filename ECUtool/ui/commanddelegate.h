#pragma once

#include <QStyledItemDelegate>
#include <QPainter>
#include <QApplication>
#include "../core/Command.hpp"
#include "../core/ScriptCommand.hpp"
#include "../core/RawCommand.hpp"

#define ICON_WIDTH 20
#define SPACING 5

class CommandDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	explicit CommandDelegate(QObject *parent)
		: QStyledItemDelegate(parent)
	{
	}

	Q_INVOKABLE void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
	{
		painter->save();

		std::shared_ptr<Command> cm = index.data(Qt::UserRole).value<std::shared_ptr<Command>>();

		QApplication::style()->drawControl(QStyle::CE_ItemViewItem, &option, painter);

		QRect outerBounds(option.rect.x() + SPACING, option.rect.y(), option.rect.width() - SPACING * 2, option.rect.height());
		QRect textRect(outerBounds.x(), outerBounds.y(), outerBounds.width() - ICON_WIDTH, option.rect.height());
		painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, QString::fromStdString(cm->name));
		
		QFont font = option.font;
		font.setBold(true);
		painter->setFont(font);
		painter->drawText(textRect, Qt::AlignRight | Qt::AlignVCenter, QString::fromStdString(cm->identifier()));

		QRect logoRect(outerBounds.x() + outerBounds.width() - ICON_WIDTH, outerBounds.y(), ICON_WIDTH, outerBounds.height());
		


		painter->restore();
	}

	
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
	{
		return QSize(2, 25);
	}

};