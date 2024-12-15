// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file btheaderview.h
 *
 * @brief bt表格头
 *
 * @date 2020-06-09 10:48
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

#ifndef BTHEADERVIEW_H
#define BTHEADERVIEW_H

#include <QObject>
#include <QHeaderView>
#include <DApplication>
#include <dguiapplicationhelper.h>

DWIDGET_USE_NAMESPACE

static int m_curWidget = 0;
class headerView : public QHeaderView
{
    Q_OBJECT
public:
    explicit headerView(Qt::Orientation orientation, QWidget *parent = 0);

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;

private:
    QRect checkBoxRect(const QRect &sourceRect) const;

    /**
     * @brief 调色板改变，重新设置颜色
     */
    void onPalettetypechanged(DGuiApplicationHelper::ColorType type);
};

#endif // BTHEADERVIEW_H
