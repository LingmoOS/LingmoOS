// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef PASSWORDINPUTDIALOG_H
#define PASSWORDINPUTDIALOG_H

#include "ddbase.h"

#include <DPasswordEdit>
#include <DTextEdit>
#include <DLabel>

DWIDGET_USE_NAMESPACE

/**
 * @class PasswordInputDialog
 * @brief 密码输入窗口类
 */

class PasswordInputDialog : public DDBase
{
    Q_OBJECT
public:
    explicit PasswordInputDialog(QWidget *parent = nullptr);

    /**
     * @brief 设置设备名称
     * @param devName 设备名称
     */
    void setDeviceName(const QString &devName);

    /**
     * @brief 设置标题
     * @param text 标题
     */
    void setTitleText(const QString &text);

    /**
     * @brief 获取密码
     * @return 输入密码
     */
    QString getPassword();

    /**
     * @brief 获取密码提示
     * @return 密码提示
     */
    QString getPasswordHint();

signals:

private slots:
    /**
     * @brief 输入密码框文本改变响应的槽函数
     */
    void onInputPasswordEditTextChanged(const QString &);

    /**
     * @brief 验证密码框文本改变响应的槽函数
     */
    void onCheckPasswordEditTextChanged(const QString &);

    /**
     * @brief 密码提示文本改变响应的槽函数
     */
    void passwordHintTextChanged();

    /**
     * @brief 按钮点击响应的槽函数
     * @param index 当前点击按钮
     * @param text 当前点击按钮文本
     */
    void onButtonClicked(int index, const QString &text);

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
    DPasswordEdit *m_inputPasswordEdit;
    DPasswordEdit *m_checkPasswordEdit;
    DTextEdit *m_textEdit;
};

#endif // PASSWORDINPUTDIALOG_H
