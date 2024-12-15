// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef TITLEWIDGET_H
#define TITLEWIDGET_H

#include "partedproxy/dmdbushandler.h"
#include "customcontrol/partitionwidget.h"
#include "partitiondialog.h"
#include "utils.h"

#include <DWidget>
#include <DPushButton>
#ifdef DTKWIDGET_CLASS_DSizeMode
#include <DSizeMode>
#endif
//#include "customcontrol/tippartdialog.h"

DWIDGET_USE_NAMESPACE

/**
 * @class TitleWidget
 * @brief 标题功能图标按钮类
 */

class TitleWidget : public DWidget
{
    Q_OBJECT
public:
    explicit TitleWidget(DWidget *parent = nullptr);

private:

    /**
     * @brief 初始化页面
     */
    void initUi();

    /**
     * @brief 紧凑模式按钮设置
     */
    void setSizebyMode(DPushButton *button);
    void resetButtonsSize();

    /**
     * @brief 初始化连接
     */
    void initConnection();

    /**
     * @brief 创建按钮
     * @param btnName 按钮名称
     * @param objName 按钮图标名称
     * @param bCheckable 是否可选
     */
    DPushButton *createBtn(const QString &btnName, const QString &objName, bool bCheckable = false);

    /**
     * @brief 更新按钮是否可选状态
     */
    void updateBtnStatus();

    /**
     * @brief 设置当前操作设备路径
     */
    void setCurDevicePath(const QString &devPath);

    /**
     * @brief 设置当前操作VG名称
     */
    void setCurVGName(const QString &vgName);

    /**
     * @brief 显示解密窗口
     */
    bool showDecryptDialog();

    /**
     * @brief 显示加密盘没有文件系统警告窗口
     * @param luksInfo 加密盘属性数据
     */
    bool showNoFileSystemWarningDialog(const LUKS_INFO &luksInfo);

    /**
     * @brief 当前选择为分区时更新按钮是否可选状态
     * @param isCreate 分区按钮是否可选
     * @param isWipe 擦除按钮是否可选
     * @param isMount 挂载按钮是否可选
     * @param isUnmount 卸载按钮是否可选
     * @param isResize 空间调整按钮是否可选
     */
    void updatePartitionBtnStatus(const bool &isCreate, const bool &isWipe, const bool &isMount,
                                  const bool &isUnmount, const bool &isResize);

    /**
     * @brief 当前选择为逻辑卷组时更新按钮是否可选状态
     * @param isDeleteVG 删除逻辑卷组按钮是否可选
     * @param isCreateLV 创建逻辑卷按钮是否可选
     * @param isWipe 擦除按钮是否可选
     * @param isMount 挂载按钮是否可选
     * @param isUnmount 卸载按钮是否可选
     * @param isResize 空间调整按钮是否可选
     */
    void updateVGBtnStatus(const bool &isDeleteVG, const bool &isCreateLV, const bool &isWipe,
                           const bool &isMount, const bool &isUnmount, const bool &isResize);

    /**
     * @brief 当前选择为逻辑卷时更新按钮是否可选状态
     * @param isDeleteVG 删除逻辑卷按钮是否可选
     * @param isCreateLV 创建逻辑卷按钮是否可选
     * @param isWipe 擦除按钮是否可选
     * @param isMount 挂载按钮是否可选
     * @param isUnmount 卸载按钮是否可选
     * @param isResize 空间调整按钮是否可选
     */
    void updateLVBtnStatus(const bool &isDeleteLV, const bool &isCreateLV, const bool &isWipe,
                           const bool &isMount, const bool &isUnmount, const bool &isResize);

    /**
     * @brief 切换左侧树节点更新按钮是否显示
     * @param isDeleteVGShow 删除逻辑卷按钮是否显示
     * @param isDeleteLVShow 删除逻辑卷按钮是否显示
     * @param isDeletePVShow 删除物理卷按钮是否显示
     * @param isPartedShow 分区按钮是否显示
     * @param isCreateLVShow 创建逻辑卷按钮是否显示
     * @param isResizShow 分区空间调整按钮是否显示
     * @param isResizeVGShow 逻辑卷组空间调整按钮是否显示
     * @param isResizeLVShow 逻辑卷空间调整按钮是否显示
     */
    void updateBtnShowStatus(const bool &isDeleteVGShow, const bool &isDeleteLVShow, const bool &isDeletePVShow,
                             const bool &isPartedShow, const bool &isCreateLVShow, const bool &isResizShow,
                             const bool &isResizeVGShow, const bool &isResizeLVShow);

    /**
     * @brief 当前选择为加密盘时更新按钮是否可选状态
     * @param LUKS_INFO 加密盘属性
     */
    void updateEncryptDeviceBtnStatus(const LUKS_INFO &luksInfo);

signals:

private slots:

    /**
     * @brief 更新按钮是否可选状态
     */
    void onCurSelectChanged();

    /**
     * @brief 分区按钮点击响应的槽函数
     */
    void showPartInfoWidget();

    /**
     * @brief 格式化按钮点击响应的槽函数
     */
    void showFormateInfoWidget();

    /**
     * @brief 挂载按钮点击响应的槽函数
     */
    void showMountInfoWidget();

    /**
     * @brief 卸载按钮点击响应的槽函数
     */
    void showUnmountInfoWidget();

    /**
     * @brief 空间调整按钮点击响应的槽函数
     */
    void showResizeInfoWidget();

    /**
     * @brief usb热插拔信号响应的槽函数
     */
    void onUpdateUsb();

    /**
     * @brief 创建逻辑卷按钮点击响应的槽函数
     */
    void onCreateLVClicked();

    /**
     * @brief 删除逻辑卷按钮点击响应的槽函数
     */
    void onDeleteLVClicked();

    /**
     * @brief 逻辑卷空间调整按钮点击响应的槽函数
     */
    void onResizeLVClicked();

    /**
     * @brief 创建逻辑卷组按钮点击响应的槽函数
     */
    void onCreateVGClicked();

    /**
     * @brief 删除逻辑卷组按钮点击响应的槽函数
     */
    void onDeleteVGClicked();

    /**
     * @brief 逻辑卷组空间调整按钮点击响应的槽函数
     */
    void onResizeVGClicked();

    /**
     * @brief 删除物理卷按钮点击响应的槽函数
     */
    void onDeletePVClicked();


protected:

    /**
     * @brief 鼠标点击事件
     */
    void mousePressEvent(QMouseEvent *event) override;

private:
    DPushButton *m_btnParted; // 分区按钮
    DPushButton *m_btnFormat; // 格式化按钮
    DPushButton *m_btnMount; // 挂载按钮
    DPushButton *m_btnUnmount; // 卸载按钮
    DPushButton *m_btnResize; // 空间调整按钮
    DPushButton *m_btnCreateVG; // 创建逻辑卷组按钮
    DPushButton *m_btnDeleteVG; // 删除逻辑卷组按钮
    DPushButton *m_btnDeleteLV; // 删除逻辑卷按钮
    DPushButton *m_btnDeletePV; // 删除物理卷按钮
    DPushButton *m_btnCreateLV; // 创建逻辑卷按钮
    DPushButton *m_btnResizeVG; // 逻辑卷组空间调整按钮
    DPushButton *m_btnResizeLV; // 逻辑卷空间调整按钮
    QString m_curChooseDevicePath; // 当前操作磁盘
    QString m_curChooseVGName; // 当前操作VG
};

#endif // TITLEWIDGET_H
