#pragma once

#include <QListView>
#include <QContextMenuEvent>
#include <QDesktopServices>
#include <QMenu>
#include "../core/Command.hpp"
#include "../core/ScriptCommand.hpp"
#include "commandmodel.h"
#include "commandnew.h"

class CommandView : public QListView
{
    Q_OBJECT

public:
    explicit CommandView(QWidget *parent = nullptr)
        : QListView(parent)
    {}

signals:
    void visibilityChanged(std::shared_ptr<Command> command);

private:
    void contextMenuEvent(QContextMenuEvent *e) override
    {
        QModelIndex index = indexAt(e->pos());
        std::shared_ptr<Command> m = index.data(Qt::UserRole).value<std::shared_ptr<Command>>();

        if (index.isValid())
        {
            QMenu menu{ this };
            QAction *toggleVisibility = new QAction("Toggle visibility", this);
            QAction *editAction = new QAction("Edit", this);
            QAction *deleteAction = new QAction("Delete", this);
            QAction *editScript = new QAction("Edit script", this);

            auto cmd = dynamic_cast<ScriptCommand *>(m.get());
            if (cmd)
            {
                menu.addAction(editScript);
                connect(editScript, &QAction::triggered, this, [=]() {
                    if (std::filesystem::is_regular_file(cmd->mainScript))
                    {
                        QString filePath = QString::fromStdString(cmd->mainScript.string());
                        QUrl fileUrl = QUrl::fromLocalFile(filePath);
                        QDesktopServices::openUrl(fileUrl);
                    }
                    });
            }

            menu.addAction(toggleVisibility);
            menu.addAction(editAction);
            menu.addAction(deleteAction);


            connect(toggleVisibility, &QAction::triggered, this, [this, m]() {
                m->visible = !m->visible;
                emit visibilityChanged(m);
                });
            connect(deleteAction, &QAction::triggered, this, [this, index]() {
                model()->removeRow(index.row());
                });
            connect(editAction, &QAction::triggered, this, [this, m, index]()
                {
                    Command *editedCommand;
                    CommandNew editWindow = CommandNew(m.get(), &editedCommand, nullptr);
                    editWindow.exec();

                    if (editWindow.result())
                    {
                        std::shared_ptr<Command> newCommand{ editedCommand };
                        model()->setData(index, QVariant::fromValue<std::shared_ptr<Command>>(newCommand));
                    }
                });

            menu.exec(e->globalPos());
        }
    }
};
