#pragma once

#include <QListView>
#include <QContextMenuEvent>
#include "commandmodel.h"
#include <QMenu>
#include "../core/Command.hpp"
#include "commandnew.h"

class CommandView : public QListView
{
	Q_OBJECT

public:
	explicit CommandView(QWidget *parent)
		: QListView(parent)
	{}

private:
	void contextMenuEvent(QContextMenuEvent *e) override
	{
		QModelIndex index = indexAt(e->pos());
		std::shared_ptr<Command> m = index.data(Qt::UserRole).value<std::shared_ptr<Command>>();

		if (index.isValid())
		{
			QMenu menu{ this };
			QAction *editAction = new QAction("Edit", this);
			QAction *deleteAction = new QAction("Delete", this);
			menu.addAction(editAction);
			menu.addAction(deleteAction);

			connect(deleteAction, &QAction::triggered, this, [this, index]() {model()->removeRow(index.row());});
			connect(editAction, &QAction::triggered, this, [this, m, index]()
				{
					Command *editedCommand;
					CommandNew editWindow = CommandNew(m.get(), &editedCommand, nullptr);
					editWindow.exec();
					
					if (editWindow.result())
					{
						// Command edited successfully
						std::shared_ptr<Command> newCommand{ editedCommand };
						model()->setData(index, QVariant::fromValue<std::shared_ptr<Command>>(newCommand));
					}
				});

			menu.exec(e->globalPos());
		}
	}
};