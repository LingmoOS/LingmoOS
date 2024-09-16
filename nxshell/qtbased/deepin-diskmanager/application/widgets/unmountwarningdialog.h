// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef UNMOUNTWARNINGDIALOG_H
#define UNMOUNTWARNINGDIALOG_H

#include "customcontrol/ddbase.h"

#include <DCheckBox>
#include <DLabel>

DWIDGET_USE_NAMESPACE

/**
 * @class UnmountWarningDialog
 * @brief 卸载警告窗口类
 */

class UnmountWarningDialog : public DDBase
{
    Q_OBJECT
public:
    explicit UnmountWarningDialog(QWidget *parent = nullptr);

private slots:
    /**
     * @brief 复选框状态改变响应的槽函数
     * @param state 复选框当前状态
     */
    void onCheckBoxStateChange(int state);

private:
    /**
     * @brief 初始化界面
     */
    void initUi();

    /**
     * @brief 初始化连接
     */
    void initConnection();

private:
    DCheckBox *m_checkBox;
};

#endif // UNMOUNTWARNINGDIALOG_H
