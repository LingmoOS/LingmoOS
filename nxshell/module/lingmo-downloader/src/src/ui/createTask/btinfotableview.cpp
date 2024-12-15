// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file btinfotableview.cpp
 *
 * @brief BT窗口中tableview类
 *
 * @date 2020-06-09 10:50
 *
 * Author: bulongwei  <bulongwei@uniontech.com>
 *
 * Maintainer: bulongwei  <bulongwei@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "btinfotableview.h"

BtInfoTableView::BtInfoTableView(QWidget *parent)
    : DTableView(parent)
{
    setEditTriggers(QAbstractItemView::SelectedClicked);
    connect(this, &QAbstractItemView::doubleClicked, this, &BtInfoTableView::onDoubleClicked);
    QFont font;
    font.setFamily("Source Han Sans");
    setFont(font);
}

void BtInfoTableView::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    //  reset();
    QModelIndex idx = indexAt(event->pos());
    emit hoverChanged(idx);
}

void BtInfoTableView::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    reset();
    emit hoverChanged(QModelIndex());
}

void BtInfoTableView::onDoubleClicked(const QModelIndex &index)
{
    emit doubleIndex(index);
    m_curRow = index.row();
    edit(index);
}

void BtInfoTableView::mouseReleaseEvent(QMouseEvent *event)
{
#if !defined(CMAKE_SAFETYTEST_ARG_ON)
    Q_UNUSED(event);
    QModelIndex idx = indexAt(event->pos());
    if (idx.row() == m_curRow) {
        return;
    }
    reset();
#endif
}
