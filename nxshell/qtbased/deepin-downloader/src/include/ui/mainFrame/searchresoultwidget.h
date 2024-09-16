// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2021-2021 Uniontech Technology Co., Ltd.
 *
 * @file searchresoultwidget.h
 *
 * @brief 搜索框类
 *
 * @date 2021-01-08 15:21
 *
 * Author: zhaoyue  <zhaoyue@uniontech.com>
 *
 * Maintainer: zhaoyue  <zhaoyue@uniontech.com>
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
 **/

#ifndef SEARCHRESOULTWIDGET_H
#define SEARCHRESOULTWIDGET_H

#include <DListWidget>
#include <DDialog>

DWIDGET_USE_NAMESPACE

/**
 * @class SearchResoultWidget
 * @brief 搜索框类
*/
class SearchResoultWidget : public QListWidget
{
    Q_OBJECT
public:
    SearchResoultWidget(QWidget *parent = nullptr);

    /**
     * @brief 设置搜索框内的显示内容
     * @param taskIDList: 任务id列表
     * @param taskStatusList: 任务状态列表
     * @param tasknameList: 任务名称列表
     */
    void setData(QList<QString> &taskIDList,
                 QList<int> &taskStatusList, QList<QString> &tasknameList);

public slots:
    void onKeypressed(Qt::Key k);
protected:

    /**
     * @brief 焦点移出事件
    */
    void focusOutEvent(QFocusEvent *event);

    void keyPressEvent(QKeyEvent *e);
};
#endif // SEARCHRESOULTWIDGET_H
