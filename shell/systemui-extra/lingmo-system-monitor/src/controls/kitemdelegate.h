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

#ifndef __KITEMDELEGATE_H__
#define __KITEMDELEGATE_H__

#include <QStyledItemDelegate>

class QModelIndex;

/**
 * @brief Base item delegate for treeview
 */
class KItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    /**
     * @brief Default constructor
     * @param parent Parent object
     */
    explicit KItemDelegate(QObject *parent = nullptr);

    /**
     * @brief createEditor Returns the editor to be used for editing the data item with the given index.
     * @param parent Parent widget
     * @param option Style option
     * @param index Current index
     * @return New editor instance
     */
    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
                          
    /**
     * @brief sizeHint Size hint for this delegate
     * @param option Style option for this delegate
     * @param index Current index
     * @return Hint size
     */
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    /**
     * @brief helpEvent Function to handle help events
     * @param e Help event
     * @param view View that generated this help event
     * @param option Style option
     * @param index Current index
     * @return Returns true if the delegate can handle the event; otherwise returns false.
     */
    bool helpEvent(QHelpEvent *e,
                   QAbstractItemView *view,
                   const QStyleOptionViewItem &option,
                   const QModelIndex &index) override;
};

#endif  // __KITEMDELEGATE_H__
