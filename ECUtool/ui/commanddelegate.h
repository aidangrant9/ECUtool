#pragma once

#include <QStyledItemDelegate>
#include <QPainter>
#include <QApplication>
#include <QSvgRenderer>
#include "../core/Command.hpp"
#include "../core/ScriptCommand.hpp"
#include "../core/RawCommand.hpp"

#define ICON_SIZE 15
#define SPACING 5

class CommandDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit CommandDelegate(QObject *parent = nullptr)
        : QStyledItemDelegate(parent)
    {
    }

    QPixmap loadSvg(const QString &path, const QColor &color) const
    {
        QSvgRenderer renderer(path);
        QRectF bounds = renderer.viewBoxF();

        QImage image(ICON_SIZE, ICON_SIZE, QImage::Format_ARGB32);
        image.fill(Qt::transparent);

        QPainter painter(&image);
        QSizeF targetSize(ICON_SIZE - 1, ICON_SIZE - 1);
        QSizeF scaledSize = bounds.size();
        scaledSize.scale(targetSize, Qt::KeepAspectRatio);
        QPointF center((ICON_SIZE - scaledSize.width()) / 2, (ICON_SIZE - scaledSize.height()) / 2);
        QRectF finalRect(center, scaledSize);
        renderer.render(&painter, finalRect);
        painter.end();

        QImage coloredImage = image.convertToFormat(QImage::Format_ARGB32);
        QPainter colorPainter(&coloredImage);
        colorPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        colorPainter.fillRect(coloredImage.rect(), color);
        colorPainter.end();

        return QPixmap::fromImage(coloredImage);
    }

    Q_INVOKABLE void paint(QPainter *painter,
        const QStyleOptionViewItem &option,
        const QModelIndex &index) const override
    {
        painter->save();

        std::shared_ptr<Command> cm = index.data(Qt::UserRole).value<std::shared_ptr<Command>>();
        if (!cm)
        {
            painter->restore();
            return;
        }

        if (option.state & QStyle::State_Selected) {
            QColor highlightColor = option.palette.highlight().color();
            highlightColor.setAlpha(150);
            painter->fillRect(option.rect, highlightColor);
        }
        else {
            painter->fillRect(option.rect, option.palette.base());
        }


        QRect outerBounds = option.rect.adjusted(SPACING, 0, -SPACING, 0);

        int iconAreaWidth = (ICON_SIZE + SPACING) * 2;
        int typeWidth = 60;

        // Draw the command name using the default text color.
        QRect textRect(outerBounds.x(), outerBounds.y(),
            outerBounds.width() - iconAreaWidth - typeWidth - SPACING,
            outerBounds.height());
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, QString::fromStdString(cm->name));

        // Now set the secondary text color for the script type string.
        QFont font = option.font;
        font.setBold(true);
        painter->setFont(font);
        QColor secondaryColor = option.palette.color(QPalette::Disabled, QPalette::Text);
        painter->setPen(secondaryColor);

        QRect identifierRect(outerBounds.right() - iconAreaWidth - typeWidth,
            outerBounds.y(),
            typeWidth,
            outerBounds.height());
        painter->drawText(identifierRect, Qt::AlignRight | Qt::AlignVCenter,
            QString::fromStdString(cm->identifier()));

        // Reset pen for the icons if needed.
        QColor iconColor = secondaryColor; // or choose a variant if you like
        int x = outerBounds.right() - ICON_SIZE;
        int y = outerBounds.top() + (outerBounds.height() - ICON_SIZE) / 2;

        QPixmap activePixmap = loadSvg(cm->active ? ":/icons/stop.svg" : ":/icons/play.svg", iconColor);
        QRect activeRect(x, y, ICON_SIZE, ICON_SIZE);
        painter->drawPixmap(activeRect, activePixmap);

        x -= (ICON_SIZE + SPACING);
        QPixmap visibilityPixmap = loadSvg(cm->visible ? ":/icons/visible.svg" : ":/icons/hidden.svg", iconColor);
        QRect visibilityRect(x, y, ICON_SIZE, ICON_SIZE);
        painter->drawPixmap(visibilityRect, visibilityPixmap);

        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        return QSize(option.rect.width(), ICON_SIZE + SPACING * 2);
    }
};
