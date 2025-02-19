#pragma once

#include <QStyledItemDelegate>
#include <QPainter>
#include <QApplication>
#include <QFontMetrics>
#include <QTextDocument>
#include <QCache>
#include <unordered_map>
#include <format>
#include "../core/Message.hpp"
#include <ctime>

#define CONTENT_SPACING 3 
#define MESSAGE_INDENT 10
#define MESSAGE_SPACING 1
#define INTERNAL_SPACING 0
#define VSPACE 5

class MessageDelegate : public QStyledItemDelegate
{
	Q_OBJECT

private:
	mutable QCache<size_t, QTextDocument> msgCache;

public:
	explicit MessageDelegate(QObject *parent)
		: QStyledItemDelegate(parent)
	{
	}

	Q_INVOKABLE void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
	{	
		std::shared_ptr<Message> m = index.data(Qt::UserRole).value<std::shared_ptr<Message>>();
		size_t mHash = std::hash<Message *>{}(m.get());

		painter->save();

		QRect baseRect = QRect(option.rect.x() + MESSAGE_SPACING, option.rect.y() + MESSAGE_SPACING, option.rect.width() - MESSAGE_SPACING * 2, option.rect.height() - MESSAGE_SPACING * 2);
		QStyleOptionViewItem newSt = option;
		newSt.rect = baseRect;
		QApplication::style()->drawPrimitive(QStyle::PE_FrameGroupBox, &newSt, painter);

		QString nonsel = option.palette.color(QPalette::ColorRole::ToolTipText).name();

		if (msgCache.contains(mHash))
		{
			QTextDocument *messageDoc = msgCache[mHash];
			painter->save();
			painter->setClipRect(baseRect);
			painter->translate(baseRect.topLeft());
			messageDoc->drawContents(painter);
			painter->restore();
		}
		else
		{

			QTextDocument *messageDoc = new QTextDocument();
			messageDoc->setTextWidth(baseRect.width());
			std::ostringstream output{};
			// info bar
			output << "<div style = \"text-indent:2px\">";
			output << "<span style=\"color:" << nonsel.toStdString() << "\">[" << m->timeString << "]</span>  ";
			output << "<span>[" << m->source << "]</span>" << "  ";
			if (m->id != -1)
			{
				output << "<span>&lt;" << m->id << "&gt;</span>";
			}
			output << "</div>";
			output << "<div style=\"margin-left:20px;\">";
			output << m->msg;
			output << "</div>";

			messageDoc->setHtml(output.str().c_str());
			//QRect docRect = QRect(baseRect.x() + MESSAGE_INDENT, baseRect.x() + infoRect.height(), baseRect.width() - MESSAGE_INDENT, docHeight);

			painter->save();
			painter->setClipRect(baseRect);
			painter->translate(baseRect.topLeft());
			messageDoc->drawContents(painter);
			painter->restore();

			msgCache.insert(mHash, messageDoc);
		}
		painter->restore();
	}


	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
	{
		std::shared_ptr<Message> m = index.data(Qt::UserRole).value<std::shared_ptr<Message>>();
		size_t mHash = std::hash<Message *>{}(m.get());

		if (msgCache.contains(mHash))
		{
			QTextDocument *messageDoc = msgCache[mHash];
			return QSize(0, messageDoc->size().height() + VSPACE);
		}
		else
		{

			QTextDocument *messageDoc = new QTextDocument();
			QRect baseRect = QRect(option.rect.x() + MESSAGE_SPACING, option.rect.y() + MESSAGE_SPACING, option.rect.width() - MESSAGE_SPACING * 2, option.rect.height() - MESSAGE_SPACING * 2);
			messageDoc->setTextWidth(baseRect.width());
			QString nonsel = option.palette.color(QPalette::ColorRole::ToolTipText).name();
			std::ostringstream output{};
			// info bar
			output << "<div style = \"text-indent:2px\">";
			output << "<span style=\"color:" << nonsel.toStdString() << "\">[" << m->timeString << "]</span>  ";
			output << "<span>[" << m->source << "]</span>" << "  ";
			if (m->id != -1)
			{
				output << "<span>&lt;" << m->id << "&gt;</span>";
			}
			output << "</div>";
			output << "<div style=\"margin-left:20px;\">";
			output << m->msg;
			output << "</div>";
			messageDoc->setHtml(output.str().c_str());

			return QSize(0, messageDoc->size().height() + VSPACE);
		}
	}
};