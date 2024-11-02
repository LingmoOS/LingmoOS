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

#ifndef __KTABLEVIEW_H__
#define __KTABLEVIEW_H__

#include "kitemdelegate.h"
#include "kheaderview.h"

#include <QTreeView>
#include <QStyledItemDelegate>
#include <QHeaderView>

class QModelIndex;

#define COLUMN_FIRST_WIDTH_MIN     150 

/**
 * @brief Base class for common styled table view
 */
class KTableView : public QTreeView
{
    Q_OBJECT

public:
    /**
     * @brief Qefault constructor
     * @param parent Parent object
     */
    explicit KTableView(QWidget *parent = nullptr);

    /**
     * @brief unconstructor
     */
    virtual ~KTableView();

    /**
     * @brief setModel Sets the model for the view to present (overloaded)
     * @param model Item model to be set
     */
    void setModel(QAbstractItemModel *model) override;

    QPoint getHeaderMousePressPoint();

public Q_SLOTS:
    virtual void adjustColumnsSize();

protected:
    /**
     * @brief paintEvent Event handler for paint events
     * @param event Paint event
     */
    void paintEvent(QPaintEvent *event) override;
    /**
     * @brief drawRow Qraws the row in the tree view that contains the model item index, using the painter given.
     * @param painter Painter object
     * @param options Style option to control how the item is displayed
     * @param index Current model index
     */
    void drawRow(QPainter *painter,
                 const QStyleOptionViewItem &options,
                 const QModelIndex &index) const override;

    void focusInEvent(QFocusEvent *event) override;
    /**
     * @brief currentChanged This slot is called when a new item becomes the current item.
     * @param current New current item
     * @param previous Previous current item
     */
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
    /**
     * @brief viewportEvent Event handler for the scrolling area
     * @param event Viewport event
     * @return Returns true to indicate to the event system that the event has been handled,
     * and needs no further processing; otherwise returns false to indicate that the event
     * should be propagated further.
     */
    bool viewportEvent(QEvent *event) override;
    /**
     * @brief scrollTo Scrolls the view if necessary to ensure that the item at index is visible (overloaded)
     * @param index Model index to ensure visible
     * @param hint The hint parameter specifies more precisely where the item should be located after the operation.
     */
    void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override;

    void resizeEvent(QResizeEvent *e) override;

private:
    // Common styled item delegate for this table view
    KItemDelegate *m_itemDelegate {nullptr};
    // Common styled header view for  this table view
    KHeaderView *m_headerView {nullptr};

    // Current index (row) hovered by mouse
    QModelIndex m_hover;
    // Current index (row) being pressed by mouse
    QModelIndex m_pressed;

    // last widget size
    QSize m_lastSize;

    int m_focusReason = Qt::TabFocusReason;
};

#endif  // __KTABLEVIEW_H__
