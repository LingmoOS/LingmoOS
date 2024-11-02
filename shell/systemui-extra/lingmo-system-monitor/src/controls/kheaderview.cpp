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

#include "kheaderview.h"

#include <QDebug>
#include <QtMath>
#include <QModelIndex>
#include <QToolTip>
#include <QEvent>
#include <QMouseEvent>
#include <QHelpEvent>
#include <QFontMetrics>

// Constructor
KHeaderView::KHeaderView(Qt::Orientation orientation, QWidget *parent)
    : QHeaderView(orientation, parent)
{
    installEventFilter(this);
}

// Get header view's hint size
QSize KHeaderView::sizeHint() const
{
    return QSize(width(), 36 + m_spacing);
}

// Get section's hint size
int KHeaderView::sectionSizeHint(int logicalIndex) const
{
    return QHeaderView::sectionSizeHint(logicalIndex);
}

void KHeaderView::paintEvent(QPaintEvent *e)
{
    // -1 清空map
    this->model()->setHeaderData(-1, this->orientation(), "");
    for (int index = 0; index < this->model()->columnCount(); ++index) {
        QString itemText = this->model()->headerData(index, this->orientation()).toString();
        const  int textMargin = this->style()->pixelMetric(QStyle::PM_FocusFrameHMargin) +  4;
        int rectWidth = this->sectionSize(index) - textMargin *  2;
        if (index == this->sortIndicatorSection()) {
            rectWidth = rectWidth - 24; //减去排序图标大小
        }
        QFontMetrics fm(this->font());
        int flag = this->model()->headerData(index, this->orientation(), Qt::TextAlignmentRole).toInt();
        QSize itemTextSize = fm.size(flag, itemText);
        if ((itemTextSize.width() > rectWidth) && !itemText.isEmpty()) {
            QFontMetrics fontMetrics(this->font());
            tooltipTextMap[index] = itemText;
            if (rectWidth < 16) {
                rectWidth = 16; // 小于16文字内容会修正为空
            }
            this->model()->setHeaderData(index,
                                         this->orientation(),
                                         fontMetrics.elidedText(itemText, Qt::ElideRight, rectWidth));

        }
    }
    QHeaderView::paintEvent(e);
}

bool KHeaderView::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        QHelpEvent *helpEvent = static_cast<QHelpEvent*>(event);
        QPoint pos = helpEvent->pos();
        int index = this->logicalIndexAt(pos);
        if (index <  0) {
            return QHeaderView::eventFilter(object, event);
        }
        QString itemText = this->model()->headerData(index, this->orientation()).toString();
        if (itemText.contains("…")) {
            QToolTip::showText(helpEvent->globalPos(), tooltipTextMap[index], this);
        } else {
            QToolTip::hideText();
        }
    }
    return QHeaderView::eventFilter(object, event);
}

void KHeaderView::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        pressPoint = e->pos();
    }
    QHeaderView::mouseReleaseEvent(e);
}

QPoint KHeaderView::getMousePressPoint()
{
    return pressPoint;
}
