// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @copyright 2020-2020 Uniontech Technology Co., Ltd.
 *
 * @file downloadsettingwidget.h
 *
 * @brief 下载设置窗口，主要实现全速下载和限速下载的切换，以及限速下载时限制参数的设置
 *
 *@date 2020-06-09 11:03
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
#ifndef DOWNLOADSETTINGWIDGET_H
#define DOWNLOADSETTINGWIDGET_H

#include <DRadioButton>
#include <DAlertControl>

#include <QWidget>
#include <QTimeEdit>
#include "timeEdit.h"
DWIDGET_USE_NAMESPACE

class SettingInfoInputWidget;

/**
 * @class DownloadSettingWidget
 * @brief 下载设置窗口类
*/
class DownloadSettingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DownloadSettingWidget(QWidget *parent = nullptr);

    /**
     * @brief 设置当前选择的选项，当传入为1时，选中全速下载
     * @param currentSelect 当前选项值
     */
    bool setCurrentSelectRadioButton(const int &currentSelect);

    /**
     * @brief 设置最大下载限速值
     * @param text 限速值
     */
    bool setMaxDownloadSpeedLimit(const QString &text);

    /**
     * @brief 设置最大上传限速值
     * @param text 限速值
     */
    bool setMaxUploadSpeedLimit(const QString &text);

    /**
     * @brief 设置限速开始时间
     * @param text 开始时间
     */
    bool setStartTime(const QString &text);

    /**
     * @brief 设置限速结束时间
     * @param text 结束时间
     */
    bool setEndTime(const QString &text);

signals:
    void speedLimitInfoChanged(QString text);

public slots:
    void onValueChanged(QVariant var);
private slots:

    /**
     * @brief 限速时间改变响应的槽函数
     * @param time 改变后的时间
     */
    void onTimeChanged(const QString &time);

    /**
     * @brief 限速值改变响应的槽函数
     * @param text 改变后的值
     */
    void onTextChanged(QString text);

    /**
     * @brief 单选按钮切换槽函数
     */
    void onRadioButtonClicked();

    /**
     * @brief 焦点是否在输入框响应的槽函数
     * @param onFocus 布尔值
     */
    void onFocusChanged(bool onFocus);

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
    DRadioButton *m_fullSpeedDownloadButton; // 全速下载按钮
    DRadioButton *m_speedLimitDownloadButton; // 限速下载按钮
    SettingInfoInputWidget *m_maxDownloadSpeedLimit; //最大下载限速输入框
    SettingInfoInputWidget *m_maxUploadSpeedLimit; // 最大上传限速输入框
    CTimeEdit *m_startTimeEdit; // 开始时间
    CTimeEdit *m_endTimeEdit; // 结束时间
    DAlertControl *m_downloadAlertControl; // 最大下载限速输入错误提示
    DAlertControl *m_uploadAlertControl; // 最大上传限速输入错误提示
    DAlertControl *m_startTimeAlertControl; // 开始时间输入错误提示
    DAlertControl *m_endTimeAlertControl; // 结束时间输入错误提示
    int m_DownloadSpeedLimitValue;
    int m_UploadSpeedLimitValue;
};

#endif // DOWNLOADSETTINGWIDGET_H
