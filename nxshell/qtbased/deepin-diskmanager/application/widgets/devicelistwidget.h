// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef DEVICELISTWIDGET_H
#define DEVICELISTWIDGET_H
#include "customcontrol/dmtreeview.h"
#include "partedproxy/dmdbushandler.h"
#include <dirent.h>
#include <DWidget>
#include <QTextCodec>
#include <QByteArray>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
DTK_USE_NAMESPACE

/**
 * @class DeviceListWidget
 * @brief 磁盘设备树列表类
 */

class DeviceListWidget : public DWidget
{
    Q_OBJECT
public:
    explicit DeviceListWidget(QWidget *parent = nullptr);
    ~DeviceListWidget();

    bool isHideSuccess = false;
    bool isShowSuccess = false;
    bool isDeleteSuccess = false;
    bool isUnmountSuccess = false;
    bool isCreateTableSuccess = false;
    bool isDeleteVGSuccess = false;
    bool isDeleteLVSuccess = false;
private:

    /**
     * @brief 初始化界面
     */
    void initUi();

    /**
     * @brief 初始化信号连接
     */
    void initConnection();

    /**
     * @brief 设置当前操作设备路径
     */
    void setCurDevicePath(const QString &devPath);

    /**
     * @brief 设置当前操作VG名称
     */
    void setCurVGName(const QString &vgName);

    DmTreeview *m_treeView = nullptr;

signals:

public slots:

private slots:
    /**
     * @brief 更新设备信息信号响应的槽函数
     */
    void onUpdateDeviceInfo();

    /**
     * @brief 删除分区信号响应的槽函数
     * @param deleteMessage 执行结果
     */
    void onDeletePartition(const QString &deleteMessage);

    /**
     * @brief 隐藏分区信号响应的槽函数
     * @param hideMessage 执行结果
     */
    void onHidePartition(const QString &hideMessage);

    /**
     * @brief 显示分区信号响应的槽函数
     * @param showMessage 执行结果
     */
    void onShowPartition(const QString &showMessage);

    /**
     * @brief 卸载分区信号响应的槽函数
     * @param unmountMessage 执行结果
     */
    void onUnmountPartition(const QString &unmountMessage);

    /**
     * @brief 接收新建分区表返回执行结果的槽函数
     * @param flag 执行结果
     */
    void onCreatePartitionTableMessage(const bool &flag);

    /**
     * @brief usb热插拔信号响应的槽函数
     */
    void onUpdateUsb();

    /**
     * @brief 设备树右键菜单按钮
     * @param pos 当前点击树节点位置
     */
    void treeMenu(const QPoint &pos);

    /**
     * @brief 磁盘信息按钮点击响应的槽函数
     */
    void onDiskInfoClicked();

    /**
     * @brief 磁盘健康检测按钮点击响应的槽函数
     */
    void onDiskCheckHealthClicked();

    /**
     * @brief 分区表错误检测按钮点击响应的槽函数
     */
    void onPartitionErrorCheckClicked();

    /**
     * @brief 隐藏分区按钮点击响应的槽函数
     */
    void onHidePartitionClicked();

    /**
     * @brief 显示分区按钮点击响应的槽函数
     */
    void onShowPartitionClicked();

    /**
     * @brief 删除分区按钮点击响应的槽函数
     */
    void onDeletePartitionClicked();

    /**
     * @brief 磁盘坏道检测与修复按钮点击响应的槽函数
     */
    void onDiskBadSectorsClicked();

    /**
     * @brief 磁盘新建分区表按钮点击响应的槽函数
     */
    void onCreatePartitionTableClicked();

    /**
     * @brief 删除逻辑卷组按钮点击响应的槽函数
     */
    void onDeleteVGClicked();

    /**
     * @brief 创建逻辑卷按钮点击响应的槽函数
     */
    void onCreateLVClicked();

    /**
     * @brief 删除逻辑卷按钮点击响应的槽函数
     */
    void onDeleteLVClicked();

    /**
     * @brief 删除VG信号响应的槽函数
     * @param vgMessage 执行结果
     */
    void onVGDeleteMessage(const QString &vgMessage);

    /**
     * @brief 删除LV信号响应的槽函数
     * @param lvMessage 执行结果
     */
    void onLVDeleteMessage(const QString &lvMessage);

    /**
     * @brief 创建分区/逻辑卷失败信号响应的槽函数
     * @param message 失败信息
     */
    void onCreateFailedMessage(const QString &message);

private:
    int m_flag = 0;
    int m_num = 0;
    int m_addItem = 0;
    QString m_devicePath;
    int m_deviceNum;
    DiskInfoData m_curDiskInfoData;
    QString m_curChooseDevicePath;
    QString m_curChooseVGName;
    bool m_vgIsShow = false;
    //    DMDbusHandler *m_handler;
    //    DmDiskinfoBox *m_box = nullptr;
    //    DmDiskinfoBox *m_childbox = nullptr;
};

#endif // DEVICELISTWIDGET_H
