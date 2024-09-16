// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef DMDBUSHANDLER_H
#define DMDBUSHANDLER_H

#include "dmdbusinterface.h"

#include <QObject>
#include <QDebug>
#include <QDBusConnection>

/**
 * @class DMDbusHandler
 * @brief 数据接口类
 */

class DMDbusHandler : public QObject
{
    Q_OBJECT
public:
    static DMDbusHandler *instance(QObject *parent = nullptr);
    ~DMDbusHandler();

    enum DeviceType {
        DISK = 0,
        PARTITION,
        VOLUMEGROUP,
        LOGICALVOLUME,
        OTHER
    };

    /**
     * @brief 开启服务
     */
    void startService();

    /**
     * @brief 关闭服务
     */
    void Quit();

    /**
     * @brief 刷新
     */
    void refresh();

    /**
     * @brief 获取登录结果
     * @return 返回登录认证结果
     */
    QString getRootLoginResult();

    /**
     * @brief 给服务发获取所有设备信息的消息
     */
    void getDeviceInfo();

    /**
     * @brief 获取所有设备信息
     */
    DeviceInfoMap &probDeviceInfo();

    /**
     * @brief 根据设备路径获取该设备的所有分区信息
     */
    PartitionVec getCurDevicePartitionArr();

    /**
     * @brief 获取当前分区信息
     */
    const PartitionInfo &getCurPartititonInfo();

    /**
     * @brief 根据设备路径获取该设备的所有信息
     */
    const DeviceInfo &getCurDeviceInfo();

    /**
     * @brief 根据设备路径获取该设备的扇区长度
     */
    const Sector &getCurDeviceInfoLength();

    /**
     * @brief 挂载分区
     * @param mountPath 挂载路径
     */
    void mount(const QString &mountPath);

    /**
     * @brief 卸载分区
     */
    void unmount();

    /**
     * @brief 获取所有文件系统支持
     */
    QStringList getAllSupportFileSystem();

    /**
     * @brief 格式化分区
     * @param fstype 分区格式
     * @param name 分区名称
     */
    void format(const QString &fstype, const QString &name = QString());

    /**
     * @brief 空间调整
     * @param info 当前分区信息
     */
    void resize(const PartitionInfo &info);

    /**
     * @brief 创建分区
     * @param infovec 将要创建的分区列表
     */
    void create(const PartitionVec &infovec);

    /**
     * @brief 获取磁盘基本信息
     * @param devicePath 磁盘路径
     * @return 返回磁盘信息
     */
    HardDiskInfo getHardDiskInfo(const QString &devicePath);

    /**
     * @brief 获取磁盘健康状态
     * @param devicePath 磁盘路径
     * @return 返回磁盘健康状态
     */
    QString getDeviceHardStatus(const QString &devicePath);

    /**
     * @brief 获取磁盘健康检测信息
     * @param devicePath 磁盘路径
     * @return 返回磁盘健康检测信息
     */
    HardDiskStatusInfoList getDeviceHardStatusInfo(const QString &devicePath);

    /**
     * @brief 删除分区
     */
    void deletePartition();

    /**
     * @brief 隐藏分区
     */
    void hidePartition();

    /**
     * @brief 显示分区
     */
    void unhidePartition();

    /**
     * @brief 分区表错误检测
     * @param devicePath 磁盘路径
     * @return 返回true检测错误，0检测正常
     */
    bool detectionPartitionTableError(const QString &devicePath);

    /**
     * @brief 获取磁盘是否存在空闲空间
     * @return 返回所有磁盘是否存在空闲空间信息
     */
    QMap<QString, QString> getIsExistUnallocated();

    /**
     * @brief 获取所有的磁盘名称
     * @return 返回磁盘名称列表
     */
    QStringList getDeviceNameList();

    /**
     * @brief 坏道检测
     * @param devicePath 磁盘路径
     * @param blockStart 检测开始
     * @param blockEnd 检测结束
     * @param checkConut 检测次数
     * @param checkSize 检测柱面大小
     * @param flag：检测状态(检测: 1，停止: 2，继续: 3)
     */
    void checkBadSectors(const QString &devicePath, int blockStart, int blockEnd, int checkNumber, int checkSize, int flag);

    /**
     * @brief 坏道修复
     * @param devicePath 磁盘路径
     * @param badBlocksList 坏道列表
     * @param repairSize 修复柱面大小
     * @param flag：修复状态(修复: 1，停止: 2，继续: 3)
     */
    void repairBadBlocks(const QString &devicePath, QStringList badBlocksList, int repairSize, int flag);

    /**
     * @brief Root认证消息
     * @param loginMessage 登录消息（1：成功，0：失败）
     */
    void onRootLogin(const QString &loginMessage);

    /**
     * @brief 创建分区表
     * @param devicepath：设备信息路径
     * @param length:设备大小
     * @param sectorSize:扇区大小
     * @param diskLabel:分区表格式
     * @param curType 当前新建分区表的类型（create：创建，replace：替换）
     */
    void createPartitionTable(const QString &devicePath, const QString &length, const QString &sectorSize, const QString &diskLabel, const QString &curType);

    /**
     * @brief 获取当前新建分区表的类型
     * @return 返回当前新建分区表的类型（create：创建，replace：替换）
     */
    QString getCurCreatePartitionTableType();

    /**
     * @brief 获取当前选择节点的类型
     * @return 返回当前选择节点的类型（0：磁盘，1：分区）
     */
    int getCurLevel();

    /**
     * @brief 获取当前选择磁盘路径
     * @return 返回磁盘路径
     */
    QString getCurDevicePath();

    /**
    * @brief 擦除
    * @param wipe 擦除信息
    */
    void clear(const WipeAction &wipe);

    /**
     * @brief 获取所有逻辑卷信息
     */
    const LVMInfo &probLVMInfo() const;

    /**
     * @brief 获取当前逻辑卷组信息
     */
    const VGInfo &getCurVGInfo();

    /**
     * @brief 获取当前逻辑卷信息
     */
    const LVInfo &getCurLVInfo();

    /**
     * @brief 获取所有的逻辑卷组名称
     * @return 返回逻辑卷组名称列表
     */
    QStringList getVGNameList();

    /**
     * @brief 获取当前选择逻辑卷组名称
     * @return 返回逻辑卷组名称
     */
    QString getCurVGName();

    /**
     * @brief 获取磁盘是否全部加入VG
     * @return 返回磁盘是否全部加入VG
     */
    QMap<QString, QString> getIsJoinAllVG();

    /**
     * @brief 判断逻辑卷组下是否存在已被挂载的逻辑卷
     * @return 存在返回true，否则返回false
     */
    bool isExistMountLV(const VGInfo &info);

    /**
     * @brief 判断LV是否被挂载
     * @param lvInfo lv信息
     * @return 挂载返回true，否则返回false
     */
    bool lvIsMount(const LVInfo &lvInfo);

    /**
     * @brief 判断当前磁盘是否存在已被挂载分区
     * @return 存在返回true，否则返回false
     */
    bool isExistMountPartition(const DeviceInfo &info);

    /**
     * @brief 判断分区是否被挂载
     * @param info 分区信息
     * @return 挂载返回true，否则返回false
     */
    bool partitionISMount(const PartitionInfo &info);

    /**
     * @brief 创建vg
     * @param vgName:待创建vg名称
     * @param devList: pv设备集合
     * @param size:vg总大小
     */
    void createVG(const QString &vgName, const QList<PVData> &devList, const long long &size);

    /**
     * @brief 创建lv
     * @param vgName:vg名称
     * @param lvList: 待创建lv列表
     */
    void createLV(const QString &vgName, const QList<LVAction> &lvList);

    /**
     * @brief 删除lv
     * @param lvlist: 待删除lv列表
     */
    void deleteLV(const QStringList &lvlist);

    /**
     * @brief 删除vg
     * @param vglist: 待删除vg列表
     */
    void deleteVG(const QStringList &vglist);

    /**
     * @brief vg空间调整
     * @param vgName:vg名称
     * @param devList: pv设备集合
     * @param size:调整后vg总大小
     */
    void resizeVG(const QString &vgName, const QList<PVData> &devList, const long long &size);

    /**
     * @brief lv空间调整
     * @param act:操作lv结构体
     */
    void resizeLV(LVAction act);

    /**
     * @brief lv挂载
     * @param act:操作lv结构体
     */
    void onMountLV(LVAction act);

    /**
     * @brief lv卸载
     * @param act:操作lv结构体
     */
    void onUmountLV(LVAction act);

    /**
     * @brief lv擦除
     * @param act:操作lv结构体
     */
    void onClearLV(LVAction act);

    /**
     * @brief pv删除
     * @param devList: 待删除pv设备集合
     */
    void onDeletePVList(QList<PVData>devList);

    /**
     * @brief 获取加密格式
     * @param formateList: 当前支持的文件系统
     * @return 加密格式文件系统
     */
    QStringList getEncryptionFormate(const QStringList &formateList);

    /**
     * @brief 获取所有加密盘信息
     */
    const LUKSMap &probLUKSInfo() const;

    /**
     * @brief 更新加密磁盘数据
     * @param devPath 设备路径
     * @param luks 加密磁盘属性
     */
    void updateLUKSInfo(const QString &devPath, const LUKS_INFO &luks);

    /**
     * @brief 获取是否为系统盘
     * @param devName: 设备名称
     * @return true是,false不是
     */
    bool getIsSystemDisk(const QString &devName);

    /**
     * @brief 加密磁盘解密
     * @param luks 加密磁盘属性
     */
    void deCrypt(const LUKS_INFO &luks);

    /**
     * @brief 加密磁盘挂载
     * @param luks 加密磁盘属性
     * @param devName 设备名称
     */
    void cryptMount(const LUKS_INFO &luks, const QString &devName);

    /**
     * @brief 加密磁盘卸载
     * @param luks 加密磁盘属性
     * @param devName 设备名称
     */
    void cryptUmount(const LUKS_INFO &luks, const QString &devName);

    /**
     * @brief 获取是否全部加密
     * @return 返回是否全部加密
     */
    QMap<QString, QString> getIsAllEncryption();

    /**
     * @brief 刷新主界面显示数据
     */
    void refreshMainWindowData();

    /**
     * @brief 获取失败提示信息
     * @param key 枚举信息
     * @param value 枚举值
     * @param devPath 设备路径
     */
    QString getFailedMessage(const QString &key, const int &value, const QString &devPath);

    /**
     * @brief 获取加密文件系统类型
     * @param luksInfo 加密信息
     * @return 加密文件系统类型
     */
    QString getEncryptionFsType(const LUKS_INFO &luksInfo);

private:
    explicit DMDbusHandler(QObject *parent = nullptr);

    /**
     * @brief 初始化连接
     */
    void initConnection();

signals:
    void showSpinerWindow(bool, const QString &title = "");
    void updateDeviceInfo();
    void curSelectChanged();
    void deletePartitionMessage(const QString &deleteMessage);
    void hidePartitionMessage(const QString &hideMessage);
    void showPartitionMessage(const QString &showMessage);
    void unmountPartitionMessage(const QString &unmountMessage);
    void createPartitionTableMessage(const bool &flag);
    void updateUsb();
    void checkBadBlocksCountInfo(const QString &cylinderNumber, const QString &cylinderTimeConsuming, const QString &cylinderStatus, const QString &cylinderErrorInfo);
//    void checkBadBlocksDeviceStatusError();
    void repairBadBlocksInfo(const QString &cylinderNumber, const QString &cylinderStatus, const QString &cylinderTimeConsuming);
    void checkBadBlocksFinished();
    void fixBadBlocksFinished();
    void rootLogin();
    void wipeMessage(const QString &clearMessage);
    void vgCreateMessage(const QString &vgMessage);
    void pvDeleteMessage(const QString &pvMessage);
    void vgDeleteMessage(const QString &vgMessage);
    void lvDeleteMessage(const QString &lvMessage);
    void deCryptMessage(const LUKS_INFO &luks);
    void createFailedMessage(const QString &message);

public slots:
    /**
     * @brief 树结构当前选择节点信号响应的槽函数
     * @param devicePath 磁盘路径
     * @param partitionPath 分区路径
     * @param start 分区扇区开始位置
     * @param end 分区扇区结束位置
     * @param level 节点类型
     */
    void onSetCurSelect(const QString &devicePath, const QString &partitionPath, Sector start, Sector end, int level);

private slots:

    /**
     * @brief 消息报告信号响应的槽函数
     * @param msg 消息报告信息
     */
    void onMessageReport(const QString &msg);

    /**
     * @brief 数据更新信号响应的槽函数
     * @param infoMap 设备信息
     * @param lvmInfo LVM信息
     */
    void onUpdateDeviceInfo(const DeviceInfoMap &infoMap, const LVMInfo &lvmInfo);

    /**
     * @brief 加密设备数据更新信号响应的槽函数
     * @param infomap：所有加密设备分区信息
     */
    void onUpdateLUKSInfo(const LUKSMap &infomap);

    /**
     * @brief 接收卸载分区返回执行结果的槽函数
     * @param unmountMessage 执行结果
     */
    void onUnmountPartition(const QString &unmountMessage);

    /**
     * @brief 接收删除分区返回执行结果的槽函数
     * @param deleteMessage 执行结果
     */
    void onDeletePartition(const QString &deleteMessage);

    /**
     * @brief 接收隐藏分区返回执行结果的槽函数
     * @param hideMessage 执行结果
     */
    void onHidePartition(const QString &hideMessage);

    /**
     * @brief 接收显示分区返回执行结果的槽函数
     * @param showMessage 执行结果
     */
    void onShowPartition(const QString &showMessage);

    /**
     * @brief 接收新建分区表返回执行结果的槽函数
     * @param flag 执行结果
     */
    void onCreatePartitionTable(const bool &flag);

    /**
     * @brief 接收USB插拔信号的槽函数
     */
    void onUpdateUsb();

private:
    DMDBusInterface *m_dbus = nullptr;
    static DMDbusHandler *m_staticHandeler;
    QString m_curDevicePath;
    QString m_curPartitionPath;
    DeviceInfoMap m_deviceMap;
    PartitionInfo m_curPartitionInfo;
    QStringList m_supportFileSystem;
    HardDiskInfo m_hardDiskInfo;
    QString m_deviceHardStatus;
    HardDiskStatusInfoList m_hardDiskStatusInfoList;
    int m_partitionHiddenFlag;
    bool m_partitionTableError;
    QMap<QString, QString> m_isExistUnallocated;
    QStringList m_deviceNameList;
    QString m_loginMessage;
    QString m_curCreateType;
    int m_curLevel = DeviceType::PARTITION;
    LVMInfo m_lvmInfo;
    QStringList m_vgNameList;
    QString m_curVGName;
    LVInfo m_curLVInfo;
    VGInfo m_curVGInfo;
    QMap<QString, QString> m_isJoinAllVG;
    LUKSMap m_curLUKSInfoMap;
    CRYPT_CIPHER_Support m_cryptSupport;
    QMap<QString, QString> m_isAllEncryption;
};

#endif // DMDBUSHANDLER_H
