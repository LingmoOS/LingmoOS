/*
 * Qt5-LINGMO's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: xibowen <xibowen@kylinos.cn>
 *
 */

#include "lingmo-config-style.h"
#include <QListView>
#include <QTreeView>
static QSizeF viewItemTextLayout(QTextLayout &textLayout, int lineWidth, int maxHeight = -1, int *lastVisibleLine = nullptr)
{
    if (lastVisibleLine)
        *lastVisibleLine = -1;
    qreal height = 0;
    qreal widthUsed = 0;
    textLayout.beginLayout();
    int i = 0;
    while (true) {
        QTextLine line = textLayout.createLine();
        if (!line.isValid())
            break;
        line.setLineWidth(lineWidth);
        line.setPosition(QPointF(0, height));
        height += line.height();
        widthUsed = qMax(widthUsed, line.naturalTextWidth());
        // we assume that the height of the next line is the same as the current one
        if (maxHeight > 0 && lastVisibleLine && height + line.height() > maxHeight) {
            const QTextLine nextLine = textLayout.createLine();
            *lastVisibleLine = nextLine.isValid() ? i : -1;
            break;
        }
        ++i;
    }
    textLayout.endLayout();
    return QSizeF(widthUsed, height);
}


QString LINGMOConfigStyle::calculateElidedText(const QString &text, const QTextOption &textOption,
                                    const QFont &font, const QRect &textRect, const Qt::Alignment valign,
                                    Qt::TextElideMode textElideMode, int flags,
                                    bool lastVisibleLineShouldBeElided, QPointF *paintStartPosition) const
{
    QTextLayout textLayout(text, font);
    textLayout.setTextOption(textOption);

    // In AlignVCenter mode when more than one line is displayed and the height only allows
    // some of the lines it makes no sense to display those. From a users perspective it makes
    // more sense to see the start of the text instead something inbetween.
    const bool vAlignmentOptimization = paintStartPosition && valign.testFlag(Qt::AlignVCenter);

    int lastVisibleLine = -1;
    viewItemTextLayout(textLayout, textRect.width(), vAlignmentOptimization ? textRect.height() : -1, &lastVisibleLine);

    const QRectF boundingRect = textLayout.boundingRect();
    // don't care about LTR/RTL here, only need the height
    const QRect layoutRect = QStyle::alignedRect(Qt::LayoutDirectionAuto, valign,
                                                 boundingRect.size().toSize(), textRect);

    if (paintStartPosition)
        *paintStartPosition = QPointF(textRect.x(), layoutRect.top());

    QString ret;
    qreal height = 0;
    const int lineCount = textLayout.lineCount();
    for (int i = 0; i < lineCount; ++i) {
        const QTextLine line = textLayout.lineAt(i);
        height += line.height();

        // above visible rect
        if (height + layoutRect.top() <= textRect.top()) {
            if (paintStartPosition)
                paintStartPosition->ry() += line.height();
            continue;
        }

        const int start = line.textStart();
        const int length = line.textLength();
        const bool drawElided = line.naturalTextWidth() > textRect.width();
        bool elideLastVisibleLine = lastVisibleLine == i;
        if (!drawElided && i + 1 < lineCount && lastVisibleLineShouldBeElided) {
            const QTextLine nextLine = textLayout.lineAt(i + 1);
            const int nextHeight = height + nextLine.height() / 2;
            // elide when less than the next half line is visible
            if (nextHeight + layoutRect.top() > textRect.height() + textRect.top())
                elideLastVisibleLine = true;
        }

        QString text = textLayout.text().mid(start, length);
        if (drawElided || elideLastVisibleLine) {
            if (elideLastVisibleLine) {
                if (text.endsWith(QChar::LineSeparator))
                    text.chop(1);
                text += QChar(0x2026);
            }
            const QStackTextEngine engine(text, font);
            ret += engine.elidedText(textElideMode, textRect.width(), flags);

            // no newline for the last line (last visible or real)
            // sometimes drawElided is true but no eliding is done so the text ends
            // with QChar::LineSeparator - don't add another one. This happened with
            // arabic text in the testcase for QTBUG-72805
            if (i < lineCount - 1 &&
                !ret.endsWith(QChar::LineSeparator))
                ret += QChar::LineSeparator;
        } else {
            ret += text;
        }

        // below visible text, can stop
        if ((height + layoutRect.top() >= textRect.bottom()) ||
                (lastVisibleLine >= 0 && lastVisibleLine == i))
            break;
    }
    return ret;
}



void LINGMOConfigStyle::viewItemDrawText(QPainter *painter, const QStyleOptionViewItem *option, const QRect &rect) const
{
    const QWidget *widget = option->widget;
    const int textMargin = proxy()->pixelMetric(QStyle::PM_FocusFrameHMargin, 0, widget) + 1;

    QRect textRect = rect.adjusted(textMargin, 0, -textMargin, 0); // remove width padding
    const bool wrapText = option->features & QStyleOptionViewItem::WrapText;
    QTextOption textOption;
    textOption.setWrapMode(wrapText ? QTextOption::WordWrap : QTextOption::ManualWrap);
    //FIX ME,bug106749::QTablewidget text cannot be aligned, forced changed it wrap mode to wrapanywhere
    if(wrapText && widget && (widget->inherits("QTableWidget") || widget->inherits("QTableView")))
    {
        textOption.setWrapMode(QTextOption::WrapAnywhere);
    }
    textOption.setTextDirection(option->direction);
    textOption.setAlignment(QStyle::visualAlignment(option->direction, option->displayAlignment));

    QPointF paintPosition;
    const QString newText = calculateElidedText(option->text, textOption,
                                                option->font, textRect, option->displayAlignment,
                                                option->textElideMode, 0,
                                                true, &paintPosition);

    QTextLayout textLayout(newText, option->font);
    textLayout.setTextOption(textOption);
    viewItemTextLayout(textLayout, textRect.width());
    textLayout.draw(painter, paintPosition);
}



void LINGMOConfigStyle::viewItemLayout(const QStyleOptionViewItem *option,  QRect *checkRect, QRect *pixmapRect, QRect *textRect, bool sizehint) const
{
    Q_ASSERT(checkRect && pixmapRect && textRect);
    *pixmapRect = QRect(QPoint(0, 0), viewItemSize(option, Qt::DecorationRole));
    *textRect = QRect(QPoint(0, 0), viewItemSize(option, Qt::DisplayRole));
    *checkRect = QRect(QPoint(0, 0), viewItemSize(option, Qt::CheckStateRole));

    int Margin_Width = 2;
    int Margin_Height = 0;

    const QWidget *widget = option->widget;
    const bool hasCheck = checkRect->isValid();
    const bool hasPixmap = pixmapRect->isValid();
    const bool hasText = textRect->isValid();
    const bool hasMargin = (hasText | hasPixmap | hasCheck);
    const int frameHMargin = hasMargin ?
                proxy()->pixelMetric(QStyle::PM_FocusFrameHMargin, option, widget) + 1 : 0;
    const int textMargin = hasText ? frameHMargin : 0;
    const int pixmapMargin = hasPixmap ? frameHMargin : 0;
    const int checkMargin = hasCheck ? frameHMargin : 0;
    const int x = option->rect.left() + Margin_Width;
    const int y = option->rect.top() + Margin_Height;
    int w, h;

    if (textRect->height() == 0 && (!hasPixmap || !sizehint)) {
        //if there is no text, we still want to have a decent height for the item sizeHint and the editor size
        textRect->setHeight(option->fontMetrics.height());
    }

    QSize pm(0, 0);
    if (hasPixmap) {
        pm = pixmapRect->size();
        pm.rwidth() += 2 * pixmapMargin;
    }
    if (sizehint) {
        h = qMax(checkRect->height(), qMax(textRect->height(), pm.height()));
        if (option->decorationPosition == QStyleOptionViewItem::Left || option->decorationPosition == QStyleOptionViewItem::Right) {
            w = textRect->width() + pm.width();
        } else {
            w = qMax(textRect->width(), pm.width());
        }
    } else {
        w = option->rect.width() - Margin_Width * 2;
        h = option->rect.height() - Margin_Height * 2;
    }

    int cw = 0;
    QRect check;
    if (hasCheck) {
        cw = checkRect->width() + 2 * checkMargin;
        if (sizehint) w += cw;
        if (option->direction == Qt::RightToLeft) {
            check.setRect(x + w - cw, y, cw, h);
        } else {
            check.setRect(x, y, cw, h);
        }
    }

    QRect display;
    QRect decoration;
    switch (option->decorationPosition) {
    case QStyleOptionViewItem::Top:
    {
        h = sizehint ? textRect->height() : h - pm.height() - pixmapMargin;

        if (option->direction == Qt::RightToLeft) {
            decoration.setRect(x, y, w - cw, pm.height());
            display.setRect(x, y + pm.height() + pixmapMargin, w - cw, h);
        } else {
            decoration.setRect(x + cw, y, w - cw, pm.height());
            display.setRect(x + cw, y + pm.height() + pixmapMargin, w - cw, h);
        }
        break;
    }
    case QStyleOptionViewItem::Bottom:
    {
        h = sizehint ? textRect->height() + textMargin + pm.height() : h;

        if (option->direction == Qt::RightToLeft) {
            display.setRect(x, y, w - cw, textRect->height());
            decoration.setRect(x, y + textRect->height() + textMargin, w - cw, h - textRect->height() - textMargin);
        } else {
            display.setRect(x + cw, y, w - cw, textRect->height());
            decoration.setRect(x + cw, y + textRect->height() + textMargin, w - cw, h - textRect->height() - textMargin);
        }
        break;
    }
    case QStyleOptionViewItem::Left:
    {
        if (option->direction == Qt::LeftToRight) {
            decoration.setRect(x + cw, y, pm.width(), h);
            display.setRect(decoration.right() + 1, y, w - pm.width() - cw, h);
        } else {
            display.setRect(x, y, w - pm.width() - cw, h);
            decoration.setRect(display.right() + 1, y, pm.width(), h);
        }
        break;
    }
    case QStyleOptionViewItem::Right:
    {
        if (option->direction == Qt::LeftToRight) {
            display.setRect(x + cw, y, w - pm.width() - cw, h);
            decoration.setRect(display.right() + 1, y, pm.width(), h);
        } else {
            decoration.setRect(x, y, pm.width(), h);
            display.setRect(decoration.right() + 1, y, w - pm.width() - cw, h);
        }
        break;
    }

    default:
    {
        qWarning("doLayout: decoration position is invalid");
        decoration = *pixmapRect;
        break;
    }
    }

    if (!sizehint) { // we only need to do the internal layout if we are going to paint
        *checkRect = QStyle::alignedRect(option->direction, Qt::AlignCenter,
                                         checkRect->size(), check);
        *pixmapRect = QStyle::alignedRect(option->direction, option->decorationAlignment,
                                          pixmapRect->size(), decoration);
        // the text takes up all available space, unless the decoration is not shown as selected
        if (option->showDecorationSelected)
            *textRect = display;
        else
            *textRect = QStyle::alignedRect(option->direction, option->displayAlignment,
                                            textRect->size().boundedTo(display.size()), display);
    } else {
        *checkRect = check;
        *pixmapRect = decoration;
        *textRect = display;
    }
}



QSize LINGMOConfigStyle::viewItemSize(const QStyleOptionViewItem *option, int role) const
{
    const QWidget *widget = option->widget;
    switch (role) {
    case Qt::CheckStateRole:
    {
        if (option->features & QStyleOptionViewItem::HasCheckIndicator)
            return QSize(proxy()->pixelMetric(QStyle::PM_IndicatorWidth, option, widget),
                         proxy()->pixelMetric(QStyle::PM_IndicatorHeight, option, widget));
        break;
    }

    case Qt::DisplayRole:
    {
        if (option->features & QStyleOptionViewItem::HasDisplay) {
            QTextOption textOption;
            textOption.setWrapMode(QTextOption::WordWrap);
            QTextLayout textLayout(option->text, option->font);
            textLayout.setTextOption(textOption);
            const bool wrapText = option->features & QStyleOptionViewItem::WrapText;
            const int textMargin = proxy()->pixelMetric(QStyle::PM_FocusFrameHMargin, option, widget) + 1;
            QRect bounds = option->rect;
            switch (option->decorationPosition) {
            case QStyleOptionViewItem::Left:
            case QStyleOptionViewItem::Right: {
                if (wrapText && bounds.isValid()) {
                    int width = bounds.width() - 2 * textMargin;
                    if (option->features & QStyleOptionViewItem::HasDecoration)
                        width -= option->decorationSize.width() + 2 * textMargin;
                    bounds.setWidth(width);
                } else
                    bounds.setWidth(QFIXED_MAX);
                break;
            }
            case QStyleOptionViewItem::Top:
            case QStyleOptionViewItem::Bottom:
                if (wrapText)
                    bounds.setWidth(bounds.isValid() ? bounds.width() - 2 * textMargin : option->decorationSize.width());
                else
                    bounds.setWidth(QFIXED_MAX);
                break;
            default:
                break;
            }

            if (wrapText && option->features & QStyleOptionViewItem::HasCheckIndicator)
                bounds.setWidth(bounds.width() - proxy()->pixelMetric(QStyle::PM_IndicatorWidth) - 2 * textMargin);

            const int lineWidth = bounds.width();
            const QSizeF size = viewItemTextLayout(textLayout, lineWidth);
            return QSize(qCeil(size.width()) + 2 * textMargin, qCeil(size.height()));
        }
        break;
    }

    case Qt::DecorationRole:
    {
        if (option->features & QStyleOptionViewItem::HasDecoration) {
            if(widget){
                if(qobject_cast<const QListView *>(widget)){
                    QSize size = qobject_cast<const QListView *>(widget)->iconSize();
                    return QSize(qMax(option->decorationSize.width(), size.width()),
                                 qMax(option->decorationSize.height(), size.height()));
                }
                if(qobject_cast<const QTreeView *>(widget)){
                    QSize size = qobject_cast<const QTreeView *>(widget)->iconSize();
                    return QSize(qMax(option->decorationSize.width(), size.width()),
                                 qMax(option->decorationSize.height(), size.height()));
                }
            }
            return option->decorationSize;
        }
        break;
    }
    default:
        break;
    }
    return QSize(0, 0);
}


