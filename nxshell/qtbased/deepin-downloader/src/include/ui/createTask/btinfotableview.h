// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file btinfotableview.h
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

#ifndef BTINFOTABLEVIEW_H
#define BTINFOTABLEVIEW_H

#include <DTableView>
#include <DHeaderView>
#include <QMouseEvent>
#include <QDebug>

DWIDGET_USE_NAMESPACE

class BtInfoTableView : public DTableView
{
    Q_OBJECT
public:
    explicit BtInfoTableView(QWidget *parent = nullptr);

    //void reset();

protected:
    /**
     * @brief 重写鼠标移动事件，鼠标移动事件，鼠标若移动该行，给窗口发送信号
     * @return event 鼠标事件
     */
    void mouseMoveEvent(QMouseEvent *event);
    /**
     * @brief 重写鼠标离开事件，鼠标离开时间，鼠标若离开，给窗口发送信号
     * @return event 鼠标事件
     */
    void leaveEvent(QEvent *event);

    /**
     * @brief 重写鼠标点击事件
     * @return event 鼠标事件
     */
    void mouseReleaseEvent(QMouseEvent *event);


signals:
    /**
     * @brief 发送选中表格信号
     * @return index 所选中行
     */
    void hoverChanged(const QModelIndex &index);

    void doubleIndex(const QModelIndex &index);

public slots:

    void onDoubleClicked(const QModelIndex &index);

private:
    int m_curRow;
};

#endif // BTINFOTABLEVIEW_H
