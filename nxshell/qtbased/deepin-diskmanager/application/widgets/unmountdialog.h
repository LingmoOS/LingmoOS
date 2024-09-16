// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef UNMOUNTDIALOG_H
#define UNMOUNTDIALOG_H

#include "customcontrol/ddbase.h"

DWIDGET_USE_NAMESPACE

/**
 * @class UnmountDialog
 * @brief 卸载类
 */

class UnmountDialog : public DDBase
{
    Q_OBJECT
public:
    explicit UnmountDialog(QWidget *parent = nullptr);

private:
    /**
     * @brief 初始化界面
     */
    void initUi();

    /**
     * @brief 初始化连接
     */
    void initConnection();

    /**
     * @brief 卸载当前选择的分区或者逻辑卷
     */
    void umountCurMountPoints();

private slots:
    /**
     * @brief 按钮点击响应的槽函数
     * @param index 当前点击按钮
     * @param text 当前点击按钮文本
     */
    void onButtonClicked(int index, const QString &text);
};

#endif // UNMOUNTDIALOG_H
