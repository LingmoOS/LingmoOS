// Copyright (C) 2022 ~ 2022 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef LVMOPERATOR_H
#define LVMOPERATOR_H

#include "lvmstruct.h"
#include "deviceinfo.h"
#include "supportedfilesystems.h"

class LVMOperator
{
public:
    LVMOperator();

    /**
     * @brief 更新lvm信息
     * @param lvmInfo:lvm数据结构体
     * @return true 成功 false 失败
     */
    static bool updateLVMInfo(LVMInfo &lvmInfo);

    /**
     * @brief 更新lvm信息 并且获取设备上lvmData
     * @param dev: 设备集合
     * @param lvmInfo:lvm数据结构体
     * @return true 成功 false 失败
     */
    static bool getDeviceDataAndLVMInfo(DeviceInfoMap &dev, LVMInfo &lvmInfo);

    /**
     * @brief 清空lvm结构体数据
     * @param lvmInfo:lvm数据结构体
     */
    static void resetLVMInfo(LVMInfo &lvmInfo);

    /**
     * @brief 打印lvm错误信息
     * @param error:lvm数据结构体
     */
    static void printError(const LVMError &error);


    /**
     * @brief 打印lvm信息
     * @param lvmInfo:lvm数据结构体
     */
    static void printLVMInfo(const LVMInfo &lvmInfo);

    /**
     * @brief 打印设备上lvm信息
     * @param dev: 设备集合
     */
    static void printDeviceLVMData(const DeviceInfoMap &dev);

    /**
     * @brief 创建vg
     * @param lvmInfo: lvm数据集合
     * @param vgName:待创建vg名称
     * @param devList: pv设备集合
     * @param size:vg总大小
     * @return true 成功 false 失败
     */
    static bool createVG(LVMInfo &lvmInfo, QString vgName, QList<PVData>devList, long long size);

    /**
     * @brief 删除vg
     * @param lvmInfo: lvm数据集合
     * @param vglist: 待删除vg列表
     * @return true 成功 false 失败
     */
    static bool deleteVG(LVMInfo &lvmInfo, QStringList vglist);

    /**
     * @brief vg空间调整
     * @param lvmInfo: lvm数据集合
     * @param vgName:vg名称
     * @param devList: pv设备集合
     * @param size:调整后vg总大小
     * @return true 成功 false 失败
     */
    static bool resizeVG(LVMInfo &lvmInfo, QString vgName, QList<PVData>devList, long long size);

    /**
     * @brief 创建lv
     * @param lvmInfo: lvm数据集合
     * @param vgName:vg名称
     * @param lvList: 待创建lv列表
     * @return true 成功 false 失败
     */
    static bool createLV(LVMInfo &lvmInfo, QString vgName, QList<LVAction>lvList);

    /**
     * @brief 删除lv
     * @param lvmInfo: lvm数据集合
     * @param lvlist: 待删除lv列表
     * @return true 成功 false 失败
     */
    static bool lvRemove(LVMInfo &lvmInfo, QStringList lvlist);

    /**
     * @brief lv空间调整
     * @param lvmInfo: lvm数据集合
     * @param lvPath:lv路径
     * @param size: 调整后lv总大小
     * @return true 成功 false 失败
     */
    static bool resizeLV(LVMInfo &lvmInfo, LVAction &lvAction, LVInfo &info);

    /**
     * @brief pv删除
     * @param lvmInfo: lvm数据集合
     * @param devList: 待删除pv设备集合
     * @return true 成功 false 失败
     */
    static bool deletePVList(LVMInfo &lvmInfo,  QList<PVData>devList);

private:

    /**
     * @brief 获取设备上lvmData
     * @param dev: 设备集合
     * @param lvmInfo:lvm数据结构体
     * @return true 成功 false 失败
     */
    static bool getDeviceLVMData(DeviceInfoMap &dev, const LVMInfo &lvmInfo);

    /**
     * @brief 更新lvm PV信息
     * @param lvmInfo:lvm数据结构体
     * @return true 成功 false 失败
     */
    static bool updatePVInfo(LVMInfo &lvmInfo);

    /**
     * @brief 更新lvm VG信息
     * @param lvmInfo:lvm数据结构体
     * @return true 成功 false 失败
     */
    static bool updateVGInfo(LVMInfo &lvmInfo);

    /**
     * @brief 更新lvm LV信息
     * @param lvmInfo:lvm数据结构体
     * @param info:vg数据结构体
     * @return true 成功 false 失败
     */
    static bool updateLVInfo(LVMInfo &lvmInfo, VGInfo &info);

    /**
     * @brief 打印设备上VG信息
     * @param info: VG结构体
     */
    static void printVGInfo(const VGInfo &info);

    /**
     * @brief 打印设备上LV信息
     * @param info: LV结构体
     */
    static void printLVInfo(const LVInfo &info);

    /**
     * @brief 打印设备上PV信息
     * @param info: PV结构体
     */
    static void printPVInfo(const PVInfo &info);

    /**
     * @brief 初始化系统命令支持
     * @return true 成功 false 失败
     */
    static bool initSuport();

    /**
     * @brief 检查并尝试修复系统上的VG
     * @return true 成功 false 失败
     */
    static bool checkVG();

    /**
     * @brief 创建vg
     * @param vgName:vg名称
     * @param pv: pv路径
     * @return true 成功 false 失败
     */
    static int vgCreate(const QString &vgName, const QStringList &pvList);

    /**
     * @brief vg重命名
     * @param uuid:vgUUid
     * @param newName:vg新名称
     * @return true 成功 false 失败
     */
    static bool vgRename(const QString &uuid, const QString &newName);

    /**
     * @brief vg增加pv
     * @param vgName:vg名称
     * @param pv: pv路径
     * @return true 成功 false 失败
     */
    static bool vgExtend(const QString &vgName, const QString &pvPath);

    /**
     * @brief vg减少pv
     * @param vgName:vg名称
     * @param pv: pv路径
     * @return true 成功 false 失败
     */
    static bool vgReduce(const QString &vgName, const QString &pvPath);

    /**
     * @brief 删除vg
     * @param vgInfo:vg数据结构体
     * @return true 成功 false 失败
     */
    static int vgRemove(const VGInfo &vgInfo);

    /**
     * @brief 创建pv
     * @param devPath:pv路径
     * @return true 成功 false 失败
     */
    static bool pvCreate(const QString &devPath);

    /**
     * @brief pv移动
     * @param pvPath: pv设备路径
     * @return true 成功 false 失败
     */
    static bool pvMove(const QString &pvPath, const QString &dest = "");

    /**
     * @brief 删除pv
     * @param devPath:pv路径
     * @return true 成功 false 失败
     */
    static bool pvRemove(const QString &devPath);

    /**
     * @brief 创建lv
     * @param vgName:vg名称
     * @param lvName:lv名称
     * @param size: lv大小
     * @return true 成功 false 失败
     */
    static bool lvCreate(const QString &vgName,const QString &lvName,const long long &size);

    /**
     * @brief 删除lv
     * @param lvPath:lv路径
     * @return true 成功 false 失败
     */
    static int lvRemove(const QString &lvPath);

    /**
     * @brief 调整lv
     * @param devPath:lv设备路径
     * @param act:lv 操作结构体
     * @return true 成功 false 失败
     */
    static bool resizeLV(const QString &devPath, const LVAction &act);

    /**
     * @brief 获取系统命令支持
     * @param cmd: 命令
     * @return NONE 不支持 EXTERNAL 支持外部命令
     */
    static inline LVM_CMD_Support::LVM_Support getExecSupport(const QString &cmd)
    {
        return Utils::findProgramInPath(cmd).isEmpty() ? LVM_CMD_Support::NONE : LVM_CMD_Support::EXTERNAL;
    }

    /**
     * @brief 获取系统命令支持
     * @param list: 字符串列表
     * @param i:index
     * @return list指定index的字符串 返回空代表index无效
     */
    static inline QString getStringListItem(const QStringList &list, const int &i)
    {
        return (i >= 0 && list.size() > i) ? list[i] : QString();
    }

    /**
     * @brief 设置lvm错误
     * @param lvmInfo:lvm数据
     * @param err:错误枚举
     * @return true 成功 false 失败
     */
    static inline bool setLVMErr(LVMInfo &lvmInfo, const LVMError &err)
    {
        lvmInfo.m_lvmErr = err;
        m_lvmErr = err;
        printError(err);
        return lvmInfo.m_lvmErr == LVMError::LVM_ERR_NORMAL;
    }

public:
    static LVM_CMD_Support m_lvmSupport;    //lvm系统支持集合
    static bool m_initSupport;              //初始化lvm系统支持标志位
    static DeviceInfoMap  m_devInfo;        //磁盘属性集合
    static DiskManager::SupportedFileSystems m_supportFs; //文件系统支持集合
    static LVMError m_lvmErr;               //lvm

};

#endif // LVMOPERATOR_H
