// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASE_TABLE_VIEW_H
#define BASE_TABLE_VIEW_H

#include <DTreeView>

DWIDGET_USE_NAMESPACE

class BaseItemDelegate;
class BaseHeaderView;
class QModelIndex;

/**
 * @brief Base class for common styled table view
 */
class BaseTableView : public DTreeView
{
    Q_OBJECT

public:
    /**
     * @brief Default constructor
     * @param parent Parent object
     */
    explicit BaseTableView(DWidget *parent = nullptr);

    /**
     * @brief setModel Sets the model for the view to present (overloaded)
     * @param model Item model to be set
     */
    void setModel(QAbstractItemModel *model) override;

protected:
    /**
     * @brief paintEvent Event handler for paint events
     * @param event Paint event
     */
    void paintEvent(QPaintEvent *event) override;
    /**
     * @brief drawRow Draws the row in the tree view that contains the model item index, using the painter given.
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

private:
    // Common styled item delegate for this table view
    BaseItemDelegate *m_itemDelegate {nullptr};
    // Common styled header view for  this table view
    BaseHeaderView *m_headerView {nullptr};

    // Current index (row) hovered by mouse
    QModelIndex m_hover;
    // Current index (row) being pressed by mouse
    QModelIndex m_pressed;

    int m_focusReason = Qt::TabFocusReason;
};

#endif  // BASE_TABLE_VIEW_H
