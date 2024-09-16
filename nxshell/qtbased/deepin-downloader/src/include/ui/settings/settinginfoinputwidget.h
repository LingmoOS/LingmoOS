// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file settinginfoinputwidget.h
 *
 * @brief 设置信息可编辑输入窗口控件，最大下载限速以及最大上传限速用到此控件
 *
 *@date 2020-06-09 10:43
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
#ifndef SETTINGINFOINPUTWIDGET_H
#define SETTINGINFOINPUTWIDGET_H

#include <DLineEdit>
#include <DLabel>

#include <QWidget>

DWIDGET_USE_NAMESPACE

/**
 * @class SettingInfoInputWidget
 * @brief 设置信息可编辑输入窗口类
*/
class SettingInfoInputWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SettingInfoInputWidget(QWidget *parent = nullptr);

    /**
     * @brief 设置标题
     * @param text 标题文本
     */
    void setTitleLabelText(const QString &text);

    /**
     * @brief 设置单位
     * @param text 单位文本
     */
    void setUnitLabelText(const QString &text);

    /**
     * @brief 设置范围
     * @param text 范围文本
     */
    void setRangeLabelText(const QString &text);

    /**
     * @brief 设置行编辑器当前值
     * @param text 当前值
     */
    void setLineEditText(const QString &text);

    /**
     * @brief 设置行编辑器是否可编辑
     * @param isDisabled 布尔值true或者false
     */
    void setLineEditIsDisabled(bool isDisabled);

    /**
     * @brief 设置行编辑器数值输入范围
     * @param minValid 最小输入值；maxValid 最大输入值
     */
    void setValid(const int &minValid, const int &maxValid);

    /**
     * @brief 获取行编辑器数值
     * @return 行编辑器当前文本
     */
    QString getLineEditText();

    /**
     * @brief 设置左侧窗口的宽度
     * @param width 宽
     */
    void setWidgetWidth(const int &width);

    /**
     * @brief 设置范围字体样式
     * @param font 范围文本字体样式
     */
    void setRangeLabelFont(QFont font);

    /**
     * @brief 设置范围字体颜色
     * @param palette 范围文本字体颜色
     */
    void setRangeLabelPalette(DPalette palette);

    /**
     * @brief 设置行编辑器是否处于警告模式
     * @param isAlert 布尔值true或者false
     */
    void setLineEditAlert(bool isAlert);

    /**
     * @brief 获取行编辑器指针
     * @return 行编辑器指针
     */
    DLineEdit *getLineEdit();

    /**
     * @brief 获取行编辑器是否处于警告模式
     * @return 布尔值，是返回true，否则返回false
     */
    bool getIsAlert();

signals:
    void textChanged(QString text);
    void focusChanged(bool onFocus);

private slots:

    /**
     * @brief 行编辑器文本改变响应的槽函数
     * @param text 改变后的文本
     */
    void onLineEditTextChanged(const QString &text);

private:

    /**
     * @brief 初始化界面
     */
    void initUI();

    /**
     * @brief 初始化信号连接
     */
    void initConnections();

private:
    DLineEdit *m_lineEdit; // 行编辑器
    DLabel *m_titleLabel; // 标题
    DLabel *m_unitLabel; // 单位
    DLabel *m_rangeLabel; // 范围
    QWidget *m_widget;
};

#endif // SETTINGINFOINPUTWIDGET_H
