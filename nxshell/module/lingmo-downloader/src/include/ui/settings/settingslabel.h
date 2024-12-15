// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file settingslabel.h
 *
 * @brief 设置页面文本显示类
 *
 *@date 2020-06-30 10:10
 *
 * Author: yuandandan  <yuandandan@uniontech.com>
 *
 * Maintainer: yuandandan  <yuandandan@uniontech.com>
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
#ifndef SETTINGSLABEL_H
#define SETTINGSLABEL_H

#include <DLabel>
#include <DPalette>

#include <QWidget>

DWIDGET_USE_NAMESPACE

/**
 * @class SettingsLabel
 * @brief 设置页面文本显示类
*/
class SettingsLabel : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsLabel(QWidget *parent = nullptr);

    /**
     * @brief 设置文本
     * @param text 文本
     */
    void setLabelText(const QString &text);

    /**
     * @brief 设置字体样式
     * @param font 文本字体样式
     */
    void setLabelFont(QFont font);

    /**
     * @brief 设置字体颜色
     * @param palette 文本字体颜色
     */
    void setLabelPalette(DPalette palette);

signals:

public slots:

private:
    DLabel *m_label;
};

#endif // SETTINGSLABEL_H
