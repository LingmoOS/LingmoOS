// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef RESIZEDIALOG_H
#define RESIZEDIALOG_H

#include "customcontrol/ddbase.h"

#include <DLabel>
#include <DLineEdit>
#include <DComboBox>

DWIDGET_USE_NAMESPACE

/**
 * @class ResizeDialog
 * @brief 空间调整类
 */

class ResizeDialog : public DDBase
{
    Q_OBJECT
public:
    ResizeDialog(QWidget *parent = nullptr);

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
     * @brief 分区空间调整
     */
    void partitionResize();

    /**
     * @brief 逻辑卷空间调整
     */
    void lvResize();

    /**
     * @brief 不支持文件系统调整提示窗口
     */
    void noSupportFSDailog();

    /**
     * @brief 更新调整区间
     */
    void updateInputRange();

    DLineEdit *m_lineEdit;
    DComboBox *m_comboBox;
    DLabel *m_label;
    DLabel *m_lvResizeLable;
    QString m_minSize = "0";
    QString m_maxSize = "0";

private slots:
    /**
     * @brief 按钮点击响应的槽函数
     * @param index 当前点击按钮
     * @param text 当前点击按钮文本
     */
    void onButtonClicked(int index, const QString &text);

    /**
     * @brief 下拉框当前文本改变响应的槽函数
     * @param index 当前选择文本
     */
    void onComboSelectedChanged(int index);

    /**
     * @brief 输入框文本改变响应的槽函数
     */
    void onEditTextChanged(const QString &);
};

#endif // RESIZEDIALOG_H
