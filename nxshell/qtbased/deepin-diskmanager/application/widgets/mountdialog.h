// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef MOUNTDIALOG_H
#define MOUNTDIALOG_H
#include "customcontrol/ddbase.h"

#include <dfilechooseredit.h>
#include <DComboBox>
#include <DLabel>
#include <commondef.h>

DWIDGET_USE_NAMESPACE

/**
 * @class MountDialog
 * @brief 挂载类
 */

class MountDialog : public DDBase
{
    Q_OBJECT
public:
    explicit MountDialog(QWidget *parent = nullptr);

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
     * @brief 判断挂载点是否存在
     * @return 存在返回true，否则返回false
     */
    bool isExistMountPoint(const QString &mountPoint);

    /**
     * @brief 判断是否是系统目录
     * @return 是返回true，否则返回false
     */
    bool isSystemDirectory(const QString &directory);

    /**
     * @brief 挂载当前选择的分区或者逻辑卷
     */
    void mountCurPath();

private slots:
    /**
     * @brief 挂载点输入框改变信号响应的槽函数
     * @param content 当前改变的文本
     */
    void onEditContentChanged(const QString &content);

    /**
     * @brief 按钮点击响应的槽函数
     * @param index 当前点击按钮
     * @param text 当前点击按钮文本
     */
    void onButtonClicked(int index, const QString &text);

    /**
    * @brief 检查特定分区格式对挂载点的要求
    * @param fsType 文件系统类型
    * @param mountPoint 挂载点
    */
    bool onCheckMountPoint(FSType fsType, const QString &mountPoint);

private:
    DFileChooserEdit *m_fileChooserEdit;
    DLabel *m_mountPointSuggest;
    DLabel *m_warnning;
    //    DComboBox *m_ComboBox;
};

#endif // MOUNTDIALOG_H
