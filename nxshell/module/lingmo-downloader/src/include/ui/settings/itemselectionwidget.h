// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file itemselectionwidget.h
 *
 * @brief 自定义复选框窗口控件，主要实现标题名称居左，复选框居右，且复选框为圆形的控件窗口
 *
 *@date 2020-06-09 10:50
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
#ifndef ITEMSELECTIONWIDGET_H
#define ITEMSELECTIONWIDGET_H

#include <DCheckBox>
#include <DLabel>

#include <QWidget>

DWIDGET_USE_NAMESPACE

/**
 * @class ItemSelectionWidget
 * @brief 自定义复选框窗口类
*/
class ItemSelectionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ItemSelectionWidget(QWidget *parent = nullptr, bool isHttp = false);

    /**
     * @brief 设置标题
     * @param text 标题文本
     */
    void setLabelText(const QString &text);

    /**
     * @brief 设置复选框是否选中
     * @param isChecked 布尔值true或者false
     */
    void setCheckBoxChecked(bool isChecked);

    /**
     * @brief 设置复选框当前状态
     * @param state 状态
     */
    void setCheckboxState(Qt::CheckState state);

    /**
     * @brief 设置是否阻塞复选框信号
     * @param lock 布尔值true或者false
     */
    void setBlockSignals(bool lock);

    /**
     * @brief 设置AccessibleName
     * @param name AccessibleName
     */
    void setAccessibleName(QString name);


signals:
    void checkBoxIsChecked(bool isChecked);

public slots:

    /**
     * @brief 复选框状态改变响应的槽函数
     * @param state 状态值
     */
    void onCheckBoxStateChanged(int state);

private:

    /**
     * @brief 初始化界面
     */
    void initUI(bool ret);

    /**
     * @brief 初始化信号连接
     */
    void initConnections();

private:
    DLabel *m_label; // 标题
    DCheckBox *m_checkBox; // 复选框

};

#endif // ITEMSELECTIONWIDGET_H
