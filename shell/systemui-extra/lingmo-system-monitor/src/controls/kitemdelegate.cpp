/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kitemdelegate.h"

#include <QApplication>

#include <QAbstractItemView>
#include <QDebug>
#include <QHelpEvent>
#include <QModelIndex>
#include <QStyleOptionViewItem>
#include <QToolTip>

// constructor
KItemDelegate::KItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

// editor for editing item data, not used here
QWidget *KItemDelegate::createEditor(QWidget *, const QStyleOptionViewItem &,
                                        const QModelIndex &) const
{
    return nullptr;
}

// size hint for this delegate
QSize KItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QSize size = QStyledItemDelegate::sizeHint(option, index);
    size.setHeight(std::max(36, size.height()));
    return size;
}

// handler for help events
bool KItemDelegate::helpEvent(QHelpEvent *e, QAbstractItemView *view,
                                 const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (!e || !view)
        return false;
    // only process tooltip events for now
    if (e->type() == QEvent::ToolTip) {
        QRect rect = view->visualRect(index);
        QRect textRect = rect;

        // initialize style option
        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);

        // content margin
        auto margin = 4;
        // content spacing
        auto spacing = view->style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing);
        // icon size
        auto iconSize = 16;

        // calc textRect's size (exclude any decoration region)
        if (index.column() == 0) {
            if (opt.features & QStyleOptionViewItem::HasDecoration) {
                textRect.setX(textRect.x() + margin * 2 + spacing + iconSize);
            } else {
                textRect.setX(textRect.x() + 2 * margin);
            }
        } else {
            textRect.setX(textRect.x() + margin);
        }

        if (rect.x() + rect.width() >= view->width()) {
            textRect.setWidth(textRect.width() - margin * 2);
        } else {
            textRect.setWidth(textRect.width() - margin);
        }

        // max width of current text calculated with current option's font metric
        QFontMetrics fm(opt.font);
        int w = fm.size(Qt::TextSingleLine, opt.text).width();

        // show tooptip text when text is truncated (text width is larger than visible region)
        if (textRect.width() < w) {
            QVariant tooltip = index.data(Qt::DisplayRole);
            if (tooltip.canConvert<QString>()) {
                QToolTip::showText(e->globalPos(),
                                   QString("%1").arg(tooltip.toString().toHtmlEscaped()),
                                   view);
                return true;
            }
        }
        if (!QStyledItemDelegate::helpEvent(e, view, option, index))
            QToolTip::hideText();
        return true;
    }

    return QStyledItemDelegate::helpEvent(e, view, option, index);
}
