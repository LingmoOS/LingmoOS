// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef FORMATEDIALOG_H
#define FORMATEDIALOG_H

#include "customcontrol/ddbase.h"
#include "customcontrol/waterloadingwidget.h"

#include <DComboBox>
#include <DLineEdit>
#include <DLabel>
#include <DPushButton>
#include <DWarningButton>
#include <DSpinner>
#include <DStackedWidget>

DWIDGET_USE_NAMESPACE

/**
 * @class FormateDialog
 * @brief 格式化窗口类
 */

class FormateDialog : public DDBase
{
    Q_OBJECT
public:
    explicit FormateDialog(QWidget *parent = nullptr);

    enum WipeType {
        FAST = 1,
        SECURE,
        DOD,
        GUTMANN
    };

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
     * @brief 更新加密算法信息
     */
    void updateEncryptionInfo(const QString &text, const int &width);

private:
    DComboBox *m_formatComboBox;
    DLineEdit *m_fileNameEdit;
    DComboBox *m_securityComboBox;
    DComboBox *m_wipingMethodComboBox;
    DLabel *m_describeInfo;
    QWidget *m_wipingMethodWidget;
    DLabel *m_labelTmp;
    DLabel *m_label;
    DPushButton *m_pushButton;
    DWarningButton *m_warningButton;
    DStackedWidget *m_stackedWidget;
    DLabel *m_failLabel;
    QString m_pathInfo;
    int m_curWipeMethod;
    QString m_curDiskMediaType;
    QHBoxLayout *m_buttonLayout;
    WaterLoadingWidget *m_waterLoadingWidget;
    DLabel *m_encryptionInfo;
    DLabel *m_emptyLabel;
    int m_height;
    int m_encryptionInfoHeight;
signals:

private slots:

    /**
     * @brief 取消按钮点击槽函数
     */
    void onCancelButtonClicked();

    /**
     * @brief 擦除按钮点击槽函数
     */
    void onWipeButtonClicked();

    /**
     * @brief 当前文本改变
     * @param text 当前文本
     */
    void onTextChanged(const QString &text);

    /**
     * @brief 下拉框分区格式切换
     * @param text 当前选择文本
     */
    void onComboxFormatTextChange(const QString &text); //下拉框分区格式切换

    /**
     * @brief 下拉框安全选择切换
     * @param index 当前选择下标
     */
    void onSecurityCurrentIndexChanged(int index);

    /**
     * @brief 下拉框覆盖次数切换
     * @param index 当前选择下标
     */
    void onWipingMethodCurrentIndexChanged(int index);

    /**
     * @brief 擦除结果显示
     * @param info 擦除结果信息
     */
    void onWipeResult(const QString &info);
};

#endif // FORMATEDIALOG_H
