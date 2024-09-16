// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef DISKMANAGERSERVICE_H
#define DISKMANAGERSERVICE_H
#include "diskoperation/partedcore.h"
#include "diskoperation/thread.h"
//#include "PolicyKitHelper.h"

#include <QObject>
#include <QDBusContext>
#include <QScopedPointer>

namespace DiskManager {

/**
 * @class DiskManagerService
 * @brief 磁盘分区操作封装类
 */

class DiskManagerService : public QObject
    , protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.diskmanager")
public:
    explicit DiskManagerService(const QString &frontEndDBusName, QObject *parent = nullptr);

Q_SIGNALS:
    /**
     * @brief 创建分区表信号
     * @param flag:true成功false失败
     */
    Q_SCRIPTABLE void createTableMessage(const bool &flag);

//    /**
//     * @brief root登录验证信号
//     * @param deleteMessage：登录信息
//     */
//    Q_SCRIPTABLE void rootLogin(const QString &loginMessage);

    /**
     * @brief 信息传递信号
     * @param msg：要传递的信息
     */
    Q_SCRIPTABLE void MessageReport(const QString &msg);

    /**
     * @brief 刷新设备信息信号
     * @param infomap：所有设备分区信息
     * @param lvmInfo：lvm信息
     */
    Q_SCRIPTABLE void updateDeviceInfo(const DeviceInfoMap &infomap, const LVMInfo &lvmInfo);


    /**
     * @brief 刷新加密设备信息信号
     * @param infomap：所有加密设备分区信息
     */
    Q_SCRIPTABLE void updateLUKSInfo(const LUKSMap &infomap);

    /**
     * @brief 卸载状态信号
     * @param umountMessage:卸载信息
     */
    Q_SCRIPTABLE void unmountPartition(const QString &unmountMessage);

    /**
     * @brief 删除分区信号
     * @param deleteMessage：删除信息
     */
    Q_SCRIPTABLE void deletePartition(const QString &deleteMessage);

    /**
     * @brief 隐藏分区信号
     * @param deleteMessage：隐藏信息
     */
    Q_SCRIPTABLE void hidePartitionInfo(const QString &hideMessage);

    /**
     * @brief 显示分区信号
     * @param deleteMessage：显示信息
     */
    Q_SCRIPTABLE void showPartitionInfo(const QString &showMessage);

    /**
     * @brief USB刷新信号
     */
    Q_SCRIPTABLE void usbUpdated();

    /**
     * @brief 坏道检测检测信息信号
     * @param cylinderNumber：检测柱面号
     * @param cylinderTimeConsuming：柱面耗时
     * @param cylinderStatus：柱面状态
     * @param cylinderErrorInfo：柱面错误信息
     */
    Q_SCRIPTABLE void checkBadBlocksCountInfo(const QString &cylinderNumber, const QString &cylinderTimeConsuming, const QString &cylinderStatus, const QString &cylinderErrorInfo);

    /**
     * @brief 坏道检测完成信号
     */
    Q_SCRIPTABLE void checkBadBlocksFinished();

    /**
     * @brief 坏道修复完成信号
     */
    Q_SCRIPTABLE void fixBadBlocksFinished();

    /**
     * @brief 坏道修复信息信号
     * @param cylinderNumber：检测柱面号
     * @param cylinderStatus：柱面状态
     */
    Q_SCRIPTABLE void fixBadBlocksInfo(const QString &cylinderNumber, const QString &cylinderStatus, const QString &cylinderTimeConsuming);

    /**
     * @brief 清除信号
     * @param clearMessage：清除信息 enum:errcode:devicePath   示例: DISK_ERROR:1:/dev/sda1
     *                                                             LVMError:1:/dev/vg01/lv01
     *                                                             CRYPTError:1:/dev/mapper/sda1-aesE
     */
    Q_SCRIPTABLE void clearMessage(const QString &clearMessage);

    /**
     * @brief 创建vg
     * @param vgMessage:创建结果
     */
    Q_SCRIPTABLE void vgCreateMessage(const QString &vgMessage);

    /**
     * @brief pv删除
     * @param pvMessage:删除结果
     */
    Q_SCRIPTABLE void pvDeleteMessage(const QString &vgMessage);

    /**
     * @brief vg删除
     * @param vgMessage:删除结果
     */
    Q_SCRIPTABLE void vgDeleteMessage(const QString &vgMessage);


    /**
     * @brief lg删除
     * @param lvMessage:删除结果
     */
    Q_SCRIPTABLE void lvDeleteMessage(const QString &lvMessage);

    /**
     * @brief 解密消息
     * @param luks:解密消息
     */
    Q_SCRIPTABLE void deCryptMessage(const LUKS_INFO &luks);

    /**
     * @brief 创建失败(lv/分区 (加密/未加密))
     * @param message:失败信息  enum:errcode:devicePath   示例: DISK_ERROR:1:/dev/sda1
     *                                                        LVMError:1:/dev/vg01/lv01
     *                                                        CRYPTError:1:/dev/mapper/sda1-aesE
     *
     */
    Q_SCRIPTABLE void createFailedMessage(const QString &message);

public Q_SLOTS:
    /**
     * @brief 退出服务
     */
    Q_SCRIPTABLE void Quit();

//    /**
//    *@brief 启动服务
//    */
//    Q_SCRIPTABLE void Start(qint64 applicationPid);
    /**
    *@brief 启动服务
    */
    Q_SCRIPTABLE void Start();


    /**
     * @brief 获取设备信息
     * @return 返回设备信息表
     */
    Q_SCRIPTABLE DeviceInfo getDeviceinfo();

    /**
     * @brief 获取全部设备信息
     */
    Q_SCRIPTABLE void getalldevice();

    /**
     * @brief 获取页面选择分区信息
     * @param 分区信息
     */
    Q_SCRIPTABLE void setCurSelect(const PartitionInfo &info);

    /**
     * @brief 卸载
     */
    Q_SCRIPTABLE bool unmount();

    /**
     * @brief 挂载
     * @param 挂载点路径
     * @return true成功false失败
     */
    Q_SCRIPTABLE bool mount(const QString &mountpath);


    /**
     * @brief 加密磁盘解密
     * @param luks  加密磁盘属性
     * @return true成功false失败
     */
    Q_SCRIPTABLE bool deCrypt(const LUKS_INFO&luks);


    /**
     * @brief 加密磁盘挂载
     * @param luks  加密磁盘属性
     * @return true成功false失败
     */
    Q_SCRIPTABLE bool cryptMount(const LUKS_INFO&luks);

    /**
     * @brief 加密磁盘卸载
     * @param luks  加密磁盘属性
     * @return true成功false失败
     */
    Q_SCRIPTABLE bool cryptUmount(const LUKS_INFO&luks);

    /**
     * @brief 获取全部文件系统格式支持
     * @return 返回支持文件系统格式列表
     */
    Q_SCRIPTABLE QStringList getallsupportfs();

    /**
     * @brief 格式化分区
     * @param fstype：文件系统类型
     * @param name：分区别名
     * @return true成功false失败
     */
    Q_SCRIPTABLE bool format(const QString &fstype, const QString &name = QString());

    /**
     * @brief 擦除磁盘
     * @param fstype：格式化格式
     * @param path：分区别名
     * @param name: 劵标名
     * @param diskType : 0为分区，1为磁盘，2为空闲
     * @param clearType: 擦除标准， 0为快速，1为安全（NIST），2为DoD标准， 3为古德曼标准
     * @return true成功false失败
     */
    Q_SCRIPTABLE bool clear(const WipeAction&wipe);


    /**
     * @brief 扩容分区
     * @param info：扩容分区信息
     * @return true成功false失败
     */
    Q_SCRIPTABLE bool resize(const PartitionInfo &info);

    /**
     * @brief 创建分区
     * @param infovec：创建分区信息列表
     * @return true成功false失败
     */
    Q_SCRIPTABLE bool create(const PartitionVec &infovec);

    /**
     * @brief 获取硬盘硬件信息
     * @param devicepath：设备信息路径
     * @return 返回硬盘信息表
     */
    Q_SCRIPTABLE HardDiskInfo onGetDeviceHardInfo(const QString &devicepath);

    /**
     * @brief 获取硬盘健康状态
     * @param devicepath：设备信息路径
     * @return 返回硬盘健康状态
     */
    Q_SCRIPTABLE QString onGetDeviceHardStatus(const QString &devicepath);

    /**
     * @brief 获取硬盘健康信息
     * @param devicepath：设备信息路径
     * @return 返回硬盘健康信息
     */
    Q_SCRIPTABLE HardDiskStatusInfoList onGetDeviceHardStatusInfo(const QString &devicepath);

    /**
     * @brief 删除分区
     * @param devicepath：设备信息路径
     * @param parttitionpath：分区路径
     * @return true成功false失败
     */
    Q_SCRIPTABLE bool onDeletePartition();

    /**
     * @brief 隐藏分区
     * @param devicepath：设备信息路径
     * @param parttitionpath：分区路径
     * @return true成功false失败
     */
    Q_SCRIPTABLE bool onHidePartition();

    /**
     * @brief 显示分区
     * @param devicepath：设备信息路径
     * @param parttitionpath：分区路径
     * @return true成功false失败
     */
    Q_SCRIPTABLE bool onShowPartition();

    /**
     * @brief 分区表错误检测
     * @param devicepath：设备信息路径
     * @return true错误false正常
     */
    Q_SCRIPTABLE bool onDetectionPartitionTableError(const QString &devicePath);

    /**
     * @brief 创建分区表
     * @param devicepath：设备信息路径
     * @param length:设备大小
     * @param sectorSize:扇区大小
     * @param diskLabel:分区表格式
     * @return true错误false正常
     */
    Q_SCRIPTABLE bool onCreatePartitionTable(const QString &devicePath, const QString &length, const QString &sectorSize, const QString &diskLabel);

    /**
     * @brief 坏道检测（检测次数）
     * @param devicePath：设备信息路径
     * @param blockStart：开始柱面
     * @param blockEnd：结束柱面
     * @param checkConut：检测次数
     * @param checkSize：检测柱面大小
     * @param flag：暂停，检测，继续标志
     * @return true错误false正常
     */
    Q_SCRIPTABLE bool onCheckBadBlocksCount(const QString &devicePath, int blockStart, int blockEnd, int checkConut, int checkSize, int flag);

    /**
     * @brief 坏道检测（检测时间）
     * @param devicePath：设备信息路径
     * @param blockStart：开始柱面
     * @param blockEnd：结束柱面
     * @param checkTime: 检测超时时间
     * @param checkSize：检测柱面大小
     * @param flag：暂停，检测，继续标志
     * @return true错误false正常
     */
    Q_SCRIPTABLE bool onCheckBadBlocksTime(const QString &devicePath, int blockStart, int blockEnd, const QString &checkTime, int checkSize, int flag);

    /**
     * @brief 坏道修复
     * @param devicePath：设备信息路径
     * @param badBlocksList：坏道柱面集合
     * @param checkSize：检测柱面大小
     * @param flag：暂停，检测，继续标志
     * @return true错误false正常
     */
    Q_SCRIPTABLE bool onFixBadBlocks(const QString &devicePath, QStringList badBlocksList, int checkSize, int flag);



    /**
     * @brief 创建vg
     * @param vgName:待创建vg名称
     * @param devList: pv设备集合
     * @param size:vg总大小
     * @return true 成功 false 失败
     */
    Q_SCRIPTABLE bool onCreateVG(QString vgName, QList<PVData>devList, long long size);

    /**
     * @brief 创建lv
     * @param vgName:vg名称
     * @param lvList: 待创建lv列表
     * @return true 成功 false 失败
     */
    Q_SCRIPTABLE bool onCreateLV(QString vgName, QList<LVAction>lvList);

    /**
     * @brief 删除vg
     * @param vglist: 待删除vg列表
     * @return true 成功 false 失败
     */
    Q_SCRIPTABLE bool onDeleteVG(QStringList vglist);

    /**
     * @brief 删除lv
     * @param lvlist: 待删除lv列表
     * @return true 成功 false 失败
     */
    Q_SCRIPTABLE bool onDeleteLV(QStringList lvlist);

    /**
     * @brief vg空间调整
     * @param vgName:vg名称
     * @param devList: pv设备集合
     * @param size:调整后vg总大小
     * @return true 成功 false 失败
     */
    Q_SCRIPTABLE bool onResizeVG(QString vgName, QList<PVData>devList, long long size);

    /**
     * @brief lv空间调整
     * @param lvAction:lv操作结构体
     * @return true 成功 false 失败
     */
    Q_SCRIPTABLE bool onResizeLV(LVAction lvAction);


    /**
     * @brief lv挂载
     * @param lvAction:lv操作结构体
     * @return true 成功 false 失败
     */
    Q_SCRIPTABLE bool onMountLV(LVAction lvAction);

    /**
     * @brief lv卸载
     * @param lvAction:lv操作结构体
     * @return true 成功 false 失败
     */
    Q_SCRIPTABLE bool onUmountLV(LVAction lvAction);

    /**
     * @brief lv擦除
     * @param lvAction:lv操作结构体
     * @return true 成功 false 失败
     */
    Q_SCRIPTABLE bool onClearLV(LVAction lvAction);

    /**
     * @brief pv删除
     * @param devList: 待删除pv设备集合
     * @return true 成功 false 失败
     */
    Q_SCRIPTABLE bool onDeletePVList(QList<PVData>devList);

    /**
     * @brief USB插入
     */
    Q_SCRIPTABLE void updateUsb();

    /**
     * @brief USB拔出
     */
    Q_SCRIPTABLE void updateUsbRemove();

    /**
     * @brief 刷新按钮
     */
    Q_SCRIPTABLE void refreshFunc();

    /**
     * @brief 测试
     * @param 无
     */
    Q_SCRIPTABLE int test();


private:
    /**
     * @brief 初始化信号和槽函数
     * @param 无
     */
    void initConnection();
    bool checkAuthorization(void);

signals:
    void getAllDeviceInfomation();

private slots:
    void onGetAllDeviceInfomation();

private:
    PartedCore *m_partedcore;  //磁盘操作类对象
    QString m_frontEndDBusName;
};

} // namespace DiskManager
#endif // DISKMANAGERSERVICE_H
