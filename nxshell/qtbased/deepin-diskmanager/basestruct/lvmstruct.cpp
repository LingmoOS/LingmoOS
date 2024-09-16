// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "lvmstruct.h"
/*********************************** PVData *********************************************/
bool PVData::operator<(const PVData &tmp) const
{
    if (m_type == DevType::DEV_UNALLOCATED_PARTITION) {
        return m_devicePath     < tmp.m_devicePath
               || m_startSector < tmp.m_startSector
               || m_endSector   < tmp.m_endSector
               || m_diskPath    < tmp.m_diskPath
               || m_sectorSize  < tmp.m_sectorSize;
    }
    return m_devicePath < tmp.m_devicePath || m_diskPath < tmp.m_diskPath;
}

bool PVData::operator==(const PVData &tmp) const
{
    return m_devicePath     == tmp.m_devicePath
           && m_startSector == tmp.m_startSector
           && m_endSector   == tmp.m_endSector
           && m_type        == tmp.m_type
           && m_pvAct       == tmp.m_pvAct
           && m_diskPath    == tmp.m_diskPath
           && m_sectorSize  == tmp.m_sectorSize;
}

QDBusArgument &operator<<(QDBusArgument &argument, const PVData &data)
{
    argument.beginStructure();
    argument << data.m_devicePath
             << data.m_startSector
             << data.m_endSector
             << static_cast<int>(data.m_pvAct)
             << static_cast<int>(data.m_type)
             << data.m_diskPath
             << data.m_sectorSize;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, PVData &data)
{
    argument.beginStructure();
    int pvAct, type;
    argument >> data.m_devicePath
             >> data.m_startSector
             >> data.m_endSector
             >> pvAct
             >> type
             >> data.m_diskPath
             >> data.m_sectorSize;
    data.m_pvAct = static_cast<LVMAction>(pvAct);
    data.m_type = static_cast<DevType>(type);
    argument.endStructure();
    return argument;
}
/*********************************** CreateLVInfo *********************************************/
QDBusArgument &operator<<(QDBusArgument &argument, const LVAction &data)
{
    argument.beginStructure();
    argument << data.m_vgName
             << data.m_lvName
             << data.m_lvSize
             << data.m_lvByteSize
             << static_cast<int>(data.m_lvFs)
             << data.m_user
             << static_cast<int>(data.m_lvAct)
             << data.m_mountPoint
             << data.m_mountUuid
             << static_cast<int>(data.m_luksFlag)
             << static_cast<int>(data.m_crypt)
             << data.m_tokenList
             << data.m_decryptStr
             << data.m_dmName;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, LVAction &data)
{
    argument.beginStructure();
    int type, act, luksFlag, crypt;
    argument >> data.m_vgName
             >> data.m_lvName
             >> data.m_lvSize
             >> data.m_lvByteSize
             >> type
             >> data.m_user
             >> act
             >> data.m_mountPoint
             >> data.m_mountUuid
             >> luksFlag
             >> crypt
             >> data.m_tokenList
             >> data.m_decryptStr
             >> data.m_dmName;
    data.m_lvFs = static_cast<FSType>(type);
    data.m_lvAct = static_cast<LVMAction>(act);
    data.m_luksFlag = static_cast<LUKSFlag>(luksFlag);
    data.m_crypt = static_cast<CRYPT_CIPHER>(crypt);
    argument.endStructure();
    return argument;
}
/*********************************** LVDATA *********************************************/
QDBusArgument &operator<<(QDBusArgument &argument, const LVData &data)
{
    argument.beginStructure();
    argument << data.m_lvName
             << data.m_lvPath
             << data.m_lvSize
             << data.m_lvByteSize;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, LVData &data)
{
    argument.beginStructure();
    argument >> data.m_lvName
             >> data.m_lvPath
             >> data.m_lvSize
             >> data.m_lvByteSize;
    argument.endStructure();
    return argument;
}
/*********************************** VGDATA *********************************************/
QDBusArgument &operator<<(QDBusArgument &argument, const VGData &data)
{
    argument.beginStructure();
    argument << data.m_vgName
             << data.m_vgSize
             << data.m_vgUuid
             << data.m_vgByteSize
             << data.m_lvList;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, VGData &data)
{
    argument.beginStructure();
    argument >> data.m_vgName
             >> data.m_vgSize
             >> data.m_vgUuid
             >> data.m_vgByteSize
             >> data.m_lvList;
    argument.endStructure();
    return argument;
}
/*********************************** PVRANGES *********************************************/
QDBusArgument &operator<<(QDBusArgument &argument, const PVRanges &data)
{
    argument.beginStructure();
    argument << data.m_lvName
             << data.m_devPath
             << data.m_vgName
             << data.m_vgUuid
             << data.m_start
             << data.m_end
             << data.m_used;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, PVRanges &data)
{
    argument.beginStructure();
    argument >> data.m_lvName
             >> data.m_devPath
             >> data.m_vgName
             >> data.m_vgUuid
             >> data.m_start
             >> data.m_end
             >> data.m_used;
    argument.endStructure();
    return argument;
}
/*********************************** PVInfo *********************************************/
QDBusArgument &operator<<(QDBusArgument &argument, const PVInfo &data)
{
    argument.beginStructure();
    argument << data.m_pvFmt
             << data.m_vgName
             << data.m_pvPath
             << data.m_pvUuid
             << data.m_vgUuid
             << data.m_pvMdaSize
             << data.m_pvMdaCount
             << data.m_pvSize
             << data.m_pvFree
             << data.m_pvUsedPE
             << data.m_pvUnusedPE
             << data.m_PESize
             << data.m_pvStatus
             << static_cast<int>(data.m_pvError)
             << data.m_lvRangesList
             << data.m_vgRangesList
             << static_cast<int>(data.m_lvmDevType)
             << data.m_pvByteTotalSize
             << data.m_pvByteFreeSize;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,  PVInfo &data)
{
    argument.beginStructure();
    int err, devType;
    argument >> data.m_pvFmt
             >> data.m_vgName
             >> data.m_pvPath
             >> data.m_pvUuid
             >> data.m_vgUuid
             >> data.m_pvMdaSize
             >> data.m_pvMdaCount
             >> data.m_pvSize
             >> data.m_pvFree
             >> data.m_pvUsedPE
             >> data.m_pvUnusedPE
             >> data.m_PESize
             >> data.m_pvStatus
             >> err
             >> data.m_lvRangesList
             >> data.m_vgRangesList
             >> devType
             >> data.m_pvByteTotalSize
             >> data.m_pvByteFreeSize;
    data.m_pvError = static_cast<LVMError>(err);
    data.m_lvmDevType = static_cast<DevType>(devType);
    argument.endStructure();
    return argument;
}
/*********************************** LVInfo *********************************************/
QDBusArgument &operator<<(QDBusArgument &argument, const LVInfo &data)
{
    argument.beginStructure();
    argument << data.m_vgName
             << data.m_lvPath
             << data.m_lvUuid
             << data.m_lvName
             << static_cast<int>(data.m_lvFsType)
             << data.m_lvSize
             << data.m_lvLECount
             << data.m_fsUsed
             << data.m_fsUnused
             << data.m_LESize
             << data.m_busy
             << data.m_mountPoints
             << data.m_lvStatus
             << static_cast<int>(data.m_lvError)
             << data.m_mountUuid
             << data.m_fsLimits.max_size
             << data.m_fsLimits.min_size
             << static_cast<int>(data.m_luksFlag)
             << data.m_fileSystemLabel
             << data.m_dataFlag;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, LVInfo &data)
{
    argument.beginStructure();
    int err, type, luksFlag;
    argument >> data.m_vgName
             >> data.m_lvPath
             >> data.m_lvUuid
             >> data.m_lvName
             >> type
             >> data.m_lvSize
             >> data.m_lvLECount
             >> data.m_fsUsed
             >> data.m_fsUnused
             >> data.m_LESize
             >> data.m_busy
             >> data.m_mountPoints
             >> data.m_lvStatus
             >> err
             >> data.m_mountUuid
             >> data.m_fsLimits.max_size
             >> data.m_fsLimits.min_size
             >> luksFlag
             >> data.m_fileSystemLabel
             >> data.m_dataFlag;
    data.m_lvFsType = static_cast<FSType>(type);
    data.m_lvError = static_cast<LVMError>(err);
    data.m_luksFlag = static_cast<LUKSFlag>(luksFlag);
    argument.endStructure();
    return argument;
}
/*********************************** VGInfo *********************************************/
QDBusArgument &operator<<(QDBusArgument &argument, const VGInfo &data)
{
    argument.beginStructure();
    argument << data.m_vgName
             << data.m_vgUuid
             << data.m_vgSize
             << data.m_vgUsed
             << data.m_vgUnused
             << data.m_pvCount
             << data.m_peCount
             << data.m_peUsed
             << data.m_peUnused
             << data.m_PESize
             << data.m_curLV
             << data.m_vgStatus
             << static_cast<int>(data.m_vgError)
             << data.m_lvlist
             << data.m_pvInfo
             << static_cast<int>(data.m_luksFlag);
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,  VGInfo &data)
{
    argument.beginStructure();
    int err, luksFlag;
    argument >> data.m_vgName
             >> data.m_vgUuid
             >> data.m_vgSize
             >> data.m_vgUsed
             >> data.m_vgUnused
             >> data.m_pvCount
             >> data.m_peCount
             >> data.m_peUsed
             >> data.m_peUnused
             >> data.m_PESize
             >> data.m_curLV
             >> data.m_vgStatus
             >> err
             >> data.m_lvlist
             >> data.m_pvInfo
             >> luksFlag;
    data.m_vgError = static_cast<LVMError>(err);
    data.m_luksFlag = static_cast<LUKSFlag>(luksFlag);
    argument.endStructure();
    return argument;
}

LVInfo VGInfo::getLVinfo(const QString &lvName)
{
    foreach (const LVInfo &info, m_lvlist) {
        if (info.m_lvName == lvName) {
            return info;
        }
    }
    return  LVInfo();
}

bool VGInfo::lvInfoExists(const QString &lvName)
{
    foreach (const LVInfo &info, m_lvlist) {
        if (info.m_lvName == lvName) {
            return true;
        }
    }
    return  false;
}

bool VGInfo::isAllPV(QVector<QString> pvList) const
{
    QVector<QString> list = m_pvInfo.keys().toVector();
    if (pvList.size() < list.size()) {
        return false;
    }

    std::sort(list.begin(), list.end());
    std::sort(pvList.begin(), pvList.end());
    return std::includes(pvList.begin(), pvList.end(), list.begin(), list.end());
}
/*********************************** LVMInfo *********************************************/
QDBusArgument &operator<<(QDBusArgument &argument, const LVMInfo &data)
{
    argument.beginStructure();
    argument << data.m_pvInfo
             << data.m_vgInfo
             << static_cast<int>(data.m_lvmErr);
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,  LVMInfo &data)
{
    argument.beginStructure();
    int err;
    argument >> data.m_pvInfo
             >> data.m_vgInfo
             >> err;
    data.m_lvmErr = static_cast<LVMError>(err);
    argument.endStructure();
    return argument;
}

LVInfo LVMInfo::getLVInfo(const QString &lvPath)
{
    // /dev/vg01/lv01    Or /dev/mapper/vg01-lv01
    QStringList list = lvPath.split("/");
    list.pop_front();
    if (list.count() < 3) {
        return LVInfo();
    }

    if (lvPath.contains("/dev/mapper/")) {
        QStringList list2 = list[2].split("-");
        if (!vgExists(list2[0])) {
            return LVInfo();
        }

        VGInfo vg =  getVG(list2[0]);
        foreach (LVInfo lv, vg.m_lvlist) {
            if ("/dev/mapper/" + vg.m_vgName + "-" + lv.m_lvName == lvPath) {
                return lv;
            }
        }
        return LVInfo();
    }
    return getLVInfo(list[1], list[2]);
}

LVInfo LVMInfo::getLVInfo(const QString &vgName, const QString &lvName)
{
    //判断lv是否存在
    auto it = m_vgInfo.find(vgName);
    if (m_vgInfo.end() == it) {
        return LVInfo();
    }
    return it.value().getLVinfo(lvName);
}

VGInfo LVMInfo::getVG(const QString &vgName)
{
    return getItem(vgName, m_vgInfo);
}

VGInfo LVMInfo::getVG(const PVData &pv)
{
    if (pvExists(pv)) {
        PVInfo info = getPV(pv);
        return getVG(info.m_vgName);
    }
    return VGInfo();
}

VGInfo LVMInfo::getVG(const PVInfo &pv)
{
    return getVG(pv.m_vgName);
}

PVInfo LVMInfo::getPV(const QString &pvPath)
{
    return getItem(pvPath, m_pvInfo);
}

PVInfo LVMInfo::getPV(const PVData &pv)
{
    return getPV(pv.m_devicePath);
}

QVector<PVInfo> LVMInfo::getVGAllPV(const QString &vgName)
{
    return vgExists(vgName) ? getVG(vgName).m_pvInfo.values().toVector() : QVector<PVInfo>();
}

QVector<PVInfo> LVMInfo::getVGAllUsedPV(const QString &vgName)
{
    return getVGPVList(vgName, true);
}

QVector<PVInfo> LVMInfo::getVGAllUnUsedPV(const QString &vgName)
{
    return getVGPVList(vgName, false);
}

QList<QString> LVMInfo::getVGOfDisk(const QString &vgName, const QString &disk)
{
    QList<QString>list;
    foreach (auto it, m_pvInfo) {
        //todo 要对这里进行优化 目前是比较是否包含磁盘路径字符串
        if (it.m_pvPath.contains(disk) && it.m_vgName == vgName) {
            list.push_back(it.m_pvPath);
        }
    }
    return list;
}

QVector<PVInfo> LVMInfo::getVGPVList(const QString &vgName, bool isUsed)
{
    QVector<PVInfo> pvList;
    if (vgExists(vgName)) {
        VGInfo vg = getVG(vgName);
        foreach (const PVInfo &pv, vg.m_pvInfo) {
            if (isUsed && pv.m_pvUsedPE > 0) {
                pvList.push_back(pv);
            } else if (!isUsed && pv.m_pvUsedPE == 0) {
                pvList.push_back(pv);
            }
        }
    }
    return pvList;
}

bool LVMInfo::lvInfoExists(const QString &vgName, const QString &lvName)
{
    //判断lv是否存在
    auto it = m_vgInfo.find(vgName);
    if (m_vgInfo.end() == it) {
        return false;
    }
    return it.value().lvInfoExists(lvName);
}

bool LVMInfo::lvInfoExists(const QString &lvPath)
{
    QStringList list = lvPath.split("/");
    list.pop_front();
    if (list.count() < 3) {
        return false;
    }

    if (lvPath.contains("/dev/mapper/")) {
        QStringList list2 = list[2].split("-");
        if (!vgExists(list2[0])) {
            return false;
        }

        VGInfo vg =  getVG(list2[0]);
        foreach (LVInfo lv, vg.m_lvlist) {
            if ("/dev/mapper/" + vg.m_vgName + "-" + lv.m_lvName == lvPath) {
                return true;
            }
        }
        return false;
    }
    return lvInfoExists(list[1], list[2]);
}

bool LVMInfo::vgExists(const QString &vgName)
{
    return itemExists(vgName, m_vgInfo);
}

bool LVMInfo::pvExists(const QString &pvPath)
{
    return itemExists(pvPath, m_pvInfo);
}

bool LVMInfo::vgExists(const PVData &pv)
{
    return pvExists(pv) ? vgExists(getPV(pv).m_vgName) : false;
}

bool LVMInfo::vgExists(const PVInfo &pv)
{
    return vgExists(pv.m_vgName);
}

bool LVMInfo::pvExists(const PVData &pv)
{
    return pvExists(pv.m_devicePath);
}

bool LVMInfo::pvOfVg(const QString &vgName, const QString &pvPath)
{
    if (!vgExists(vgName)) {
        return false;
    }

    auto vg = getVG(vgName);
    return vg.m_pvInfo.find(pvPath) != vg.m_pvInfo.end();
}

bool LVMInfo::pvOfVg(const QString &vgName, const PVData &pv)
{
    return pvOfVg(vgName, pv.m_devicePath);
}

bool LVMInfo::pvOfVg(const PVInfo &pv)
{
    return pvOfVg(pv.m_vgName, pv.m_pvPath);
}

bool LVMInfo::pvOfVg(const VGInfo &vg, const PVInfo &pv)
{
    return pvOfVg(vg.m_vgName, pv.m_pvPath);
}

bool LVMInfo::pvOfVg(const VGInfo &vg, const PVData &pv)
{
    return pvOfVg(vg.m_vgName, pv.m_devicePath);
}

bool LVMInfo::pvOfVg(const QString &vgName, const PVInfo &pv)
{
    return pvOfVg(vgName, pv.m_pvPath);
}

template<class T>
T LVMInfo:: getItem(const QString &str,  const QMap<QString, T> &containers)
{
    return itemExists(str, containers) ? *containers.find(str) : T();
}

template<class T>
bool LVMInfo::itemExists(const QString &str, const QMap<QString, T> &containers)
{
    return containers.find(str) != containers.end();
}
