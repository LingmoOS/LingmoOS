// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file notificationssettiingwidget.h
 *
 * @brief 通知提醒
 *
 * @date 2020-06-09 10:52
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
 */

#ifndef NOTIFICATIONSSETTIINGWIDGET_H
#define NOTIFICATIONSSETTIINGWIDGET_H

#include <QWidget>
#include <DLabel>
DWIDGET_USE_NAMESPACE


/**
 * @class NotificationsSettiingWidget
 * @brief 系统通知设置类
 */
class NotificationsSettiingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NotificationsSettiingWidget(QWidget *parent = nullptr);

signals:

public slots:
};

#endif // NOTIFICATIONSSETTIINGWIDGET_H
