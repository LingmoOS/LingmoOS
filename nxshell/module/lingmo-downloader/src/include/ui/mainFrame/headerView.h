// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file headerView.h
 *
 * @brief 表头
 *
 * @date 2020-06-09 09:59
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

#ifndef HEADERVIEW_H
#define HEADERVIEW_H

#include <QHeaderView>
#include <DCheckBox>
#include <DApplication>
#include <dguiapplicationhelper.h>

DWIDGET_USE_NAMESPACE

/**
 * @class HeaderView
 * @brief 表头
 */
class DownloadHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    explicit DownloadHeaderView(Qt::Orientation orientation,
                        QWidget        *parent = 0);
    ~DownloadHeaderView();

protected:
    /**
     * @brief 更新位置
     */
    void updateGeometries();

private:
    DCheckBox *m_headerCbx;
signals:
    /**
     * @brief 表头全选按键状态改变按钮
     */
    void Statechanged(bool checked);
public slots:
    /**
     * @brief 清除表头选中状态
     */
//    void onClearHeaderChecked();

    /**
     * @brief 表头全选按键选中
     */
    void onHeaderChecked(bool checked);

    /**
     * @brief 调色板改变，重新设置颜色
     */
    void onPalettetypechanged(DGuiApplicationHelper::ColorType type);
};

#endif // HEADERVIEW_H
