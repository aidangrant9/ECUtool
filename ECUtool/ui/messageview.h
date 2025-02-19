#pragma once

#include <QListView>
#include <QContextMenuEvent>
#include <QClipboard>
#include <QMenu>
#include <QApplication>
#include "../core/Message.hpp"

class MessageView : public QListView
{
Q_OBJECT
public:
	explicit MessageView(QWidget *parent = nullptr)
		: QListView(parent)
	{}

protected:
	void contextMenuEvent(QContextMenuEvent *e) override
	{
		QModelIndex index = indexAt(e->pos());
		std::shared_ptr<Message> m = index.data(Qt::UserRole).value<std::shared_ptr<Message>>();
		
		if (index.isValid())
		{
			QMenu menu{ this };
			for (std::pair<std::string, std::string> &fmt : m->formats)
			{
				QAction *a = new QAction(std::string{ "Copy " + fmt.first}.c_str(), this);
				connect(a, &QAction::triggered, this, [this, fmt]() {QApplication::clipboard()->setText(fmt.second.c_str());});
				menu.addAction(a);
			}
			menu.exec(e->globalPos());
		}
	}
};