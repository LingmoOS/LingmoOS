// Copyright (C) 2022 ~ 2022 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "lvmoperator.h"
#include "utils.h"
#include "mountinfo.h"
#include "../fsinfo.h"

#include <QStringList>
#include <QDebug>
#include <set>

using namespace DiskManager;
using std::set;

LVM_CMD_Support LVMOperator::m_lvmSupport;
bool LVMOperator::m_initSupport = false;
DeviceInfoMap LVMOperator::m_devInfo;
SupportedFileSystems LVMOperator::m_supportFs;
LVMError LVMOperator::m_lvmErr = LVM_ERR_NORMAL;
/******************************** 初始化操作 ******************************/
LVMOperator::LVMOperator()
{
    initSuport();
}

void LVMOperator::resetLVMInfo(LVMInfo &lvmInfo)
{
    lvmInfo.m_pvInfo.clear();
    lvmInfo.m_vgInfo.clear();
    lvmInfo.m_lvmErr = LVMError::LVM_ERR_NORMAL;
}

bool LVMOperator::initSuport()
{
    if (!m_initSupport) {
        m_lvmSupport.LVM_CMD_lvchange = getExecSupport("lvchange");
        m_lvmSupport.LVM_CMD_lvconvert = getExecSupport("lvconvert");
        m_lvmSupport.LVM_CMD_lvcreate = getExecSupport("lvcreate");
        m_lvmSupport.LVM_CMD_lvdisplay = getExecSupport("lvdisplay");
        m_lvmSupport.LVM_CMD_lvextend = getExecSupport("lvextend");
        m_lvmSupport.LVM_CMD_lvreduce = getExecSupport("lvreduce");
        m_lvmSupport.LVM_CMD_lvremove = getExecSupport("lvremove");
        m_lvmSupport.LVM_CMD_lvrename = getExecSupport("lvrename");
        m_lvmSupport.LVM_CMD_lvresize = getExecSupport("lvresize");
        m_lvmSupport.LVM_CMD_lvs = getExecSupport("lvs");
        m_lvmSupport.LVM_CMD_lvscan = getExecSupport("lvscan");
        m_lvmSupport.LVM_CMD_pvchange = getExecSupport("pvchange");
        m_lvmSupport.LVM_CMD_pvck = getExecSupport("pvck");
        m_lvmSupport.LVM_CMD_pvcreate = getExecSupport("pvcreate");
        m_lvmSupport.LVM_CMD_pvdisplay = getExecSupport("pvdisplay");
        m_lvmSupport.LVM_CMD_pvmove = getExecSupport("pvremove");
        m_lvmSupport.LVM_CMD_pvremove = getExecSupport("pvremove");
        m_lvmSupport.LVM_CMD_pvresize = getExecSupport("pvresize");
        m_lvmSupport.LVM_CMD_pvs = getExecSupport("pvs");
        m_lvmSupport.LVM_CMD_pvscan = getExecSupport("pvscan");
        m_lvmSupport.LVM_CMD_vgcfgbackup = getExecSupport("vgcfgbackup");
        m_lvmSupport.LVM_CMD_vgcfgrestore = getExecSupport("vgcfgbackup");
        m_lvmSupport.LVM_CMD_vgchange = getExecSupport("vgcfgrestore");
        m_lvmSupport.LVM_CMD_vgck = getExecSupport("vgck");
        m_lvmSupport.LVM_CMD_vgconvert = getExecSupport("vgconvert");
        m_lvmSupport.LVM_CMD_vgcreate = getExecSupport("vgcreate");
        m_lvmSupport.LVM_CMD_vgdisplay = getExecSupport("vgdisplay");
        m_lvmSupport.LVM_CMD_vgexport = getExecSupport("vgexport");
        m_lvmSupport.LVM_CMD_vgextend = getExecSupport("vgextend");
        m_lvmSupport.LVM_CMD_vgimport = getExecSupport("vgimport");
        m_lvmSupport.LVM_CMD_vgimportclone = getExecSupport("vgimportclone");
        m_lvmSupport.LVM_CMD_vgmerge = getExecSupport("vgmerge");
        m_lvmSupport.LVM_CMD_vgmknodes = getExecSupport("vgmknodes");
        m_lvmSupport.LVM_CMD_vgreduce = getExecSupport("vgreduce");
        m_lvmSupport.LVM_CMD_vgremove = getExecSupport("vgremove");
        m_lvmSupport.LVM_CMD_vgrename = getExecSupport("vgrename");
        m_lvmSupport.LVM_CMD_vgs = getExecSupport("vgs");
        m_lvmSupport.LVM_CMD_vgscan = getExecSupport("vgscan");
        m_lvmSupport.LVM_CMD_vgsplit = getExecSupport("vgsplit");
        m_supportFs.findSupportedFilesystems();
        m_initSupport = true;
    }
    return true;
}

/******************************** 更新DATA ******************************/
bool LVMOperator::getDeviceLVMData(DeviceInfoMap &dev, const LVMInfo &lvmInfo)
{
    auto pv2vg_func = [ = ](const PVInfo & pv) {
        VGData vg;
        vg.m_vgName = pv.m_vgName;
        if (vg.m_vgName.isEmpty()) {
            vg.m_vgName = pv.m_pvPath;
        }
        vg.m_vgSize = pv.m_pvSize;
        vg.m_vgByteSize = pv.m_pvByteTotalSize;
        vg.m_vgUuid = pv.m_vgUuid;
        for (QMap<QString, QVector<LV_PV_Ranges>>::ConstIterator lvIt = pv.m_lvRangesList.begin(); lvIt != pv.m_lvRangesList.end(); ++lvIt) {
            if (lvIt->size() <= 0) {
                continue;
            }

            LVData lvData;
            lvData.m_lvName = lvIt->begin()->m_lvName;
            lvData.m_lvPath = lvIt.key();
            long long sum = 0;
            foreach (const LV_PV_Ranges &it, lvIt.value()) {
                sum += (it.m_end - it.m_start + 1);
            }
            lvData.m_lvByteSize = sum * pv.m_PESize;
            lvData.m_lvSize = Utils::LVMFormatSize(lvData.m_lvByteSize);
            vg.m_lvList.push_back(lvData);
        }
        return vg;
    };

    for (QMap<QString, DeviceInfo>::iterator diskIter = dev.begin(); diskIter != dev.end(); ++diskIter) {
        diskIter->m_vglist.clear();
        diskIter->m_vgFlag = LVM_FLAG_NOT_PV;
        QMap<QString, PVInfo>::ConstIterator pvIter = lvmInfo.m_pvInfo.find(diskIter->m_path);
        if (pvIter != lvmInfo.m_pvInfo.end() && QString("unrecognized") == diskIter.value().m_disktype) { //不存在分区表 磁盘加入
            diskIter->m_vgFlag = pvIter->noJoinVG() ? LVMFlag::LVM_FLAG_NOT_JOIN_VG : LVMFlag::LVM_FLAG_JOIN_VG;
            diskIter->m_vglist.push_back(pv2vg_func(pvIter.value()));
            continue;
        }

        for (QVector<PartitionInfo>::iterator partIter = diskIter.value().m_partition.begin(); partIter != diskIter.value().m_partition.end(); ++partIter) {
            QMap<QString, PVInfo>::ConstIterator pvIter = lvmInfo.m_pvInfo.find(partIter->m_path);
            if (lvmInfo.m_pvInfo.end() != pvIter) {
                const PVInfo &pv = pvIter.value();
                LVMFlag flag = pvIter->noJoinVG() ? LVMFlag::LVM_FLAG_NOT_JOIN_VG : LVMFlag::LVM_FLAG_JOIN_VG; //判断pv是否加入vg
                diskIter->m_vgFlag = flag;
                partIter->m_vgFlag = flag;
                partIter->m_vgData = pv2vg_func(pv);
                diskIter->m_vglist.push_back(partIter->m_vgData);
            }
        }
    }

    return true;
}

bool LVMOperator::getDeviceDataAndLVMInfo(DeviceInfoMap &dev, LVMInfo &lvmInfo)
{
    m_devInfo = dev;
    updateLVMInfo(lvmInfo);
    getDeviceLVMData(dev, lvmInfo);
    return setLVMErr(lvmInfo, LVMError::LVM_ERR_NORMAL);
}

/******************************** 更新info ******************************/
bool LVMOperator::updateLVMInfo(LVMInfo &lvmInfo)
{
    initSuport();
    resetLVMInfo(lvmInfo);
    updateVGInfo(lvmInfo);
    updatePVInfo(lvmInfo);
    return setLVMErr(lvmInfo, LVMError::LVM_ERR_NORMAL);
}

bool LVMOperator::updatePVInfo(LVMInfo &lvmInfo)
{
    if (Utils::findProgramInPath("lsblk").isEmpty() || LVM_CMD_Support::NONE == m_lvmSupport.LVM_CMD_pvs) {
        return setLVMErr(lvmInfo, LVMError::LVM_ERR_NO_CMD_SUPPORT);
    }

    lvmInfo.m_pvInfo.clear();

    QString strout, strerror;
    Utils::executCmd("pvs --noheadings -o pv_name", strout, strerror);
    strout = strout.trimmed();
    foreach (QString str, strout.split("\n")) {
        if (str.isEmpty()) {
            continue;
        }

        PVInfo pv;
        pv.m_pvPath = str.trimmed();
        QString cmd = QString("pvs %1  --units b --noheadings --separator # "
                              "-o vg_name,pv_fmt,pv_size,pv_free,pv_uuid,pv_mda_count,pv_attr,"
                              "pv_pe_alloc_count,pv_pe_count,pv_mda_size,vg_extent_size,vg_uuid"
                              "").arg(pv.m_pvPath);
        Utils::executCmd(cmd, strout, strerror);
        QStringList pvsRes = strout.split("#");

        //获取基本属性
        pv.m_vgName = getStringListItem(pvsRes, 0).trimmed();
        pv.m_pvFmt = getStringListItem(pvsRes, 1).trimmed();
        pv.m_pvByteTotalSize = getStringListItem(pvsRes, 2).trimmed().replace('B', "").toLongLong();
        pv.m_pvByteFreeSize = getStringListItem(pvsRes, 3).trimmed().replace('B', "").toLongLong();
        pv.m_pvSize = Utils::LVMFormatSize(pv.m_pvByteTotalSize);
        pv.m_pvFree = Utils::LVMFormatSize(pv.m_pvByteFreeSize);
        pv.m_pvUuid = getStringListItem(pvsRes, 4).trimmed();
        pv.m_pvMdaCount = getStringListItem(pvsRes, 5).toInt();
        pv.m_pvStatus = getStringListItem(pvsRes, 6).trimmed();
        pv.m_pvUsedPE = getStringListItem(pvsRes, 7).toInt();
        pv.m_pvUnusedPE = getStringListItem(pvsRes, 8).toInt() - pv.m_pvUsedPE;
        pv.m_pvMdaSize = getStringListItem(pvsRes, 9).replace('B', "").toInt();
        pv.m_PESize = getStringListItem(pvsRes, 10).replace('B', "").toInt();
        pv.m_vgUuid = getStringListItem(pvsRes, 11).trimmed();

        //设备类型
        cmd = QString("lsblk %1 -o type -nd").arg(pv.m_pvPath);
        Utils::executCmd(cmd, strout, strerror);
        if (strout.contains("part")) {
            pv.m_lvmDevType = DevType::DEV_PARTITION;
        } else if (strout.contains("disk")) {
            pv.m_lvmDevType = DevType::DEV_DISK;
        } else if (strout.contains("loop")) {
            pv.m_lvmDevType = DevType::DEV_LOOP;
        } else if (strout.contains("raid")) {
            pv.m_lvmDevType = DevType::DEV_META_DEVICES;
        } else if (strout.contains("crypt")) {
            pv.m_lvmDevType = DevType::DEV_META_DEVICES;
        } else {
            pv.m_lvmDevType = DevType::DEV_UNKNOW_DEVICES;
        }

        //pv上pe使用情况
        cmd = QString("pvs %1 -o seg_type,pvseg_all,lv_path,lv_name --noheadings  --separator ### ").arg(pv.m_pvPath);
        Utils::executCmd(cmd, strout, strerror);
        foreach (QString str, strout.split("\n")) {
            QStringList list = str.trimmed().split("###");
            if (list.size() < 4) {
                continue;
            }
            QVector<LV_PV_Ranges>lvVec;
            VG_PV_Ranges vgRanges;
            vgRanges.m_vgName = pv.m_vgName;
            vgRanges.m_vgUuid = pv.m_vgUuid;
            vgRanges.m_used = false;
            vgRanges.m_start = getStringListItem(list, 1).toLongLong();
            vgRanges.m_end = getStringListItem(list, 2).toLongLong() + vgRanges.m_start - 1;
            if (!list[0].contains("free")) {
                vgRanges.m_used = true;
                LV_PV_Ranges lvRanges = vgRanges;
                lvRanges.m_devPath = getStringListItem(list, 3);
                lvRanges.m_lvName = getStringListItem(list, 4);
                lvVec.push_back(lvRanges);
                pv.m_lvRangesList.insert(lvRanges.m_devPath, lvVec);
            }
            pv.m_vgRangesList.push_back(vgRanges);
        }
        if (pv.joinVG() && lvmInfo.vgExists(pv)) {
            lvmInfo.m_vgInfo[pv.m_vgName].m_pvInfo.insert(pv.m_pvPath, pv);
        }
        lvmInfo.m_pvInfo.insert(pv.m_pvPath, pv);
    }

    return setLVMErr(lvmInfo, LVMError::LVM_ERR_NORMAL);
}

bool LVMOperator::updateVGInfo(LVMInfo &lvmInfo)
{
    if (m_lvmSupport.LVM_CMD_vgs == LVM_CMD_Support::NONE) {
        return setLVMErr(lvmInfo, LVMError::LVM_ERR_NO_CMD_SUPPORT);
    }
    //查看是否有重名vg存在 如果存在 将其中一个vg更换名称
    if (!checkVG()) {
        return setLVMErr(lvmInfo, LVMError::LVM_ERR_VG_ALREADY_EXISTS);
    }

    lvmInfo.m_vgInfo.clear();

    QString strout, strerror;
    Utils::executCmd("vgs --noheadings -o vg_uuid", strout, strerror);
    strout = strout.trimmed();

    foreach (QString str, strout.split("\n")) {
        if (str.isEmpty()) {
            continue;
        }

        VGInfo vg;
        vg.m_vgUuid = str.trimmed();

        QString cmd = QString("vgs -S vg_uuid=%1 --units b --noheadings --separator # "
                              "-o vg_name,vg_size,vg_free,pv_count,vg_extent_count,vg_free_count,lv_count,vg_extent_size,vg_attr "
                              "").arg(vg.m_vgUuid);
        Utils::executCmd(cmd, strout, strerror);
        QStringList vgsRes = getStringListItem(strout.split("\n"), 0).split("#");
        if (vgsRes.size() < 9) {
            vg.m_vgError = LVMError::LVM_ERR_VG;
            vg.m_vgName = getStringListItem(vgsRes, 0).trimmed();
            lvmInfo.m_vgInfo.insert(vg.m_vgName, vg);
            continue;
        }

        //获取基本属性
        vg.m_vgName = getStringListItem(vgsRes, 0).trimmed();
        long long Size = getStringListItem(vgsRes, 1).trimmed().replace('B', "").toLongLong();
        long long unUsed = getStringListItem(vgsRes, 2).trimmed().replace('B', "").toLongLong();
        vg.m_vgSize = Utils::LVMFormatSize(Size);
        vg.m_vgUnused = Utils::LVMFormatSize(unUsed);
        vg.m_vgUsed = Utils::LVMFormatSize(Size - unUsed);
        vg.m_pvCount = getStringListItem(vgsRes, 3).toInt();
        vg.m_peCount = getStringListItem(vgsRes, 4).toInt();;
        vg.m_peUnused  = getStringListItem(vgsRes, 5).toInt();;
        vg.m_peUsed = vg.m_peCount - vg.m_peUnused;
        vg.m_curLV  = getStringListItem(vgsRes, 6).toInt();
        vg.m_PESize = getStringListItem(vgsRes, 7).trimmed().replace('B', "").toInt();
        vg.m_vgStatus = getStringListItem(vgsRes, 8).trimmed();
        //获取lv基本属性
        updateLVInfo(lvmInfo, vg);
        lvmInfo.m_vgInfo.insert(vg.m_vgName, vg);
    }

    return setLVMErr(lvmInfo, LVMError::LVM_ERR_NORMAL);
}

bool LVMOperator::updateLVInfo(LVMInfo &lvmInfo, VGInfo &vg)
{
    if (LVM_CMD_Support::NONE == m_lvmSupport.LVM_CMD_lvs) {
        return setLVMErr(lvmInfo, LVMError::LVM_ERR_NO_CMD_SUPPORT);
    }

    vg.m_lvlist.clear();

    QString strout, strerror;
    QString cmd = QString("lvs -S vg_uuid=%1 --noheadings -o lv_path").arg(vg.m_vgUuid);
    Utils::executCmd(cmd, strout, strerror);
    strout = strout.trimmed();
    foreach (QString str, strout.split("\n")) {
        if (str.isEmpty()) {
            continue;
        }
        LVInfo lv;
        //基本属性
        lv.m_lvPath = str.trimmed(); //lv名称 lv0 lv1 ...
        lv.m_vgName = vg.m_vgName; //vg名称
        lv.m_LESize = vg.m_PESize;  //单个pe大小 与所在vg的pe大小相同 单位byte

        QString cmd = QString("lvs %1 -S vg_uuid=%2 --units b --noheadings --separator # "
                              " -o lv_name,lv_uuid,lv_attr,lv_size"
                              "").arg(lv.m_lvPath).arg(vg.m_vgUuid);
        Utils::executCmd(cmd, strout, strerror);
        QStringList lvsRes = strout.split("#");

        lv.m_lvName = getStringListItem(lvsRes, 0).trimmed();
        lv.m_lvUuid = getStringListItem(lvsRes, 1).trimmed(); //lv uuid 唯一名称
        lv.m_lvStatus = getStringListItem(lvsRes, 2).trimmed();
        long long Size = getStringListItem(lvsRes, 3).trimmed().replace('B', "").toLongLong();
        lv.m_lvSize = Utils::LVMFormatSize(Size); //字符串类型 展示用
        lv.m_lvLECount = Size / lv.m_LESize;

        QString rootFsName;
        MountInfo::loadCache(rootFsName);
        QString lvPath = lv.toMapperPath();
        if (rootFsName == lvPath) {
            lv.m_dataFlag = true;
        }

        bool labelFound = false;
        QString label = DiskManager::FsInfo::getLabel(lvPath, labelFound);
        if (labelFound) {
            lv.m_fileSystemLabel = label;
        }

        auto devIt = m_devInfo.find(lvPath);
        if (devIt != m_devInfo.end()) {
            auto partIt = devIt.value().m_partition.begin();
            if (partIt != devIt.value().m_partition.end() && partIt->m_devicePath == lvPath) {
                PartitionInfo part = *partIt;
                lv.m_busy = part.m_busy;
                lv.m_mountPoints = part.m_mountPoints;
                lv.m_lvFsType = static_cast<FSType>(part.m_fileSystemType);
                lv.m_fsUsed = part.m_sectorsUsed * devIt.value().m_sectorSize;
                lv.m_fsUnused = part.m_sectorsUnused * devIt.value().m_sectorSize;
                lv.m_mountUuid = part.m_uuid;
                lv.m_fsLimits = part.m_fsLimits;
                if (FS_FAT32 == lv.m_lvFsType || FS_FAT16 == lv.m_lvFsType) {
                    lv.m_fsLimits = FS_Limits(-1, -1); //fat格式不支持逻辑卷的扩展缩小
                } else if (FS_UNALLOCATED ==  lv.m_lvFsType) { //empty fs , no limits
                    lv.m_fsLimits = FS_Limits(0, 0);
                }
            }
        }
        vg.m_lvlist.push_back(lv);
    }

    long long Size = vg.m_peUnused * vg.m_PESize;
    if (Size) {
        LVInfo unallocLv;
        unallocLv.m_lvPath = "Unallocated";
        unallocLv.m_vgName = vg.m_vgName; //vg名称
        unallocLv.m_LESize = vg.m_PESize;
        unallocLv.m_lvSize = Utils::LVMFormatSize(Size); //字符串类型 展示用
        unallocLv.m_lvLECount = Size / unallocLv.m_LESize;
        unallocLv.m_fsUsed = -1;
        unallocLv.m_fsUnused = -1;
        unallocLv.m_lvFsType = FSType::FS_UNALLOCATED;
        unallocLv.m_busy = false;
        vg.m_lvlist.push_back(unallocLv);
    }

    return setLVMErr(lvmInfo, LVMError::LVM_ERR_NORMAL);
}

/******************************** lvm操作 ******************************/
bool LVMOperator::createVG(LVMInfo &lvmInfo, QString vgName, QList<PVData> devList, long long size)
{
    if (LVM_CMD_Support::NONE == m_lvmSupport.LVM_CMD_vgcreate  || LVM_CMD_Support::NONE == m_lvmSupport.LVM_CMD_pvcreate) {
        return setLVMErr(lvmInfo, LVMError::LVM_ERR_NO_CMD_SUPPORT);
    }

    if (vgName.isEmpty()  || vgName.count() >= 128 || devList.count() == 0 || size == 0) {
        return setLVMErr(lvmInfo, LVMError::LVM_ERR_VG_ARGUMENT);
    }

    //判断vg是否已经存在 存在退出
    if (lvmInfo.vgExists(vgName)) {
        return setLVMErr(lvmInfo, LVMError::LVM_ERR_VG_ALREADY_EXISTS);
    }
    //创建pv
    QStringList vgPV, strout, strerror;
    foreach (PVData pv, devList) {
        if (lvmInfo.pvExists(pv)) { //如果pv存在
            if (lvmInfo.getPV(pv).joinVG()) { //已经加入vg
                return setLVMErr(lvmInfo, LVMError::LVM_ERR_VG_ARGUMENT);
            }
        } else {
            if (!pvCreate(pv.m_devicePath)) { //可能出现的问题有 输入输出错误  pv创建错误等,此处可能修改函数
                return setLVMErr(lvmInfo, LVMError::LVM_ERR_PV_CREATE_FAILED);
            }
        }

        vgPV.push_back(pv.m_devicePath);
    }

    //创建vg
    int errCode = vgCreate(vgName, vgPV);
    if (errCode != 0) {
        if (errCode == 3) { //之后可能出现的问题有 vg存在 输入输出设备错误  vg创建失败
            return setLVMErr(lvmInfo, LVMError::LVM_ERR_VG_ALREADY_EXISTS);
        } else {
            return setLVMErr(lvmInfo, LVMError::LVM_ERR_VG_CREATE_FAILED);
        }
    }

    //更新lvm数据
    updateLVMInfo(lvmInfo);
    return setLVMErr(lvmInfo, lvmInfo.vgExists(vgName) ? LVM_ERR_NORMAL : LVM_ERR_VG_CREATE_FAILED);
}

bool LVMOperator::deleteVG(LVMInfo &lvmInfo, QStringList vglist)
{
    if (LVM_CMD_Support::NONE == m_lvmSupport.LVM_CMD_pvremove
            || LVM_CMD_Support::NONE == m_lvmSupport.LVM_CMD_vgremove
            || LVM_CMD_Support::NONE == m_lvmSupport.LVM_CMD_lvremove) {
        return setLVMErr(lvmInfo, LVMError::LVM_ERR_NO_CMD_SUPPORT);
    }
    foreach (const QString &vgName, vglist) {
        if (!lvmInfo.vgExists(vgName)) { //vg 不存在 报错返回
            return setLVMErr(lvmInfo, LVMError::LVM_ERR_VG_NO_EXISTS);
        }
        VGInfo vgInfo = lvmInfo.getVG(vgName);

        //删除lv
        QStringList lvList;
        for (int i = 0; i < vgInfo.m_lvlist.count() - 1; ++i) { //删除lv
            lvList << vgInfo.m_lvlist[i].m_lvPath; //最后一个lv不要删除 前端显示占位用;
        }

        if (!lvRemove(lvmInfo, lvList)) {
            return setLVMErr(lvmInfo, lvmInfo.m_lvmErr);
        }

        //删除vg
        int exitCode =  vgRemove(vgInfo);
        if (exitCode != 0) {
            return exitCode == 5 ? setLVMErr(lvmInfo, LVMError::LVM_ERR_VG_IN_USED) : setLVMErr(lvmInfo, LVMError::LVM_ERR_VG_DELETE_FAILED);
        }

        //删除pv
        foreach (const QString &devPath, vgInfo.m_pvInfo.keys()) { //删除pv
            if (!pvRemove(devPath)) {
                return setLVMErr(lvmInfo, LVMError::LVM_ERR_PV_DELETE_FAILED);
            }
        }
    }
    return setLVMErr(lvmInfo, LVMError::LVM_ERR_NORMAL);
}

bool LVMOperator::resizeVG(LVMInfo &lvmInfo, QString vgName, QList<PVData> devList, long long size)
{
    if (!lvmInfo.vgExists(vgName) || devList.size() <= 0 || size <= 0) {
        return setLVMErr(lvmInfo, LVMError::LVM_ERR_VG_ARGUMENT);
    }
    if (LVM_CMD_Support::NONE == m_lvmSupport.LVM_CMD_vgextend
            ||  LVM_CMD_Support::NONE == m_lvmSupport.LVM_CMD_vgreduce
            ||  LVM_CMD_Support::NONE == m_lvmSupport.LVM_CMD_pvcreate
            ||  LVM_CMD_Support::NONE == m_lvmSupport.LVM_CMD_pvremove
            ||  LVM_CMD_Support::NONE == m_lvmSupport.LVM_CMD_pvmove) {
        return setLVMErr(lvmInfo, LVMError::LVM_ERR_NO_CMD_SUPPORT);
    }


    VGInfo vg = lvmInfo.getVG(vgName);
    QList<PVData>oldPVlist;
    QList<PVData>delPVList;

    foreach (const PVData &pv, devList) {
        if (lvmInfo.pvOfVg(vgName, pv)) { //pv存在 且属于当前vg
            oldPVlist.push_back(pv);
            continue;
        }

        QString output, error;
        Utils::executCmd(QString("lsblk %1 --raw --output NAME").arg(pv.m_devicePath), output, error);
        auto strList = output.split("\n");
        //安装器全盘加密时已经创建了pv
        if (strList.size() > 2 && strList.at(1) == "luks_crypt0") {
            continue;
        }

        if (!lvmInfo.pvExists(pv)) {    //pv不存在
            if (!pvCreate(pv.m_devicePath)) {
                return setLVMErr(lvmInfo, LVMError::LVM_ERR_PV_CREATE_FAILED);
            }
        } else {
            if (lvmInfo.getPV(pv).joinVG()) { //加入了其他vg
                return setLVMErr(lvmInfo, LVMError::LVM_ERR_VG_ARGUMENT);
            }
        }
        //扩展vg
        if (!vgExtend(vgName, pv.m_devicePath)) {
            return setLVMErr(lvmInfo, LVMError::LVM_ERR_VG_EXTEND_FAILED);
        }
    }
    //查找删除的列表
    foreach (auto it, vg.m_pvInfo) {
        auto it2 = std::find_if(oldPVlist.begin(), oldPVlist.end(), [ = ](const PVData & pv)->bool{
            return pv.m_devicePath == it.m_pvPath;
        });
        if (it2 == oldPVlist.end()) {
            PVData data;
            data.m_pvAct = LVM_ACT_DELETEPV;
            data.m_devicePath = it.m_pvPath;
            delPVList.push_back(data);
        }
    }

    updateLVMInfo(lvmInfo);

    return deletePVList(lvmInfo, delPVList);
}

bool LVMOperator::createLV(LVMInfo &lvmInfo, QString vgName, QList<LVAction> lvList)
{
    if (LVM_CMD_Support::NONE == m_lvmSupport.LVM_CMD_lvcreate) {
        return setLVMErr(lvmInfo, LVMError::LVM_ERR_NO_CMD_SUPPORT);
    }

    //判断参数是否正确
    if (vgName.isEmpty()  || lvList.count() == 0) {
        return setLVMErr(lvmInfo, LVMError::LVM_ERR_LV_ARGUMENT);
    }

    //vg是否存在
    auto vgIt = lvmInfo.m_vgInfo.find(vgName);
    if (lvmInfo.m_vgInfo.end() == vgIt) {
        return setLVMErr(lvmInfo, LVMError::LVM_ERR_VG_NO_EXISTS);
    }

    VGInfo &vg = vgIt.value();
    long long vgSize = vg.m_peUnused * vg.m_PESize;
    set<QString>s_tmp;
    foreach (const LVAction &clv, lvList) {
        vgSize -= clv.m_lvByteSize;
        if (vgSize < 0
                || clv.m_user.isEmpty()
                || clv.m_vgName != vgName
                || clv.m_lvName.isEmpty()
                || clv.m_lvName.count() >= 128
                || clv.m_lvByteSize <= 0  // lv大小不超过vg未使用空间  lv是否重复 用户名是否存在等条件
                /*|| m_supportFs.getFsObject(clv.m_lvFs) == nullptr   //判断文件系统是否支持      不要在此判断文件系统 这里应该专注于创建lv*/
                || !(s_tmp.insert(clv.m_lvName).second)) {          //插入失败 说明lvList 列表中存在重复的lv名称
            return setLVMErr(lvmInfo, LVMError::LVM_ERR_LV_ARGUMENT);
        }

        //查找vg中是否已经存在了lvName 存在报错退出
        if (vg.lvInfoExists(clv.m_lvName)) {
            return setLVMErr(lvmInfo, LVMError::LVM_ERR_LV_ALREADY_EXISTS);
        }
    }

    //创建lv
    foreach (const LVAction &clv, lvList) {
        if (!lvCreate(clv.m_vgName, clv.m_lvName, clv.m_lvByteSize)) {
            return setLVMErr(lvmInfo, LVMError::LVM_ERR_LV_CREATE_FAILED);
        }
    }

    //更新lv数据
    updateLVInfo(lvmInfo, vg);
    return setLVMErr(lvmInfo, LVMError::LVM_ERR_NORMAL);
}

bool LVMOperator::lvRemove(LVMInfo &lvmInfo, QStringList lvlist)
{
    if (LVM_CMD_Support::NONE == m_lvmSupport.LVM_CMD_lvremove) {
        return setLVMErr(lvmInfo, LVMError::LVM_ERR_NO_CMD_SUPPORT);
    }
    foreach (const QString &lvPath, lvlist) {
        int exitCode = lvRemove(lvPath);
        if (exitCode != 0) {
            return exitCode == 5 ? setLVMErr(lvmInfo, LVMError::LVM_ERR_LV_IN_USED) : setLVMErr(lvmInfo, LVMError::LVM_ERR_LV_DELETE_FAILED);
        }
    }
    return setLVMErr(lvmInfo, LVMError::LVM_ERR_NORMAL);
}

bool LVMOperator::resizeLV(LVMInfo &lvmInfo,  LVAction &lvAct, LVInfo &info)
{
    if ((LVM_ACT_LV_EXTEND == lvAct.m_lvAct && LVM_CMD_Support::NONE == m_lvmSupport.LVM_CMD_lvextend)
            || (LVM_ACT_LV_REDUCE == lvAct.m_lvAct   && LVM_CMD_Support::NONE == m_lvmSupport.LVM_CMD_lvreduce)
            || FS_FAT16 == lvAct.m_lvFs || FS_FAT32 == lvAct.m_lvFs) {
        return setLVMErr(lvmInfo, LVMError::LVM_ERR_NO_CMD_SUPPORT);
    }

    FileSystem *fs = m_supportFs.getFsObject(lvAct.m_lvFs);
    if (fs == nullptr) {
        if (lvAct.m_lvFs == FS_UNALLOCATED) { //文件系统不存在 可以直接扩大或缩小lv
            return setLVMErr(lvmInfo, resizeLV(info.m_lvPath, lvAct) ? LVMError::LVM_ERR_NORMAL : (lvAct.m_lvAct == LVM_ACT_LV_EXTEND ? LVM_ERR_LV_EXTEND_FAILED : LVM_ERR_LV_REDUCE_FAILED));
        }
        return setLVMErr(lvmInfo, LVMError::LVM_ERR_LV_RESIZE_FS_NO_SUPPORT); //fs no support resize
    }

    Partition p;
    p.setPath(info.m_lvPath);
    p.m_sectorStart = 0;
    p.m_sectorEnd = lvAct.m_lvByteSize / info.m_LESize - 1;
    p.m_sectorSize = info.m_LESize;
    if (lvAct.m_lvAct == LVM_ACT_LV_EXTEND) {
        //放大lv
        if (!resizeLV(info.m_lvPath, lvAct)) {
            return setLVMErr(lvmInfo, LVMError::LVM_ERR_LV_EXTEND_FAILED);
        }
        //放大文件系统
        if (!(fs->checkRepair(p) && fs->resize(p, true))) {
            return setLVMErr(lvmInfo, LVMError::LVM_ERR_LV_EXTEND_FS_FAILED);
        }
    } else {
        //缩小文件系统后
        if (!(fs->checkRepair(p) && fs->resize(p, false))) {
            return setLVMErr(lvmInfo, LVMError::LVM_ERR_LV_EXTEND_FS_FAILED);
        }
        //缩小lv
        if (!resizeLV(info.m_lvPath, lvAct)) {
            return setLVMErr(lvmInfo, LVMError::LVM_ERR_LV_EXTEND_FAILED);
        }
        //再次放大文件系统的原因是 进制单位不同 导致缩小后的文件系统跟设备大小不对等 所以再次扩大文件系统使得文件系统填充设备
        if (!(fs->checkRepair(p) && fs->resize(p, true))) {
            return setLVMErr(lvmInfo, LVMError::LVM_ERR_LV_EXTEND_FAILED);
        }
    }
    return setLVMErr(lvmInfo, LVMError::LVM_ERR_NORMAL);
}

bool LVMOperator::deletePVList(LVMInfo &lvmInfo, QList<PVData> devList)
{
    if (devList.size() == 0) {
        return setLVMErr(lvmInfo, LVMError::LVM_ERR_NORMAL);
    }

    set<QString>pvList;
    QVector<QString>allDeletePV;
    QSet<QString> vgList;

    foreach (const PVData &pv, devList) {
        if (!lvmInfo.pvExists(pv)                                           //pv不存在
                || pv.m_pvAct != LVMAction::LVM_ACT_DELETEPV                //参数错误
                || !pvList.insert(pv.m_devicePath).second) {                //pv重复
            return setLVMErr(lvmInfo, LVMError::LVM_ERR_PV_ARGUMENT);
        }

        PVInfo info = lvmInfo.getPV(pv);
        allDeletePV.push_back(info.m_pvPath);
    }

    if (m_lvmSupport.LVM_CMD_pvremove == LVM_CMD_Support::NONE) {           //命令不支持
        return setLVMErr(lvmInfo, LVMError::LVM_ERR_NO_CMD_SUPPORT);
    }

    foreach (const PVData &pv, devList) {
        PVInfo info = lvmInfo.getPV(pv);
        if (info.joinVG()) {                                                //加入vg
            if (!lvmInfo.vgExists(info)) {                                  //vg是否存在
                return setLVMErr(lvmInfo, LVMError::LVM_ERR_VG_NO_EXISTS);
            }
            VGInfo vg = lvmInfo.getVG(info.m_vgName);
            if (vg.isAllPV(allDeletePV)) {
                vgList.insert(vg.m_vgName);
                continue;
            }

            //pv正常删除 //todo 此处可以优化算法  找出节省时间和移动最少的move方式
            if (info.m_pvUsedPE > 0) {                                      //pv存在被使用 移动pv
                if (m_lvmSupport.LVM_CMD_pvmove == LVM_CMD_Support::NONE) { //命令不支持
                    return setLVMErr(lvmInfo, LVMError::LVM_ERR_NO_CMD_SUPPORT);
                }
                if (!pvMove(info.m_pvPath)) {                               //移动失败
                    return setLVMErr(lvmInfo, LVMError::LVM_ERR_PV_MOVE_FAILED);
                }
            }

            //移除pv 在vg内
            if (m_lvmSupport.LVM_CMD_vgreduce == LVM_CMD_Support::NONE) {   //命令不支持
                return setLVMErr(lvmInfo, LVMError::LVM_ERR_NO_CMD_SUPPORT);
            }
            if (!vgReduce(info.m_vgName, info.m_pvPath)) {                  //移除pv失败
                return setLVMErr(lvmInfo, LVMError::LVM_ERR_VG_REDUCE_FAILED);
            }
        }

        //删除pv
        if (!pvRemove(info.m_pvPath)) {
            return setLVMErr(lvmInfo, LVMError::LVM_ERR_PV_DELETE_FAILED);
        }
    }

    if (vgList.size()) {
        return deleteVG(lvmInfo, vgList.toList());
    }

    return setLVMErr(lvmInfo, LVMError::LVM_ERR_NORMAL);
}

/******************************** private lvm操作 ******************************/
int LVMOperator::vgCreate(const QString &vgName, const QStringList &pvList)
{
    QString strout, strerror;
    return Utils::executCmd(QString("vgcreate %1 %2 -y").arg(vgName).arg(pvList.join(" ")), strout, strerror);
}

int LVMOperator::vgRemove(const VGInfo &vgInfo)
{
    //删除vg
    QString strout, strerror;
    return  Utils::executCmd(QString("vgremove %1 -y").arg(vgInfo.m_vgName), strout, strerror); //删除vg
}

bool LVMOperator::vgRename(const QString &uuid, const QString &newName)
{
    if (m_lvmSupport.LVM_CMD_vgrename == LVM_CMD_Support::NONE || uuid.isEmpty() || newName.isEmpty()) {
        return false;
    }
    QString strout, strerror;
    return Utils::executCmd(QString("vgrename %1 %2 -y").arg(uuid).arg(newName), strout, strerror) == 0;
}

bool LVMOperator::vgReduce(const QString &vgName, const QString &pvPath)
{
    QString strout, strerror;
    return Utils::executCmd(QString("vgreduce %1 %2 -y").arg(vgName).arg(pvPath), strout, strerror) == 0;
}

bool LVMOperator::vgExtend(const QString &vgName, const QString &pvPath)
{
    QString strout, strerror;
    return Utils::executCmd(QString("vgextend %1 %2 -y").arg(vgName).arg(pvPath), strout, strerror) == 0;
}

bool LVMOperator::checkVG()
{
    struct VG {
        QString vgName;
        QString vgUuid;

        bool operator<(const VG &tmp)const
        {
            return this->vgUuid < tmp.vgUuid;
        }
    } ;

    QString strout, strerror;
    Utils::executCmd("vgs --noheadings --separator # "
                     "-o vg_name,vg_uuid", strout, strerror);
    strout = strout.trimmed();

    //获取所有vg集合
    QMap<QString, set<VG>>vgList;
    foreach (const QString &str, strout.split("\n")) {
        QStringList list = str.trimmed().split("#");
        if (list.count() == 2) {
            VG vg { list[0], list[1]};
            auto it = vgList.find(vg.vgName);
            if (it != vgList.end()) {
                it.value().insert(vg);
            } else {
                vgList.insert(vg.vgName, set<VG>() = {vg});
            }
        }
    }

    //查找重复
    QMap<QString, set<VG>>copy = vgList;
    QVector<VG> renameVg;
    for (QMap<QString, set<VG>>::iterator  it = vgList.begin(); it != vgList.end(); ++it) {
        int i = 1;
        for (auto vit = (++it.value().begin()); vit != it.value().end(); ++vit) {
            QString tmpVgName =  QString("%1_%2").arg(vit->vgName).arg(i);
            auto mit = copy.find(tmpVgName);
            while (mit != copy.end()) {
                tmpVgName =  QString("%1_%2").arg(vit->vgName).arg(++i);
                mit = copy.find(tmpVgName);
            }
            VG vg{tmpVgName, vit->vgUuid};
            copy.insert(tmpVgName, set<VG>() = {vg});
            renameVg.push_back(vg);
        }
    }

    //修改名称
    foreach (const VG &vg, renameVg) {
        vgRename(vg.vgUuid, vg.vgName);
    }
    return true;
}

bool LVMOperator::pvCreate(const QString &devPath)
{
    QString strout, strerror;
    auto errCode = Utils::executCmd(QString("pvcreate %1 -y").arg(devPath), strout, strerror);
    qInfo() << "pvcreate " << devPath << " output:" << strout << " error:" << strerror;
    return errCode == 0;
}

bool LVMOperator::pvMove(const QString &pvPath, const QString &dest)
{
    QString strout, strerror;
    return Utils::executCmd(QString("pvmove %1 %2-y").arg(pvPath).arg(dest), strout, strerror) == 0;
}

bool LVMOperator::pvRemove(const QString &devPath)
{
    QString strout, strerror;
    return Utils::executCmd(QString("pvremove %1 -y").arg(devPath), strout, strerror) == 0;
}

bool LVMOperator::lvCreate(const QString &vgName, const QString &lvName, const long long &size)
{
    QString strout, strerror;
    return Utils::executCmd(QString("lvcreate -L %1b -n %2 %3 -y").arg(size).arg(lvName).arg(vgName), strout, strerror) == 0;
}

int LVMOperator::lvRemove(const QString &lvPath)
{
    if (DiskManager::MountInfo::getMountedMountpoints(lvPath).count() == 0) {
        QString strout, strerror;
        return Utils::executCmd(QString("lvremove %1 -y").arg(lvPath), strout, strerror);
    }
    return 5; //因为lvremove 删除一个挂载的lv时 返回的错误码是5 如果发现待删除的lv被挂载了，就返回5 与lvremove错误码保持一致
}

bool LVMOperator::resizeLV(const QString &devPath, const LVAction &act)
{
    QString cmd, strout, strerr;
    if (LVM_ACT_LV_EXTEND == act.m_lvAct) {
        cmd = "lvextend" ;
    } else if (LVM_ACT_LV_REDUCE == act.m_lvAct) {
        cmd = "lvreduce" ;
    } else {
        return  false;
    }
    cmd += QString(" -f -y -L %1b %2").arg(act.m_lvByteSize).arg(devPath);
    return  Utils::executCmd(cmd, strout, strerr) == 0;
}

/******************************** 打印输出 ******************************/

void LVMOperator::printError(const LVMError &error)
{
    switch (error) {
    case LVMError::LVM_ERR_NORMAL:
        qDebug() << "LVM_ERR_NORMAL";
        break;
    case LVMError::LVM_ERR_NO_CMD_SUPPORT:
        qDebug() << "LVM_ERR_NO_CMD_SUPPORT";
        break;
    case LVMError::LVM_ERR_PV:
        qDebug() << "LVM_ERR_PV";
        break;
    case LVMError::LVM_ERR_PV_CREATE_FAILED:
        qDebug() << "LVM_ERR_PV_CREATE_FAILED";
        break;
    case LVMError::LVM_ERR_PV_DELETE_FAILED:
        qDebug() << "LVM_ERR_PV_DELETE_FAILED";
        break;
    case LVMError::LVM_ERR_LV:
        qDebug() << "LVM_ERR_LV";
        break;
    case LVMError::LVM_ERR_LV_CREATE_FAILED:
        qDebug() << "LVM_ERR_LV_CREATE_FAILED";
        break;
    case LVMError::LVM_ERR_LV_ARGUMENT:
        qDebug() << "LVM_ERR_LV_ARGUMENT";
        break;
    case LVMError::LVM_ERR_LV_NO_EXISTS:
        qDebug() << "LVM_ERR_LV_NO_EXISTS";
        break;
    case LVMError::LVM_ERR_LV_DELETE_FAILED:
        qDebug() << "LVM_ERR_LV_DELETE_FAILED";
        break;
    case LVMError::LVM_ERR_VG:
        qDebug() << "LVM_ERR_VG";
        break;
    case LVMError::LVM_ERR_VG_CREATE_FAILED:
        qDebug() << "LVM_ERR_VG_CREATE_FAILED";
        break;
    case LVMError::LVM_ERR_VG_ARGUMENT:
        qDebug() << "LVM_ERR_VG_ARGUMENT";
        break;
    case LVMError::LVM_ERR_VG_NO_EXISTS:
        qDebug() << "LVM_ERR_VG_NO_EXISTS";
        break;
    case LVMError::LVM_ERR_VG_ALREADY_EXISTS:
        qDebug() << "LVM_ERR_VG_ALREADY_EXISTS";
        break;
    case LVMError::LVM_ERR_VG_DELETE_FAILED:
        qDebug() << "LVM_ERR_VG_DELETE_FAILED";
        break;
    default:
        qDebug() << "LVMError default";
    }
}

void LVMOperator::printLVMInfo(const LVMInfo &lvmInfo)
{
    qDebug() << "**********************************************************************";
    qDebug() << QString("LVMVGINFO:====================================being");
    foreach (auto it, lvmInfo.m_vgInfo) {
        printVGInfo(it);
    }
    qDebug() << QString("LVMVGINFO:====================================end");
    qDebug() << "**********************************************************************";

    qDebug() << QString("LVMPVINFO:====================================being");

    foreach (auto it, lvmInfo.m_pvInfo) {
        printPVInfo(it);
    }
    qDebug() << QString("LVMPVINFO:====================================end");
    qDebug() << "**********************************************************************";
}

void LVMOperator::printDeviceLVMData(const DeviceInfoMap &dev)
{
    auto printVGData = [ = ](const VGData & vg) {
        qDebug() << "vgName" << vg.m_vgName;
        qDebug() << "vgSize" << vg.m_vgSize;
        qDebug() << "m_vgUuid" << vg.m_vgUuid;
        qDebug() << "vgByteSize" << vg.m_vgByteSize;
        foreach (auto lvIt, vg.m_lvList) {
            qDebug() << "";
            qDebug() << "lvName" << lvIt.m_lvName;
            qDebug() << "lvPath" << lvIt.m_lvPath;
            qDebug() << "lvSize" << lvIt.m_lvSize;
            qDebug() << "lvByteSize" << lvIt.m_lvByteSize;
            qDebug() << "";
        }
    };

    qDebug() << "**********************************************************************";
    foreach (auto it, dev) {
        if (it.m_vgFlag == LVMFlag::LVM_FLAG_JOIN_VG) {
            qDebug() << QString("dev disk %1 LVMData:====================================being").arg(it.m_path);
            qDebug() << "vgData COUNT:" << it.m_vglist.count();
            foreach (auto vgIt, it.m_vglist) {
                printVGData(vgIt);
            }
            qDebug() << QString("dev disk %1 LVMData:====================================end").arg(it.m_path);
        }

        foreach (auto it2, it.m_partition) {
            if (it2.m_vgFlag) {
                qDebug() << QString("dev part %1 LVMData:====================================being").arg(it2.m_path);
                printVGData(it2.m_vgData);
                qDebug() << QString("dev part %1 LVMData:====================================end").arg(it2.m_path);
            }
        }
    }
    qDebug() << "**********************************************************************";
}

void LVMOperator::printPVInfo(const PVInfo &info)
{
    qDebug() << "PVInfo:====================================being";
    qDebug() << "vg_name: " << info.m_vgName;
    qDebug() << "pvFmt: " << info.m_pvFmt; //pv格式 lvm1 lvm2
    qDebug() << "pvPath: " << info.m_pvPath; //pv路径 /dev/sdb1 ...
    qDebug() << "pvUuid: " << info.m_pvUuid; //pv uuid 唯一名称
    qDebug() << "pvMdaSize: " << info.m_pvMdaSize; //pv metadata size
    qDebug() << "pvMdaCount: " << info.m_pvMdaCount; //pv metadata 个数
    qDebug() << "pvSize: " << info.m_pvSize; //字符串类型 展示用
    qDebug() << "pvFree: " << info.m_pvFree; //字符串类型 展示用
    qDebug() << "pvUsedPE: " << info.m_pvUsedPE; //已经使用pe个数
    qDebug() << "pvUnusedPE: " << info.m_pvUnusedPE; //未使用pe个数
    qDebug() << "PESize: " << info.m_PESize; //单个pe大小
    qDebug() << "pvStatus: " << info.m_pvStatus; //状态
    qDebug() << "pvError: " << info.m_pvError; //物理卷错误码

    for (auto it = info.m_lvRangesList.begin(); it != info.m_lvRangesList.end(); ++it) {
        //lv pv分布情况 vgName
        foreach (const LV_PV_Ranges &tmp, it.value()) {
            qDebug() << "";
            qDebug() << "lvName :" << tmp.m_lvName;
            qDebug() << "lv path:" << tmp.m_devPath ;
            qDebug() << "lv start:" << tmp.m_start ;
            qDebug() << "lv end:" << tmp.m_end ;
            qDebug() << "";
        }
    }

    foreach (const VG_PV_Ranges &tmp, info.m_vgRangesList) {
        //vg pv分布情况
        qDebug() << "";
        qDebug() << "vgName :" << tmp.m_vgName;
        qDebug() << "vg used:" << tmp.m_used ;
        qDebug() << "vg start:" << tmp.m_start ;
        qDebug() << "vg end:" << tmp.m_end ;
        qDebug() << "";
    }
    qDebug() << "lvmDevType: " << info.m_lvmDevType; //lvm 设备类型
    qDebug() << "pvByteTotalSize: " << info.m_pvByteTotalSize;//pv总大小  单位byte
    qDebug() << "pvByteFreeSize: " << info.m_pvByteFreeSize;//pv未使用大小 单位byte
    qDebug() << "PVInfo:====================================end";
}

void LVMOperator::printVGInfo(const VGInfo &info)
{
    qDebug() << "VGInfo:====================================being";
    qDebug() << "vgName: " << info.m_vgName; //vg名称 vg0, vg1 ....
    qDebug() << "vgUuid: " << info.m_vgUuid; //vg唯一名称 uuid
    qDebug() << "vgSize: " << info.m_vgSize; //字符串类型 展示用  vg总大小
    qDebug() << "vgUsed: " << info.m_vgUsed; //字符串类型 展示用  vg使用
    qDebug() << "vgUnused: " << info.m_vgUnused; //字符串类型 展示用 vg已用
    qDebug() << "pvCount: " << info.m_pvCount; //物理卷个数
    qDebug() << "peCount: " << info.m_peCount; //pe个数
    qDebug() << "peUsed: " << info.m_peUsed; //pe使用个数
    qDebug() << "peUnused: " << info.m_peUnused; //pe未使用个数
    qDebug() << "PESize: " << info.m_PESize; //单个pe大小
    qDebug() << "curLV: " << info.m_curLV; //当前lv个数
    qDebug() << "vgStatus: " << QString(info.m_vgStatus); //状态
    qDebug() << "vgError: " << info.m_vgError; //逻辑卷组错误码

    foreach (auto it, info.m_lvlist) {
        printLVInfo(it);
    }
    qDebug() << "pvList: " << info.m_pvInfo.keys(); //逻辑卷pv
    qDebug() << "VGInfo:====================================end";
}

void LVMOperator::printLVInfo(const LVInfo &info)
{
    qDebug() << "LVInfo:====================================being";
    qDebug() << "vgName: " << info.m_vgName; //vg名称
    qDebug() << "lvPath: " << info.m_lvPath; //lv路径
    qDebug() << "lvUuid: " << info.m_lvUuid; //lv uuid 唯一名称
    qDebug() << "lvName: " << info.m_lvName; //lv名称 lv0 lv1 ....
    qDebug() << "lvFsType: " << Utils::fileSystemTypeToString(info.m_lvFsType); //文件系统类型
    qDebug() << "lvSize: " << info.m_lvSize; //字符串类型 展示用
    qDebug() << "lvLECount: " << info.m_lvLECount; //le个数
    qDebug() << "fsUsed: " << info.m_fsUsed; //文件已经使用大小
    qDebug() << "fsUnused: " << info.m_fsUnused; //文件未使用大小
    qDebug() << "LESize: " << info.m_LESize; //单个pe大小 与所在vg的pe大小相同
    qDebug() << "busy: " << info.m_busy; //挂载标志
    qDebug() << "mountpoints: " << info.m_mountPoints;
    qDebug() << "lvStatus: " << QString(info.m_lvStatus);
    qDebug() << "lvError: " << info.m_lvError; //逻辑卷错误码
    qDebug() << "mountUuid: " << info.m_mountUuid;//逻辑卷挂载uuid
    qDebug() << "fsLimits_min_size: " << info.m_fsLimits.min_size;
    qDebug() << "fsLimits_max_size: " << info.m_fsLimits.max_size;
    qDebug() << "luksFlag: " << info.m_luksFlag;
    qDebug() << "fileSystemLabel: " << info.m_fileSystemLabel;
    qDebug() << "LVInfo:====================================end";
}

