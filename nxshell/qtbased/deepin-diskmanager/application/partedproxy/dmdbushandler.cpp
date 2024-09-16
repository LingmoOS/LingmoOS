// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "dmdbushandler.h"

#include <QObject>
#include <QDBusError>
#include <QDBusPendingCallWatcher>

DMDbusHandler *DMDbusHandler::m_staticHandeler = nullptr;

DMDbusHandler *DMDbusHandler::instance(QObject *parent)
{
    if (parent != nullptr && m_staticHandeler == nullptr)
        m_staticHandeler = new DMDbusHandler(parent);
    return m_staticHandeler;
}

DMDbusHandler::~DMDbusHandler()
{
//    Quit();
}

DMDbusHandler::DMDbusHandler(QObject *parent)
    : QObject(parent)
{
    qRegisterMetaType<DeviceInfo>("DeviceInfo");
    qRegisterMetaType<DeviceInfoMap>("DeviceInfoMap");

    qDBusRegisterMetaType<DeviceInfo>();
    qDBusRegisterMetaType<DeviceInfoMap>();
    qDBusRegisterMetaType<PartitionInfo>();
    qDBusRegisterMetaType<PartitionVec>();
    qDBusRegisterMetaType<stCustest>();
    qDBusRegisterMetaType<HardDiskInfo>();
    qDBusRegisterMetaType<HardDiskStatusInfo>();
    qDBusRegisterMetaType<HardDiskStatusInfoList>();

    //注册结构体 lvm dbus通信使用
    qDBusRegisterMetaType<PVRanges>();
    qDBusRegisterMetaType<LVAction>();
    qDBusRegisterMetaType<QVector<PVRanges>>();
    qDBusRegisterMetaType<VGDATA>();
    qDBusRegisterMetaType<PVData>();
    qDBusRegisterMetaType<LVDATA>();
    qDBusRegisterMetaType<LVInfo>();
    qDBusRegisterMetaType<VGInfo>();
    qDBusRegisterMetaType<PVInfo>();
    qDBusRegisterMetaType<LVMInfo>();
    qDBusRegisterMetaType<QVector<QString>>();
    qDBusRegisterMetaType<QList<PVData>>();
    qDBusRegisterMetaType<QList<LVAction>>();

    //注册结构体 luks dbus通信使用
    qDBusRegisterMetaType<LUKS_INFO>();
    qDBusRegisterMetaType<LUKS_MapperInfo>();
    qDBusRegisterMetaType<CRYPT_CIPHER_Support>();
    qDBusRegisterMetaType<LUKSInfoMap>();
    qDBusRegisterMetaType<LUKSMap>();
    qDBusRegisterMetaType<WipeAction>();

    m_dbus = new DMDBusInterface("com.deepin.diskmanager", "/com/deepin/diskmanager",
                                 QDBusConnection::systemBus(), this);
    //Note: when dealing with remote objects, it is not always possible to determine if it exists when creating a QDBusInterface.
    if (!m_dbus->isValid() && !m_dbus->lastError().message().isEmpty()) {
        qDebug() << "m_dbus isValid false error:" << m_dbus->lastError() << m_dbus->lastError().message();
    }
    qDebug() << "m_dbus isValid true";
    initConnection();

    startService();
}

void DMDbusHandler::initConnection()
{
    connect(m_dbus, &DMDBusInterface::MessageReport, this, &DMDbusHandler::onMessageReport);
    //  connect(m_dbus, &DMDBusInterface::sigUpdateDeviceInfo, this, &DMDbusHandler::sigUpdateDeviceInfo);
    connect(m_dbus, &DMDBusInterface::updateDeviceInfo, this, &DMDbusHandler::onUpdateDeviceInfo);
    connect(m_dbus, &DMDBusInterface::updateLUKSInfo, this, &DMDbusHandler::onUpdateLUKSInfo);
    connect(m_dbus, &DMDBusInterface::unmountPartition, this, &DMDbusHandler::onUnmountPartition);
    connect(m_dbus, &DMDBusInterface::deletePartition, this, &DMDbusHandler::onDeletePartition);
    connect(m_dbus, &DMDBusInterface::hidePartitionInfo, this, &DMDbusHandler::onHidePartition);
    connect(m_dbus, &DMDBusInterface::showPartitionInfo, this, &DMDbusHandler::onShowPartition);
    connect(m_dbus, &DMDBusInterface::createTableMessage, this, &DMDbusHandler::onCreatePartitionTable);
    connect(m_dbus, &DMDBusInterface::usbUpdated, this, &DMDbusHandler::onUpdateUsb);
    connect(m_dbus, &DMDBusInterface::checkBadBlocksCountInfo, this, &DMDbusHandler::checkBadBlocksCountInfo);
    connect(m_dbus, &DMDBusInterface::fixBadBlocksInfo, this, &DMDbusHandler::repairBadBlocksInfo);
    connect(m_dbus, &DMDBusInterface::checkBadBlocksFinished, this, &DMDbusHandler::checkBadBlocksFinished);
    connect(m_dbus, &DMDBusInterface::fixBadBlocksFinished, this, &DMDbusHandler::fixBadBlocksFinished);
    connect(m_dbus, &DMDBusInterface::clearMessage, this, &DMDbusHandler::wipeMessage);
    connect(m_dbus, &DMDBusInterface::vgCreateMessage, this, &DMDbusHandler::vgCreateMessage);
    connect(m_dbus, &DMDBusInterface::pvDeleteMessage, this, &DMDbusHandler::pvDeleteMessage);
    connect(m_dbus, &DMDBusInterface::vgDeleteMessage, this, &DMDbusHandler::vgDeleteMessage);
    connect(m_dbus, &DMDBusInterface::lvDeleteMessage, this, &DMDbusHandler::lvDeleteMessage);
    connect(m_dbus, &DMDBusInterface::deCryptMessage, this, &DMDbusHandler::deCryptMessage);
    connect(m_dbus, &DMDBusInterface::createFailedMessage, this, &DMDbusHandler::createFailedMessage);
//    connect(m_dbus, &DMDBusInterface::rootLogin, this, &DMDbusHandler::onRootLogin);
}

void DMDbusHandler::onUnmountPartition(const QString &unmountMessage)
{
    emit unmountPartitionMessage(unmountMessage);
    emit curSelectChanged();
}

void DMDbusHandler::onDeletePartition(const QString &deleteMessage)
{
    emit deletePartitionMessage(deleteMessage);
}

void DMDbusHandler::onHidePartition(const QString &hideMessage)
{
    emit hidePartitionMessage(hideMessage);
    emit curSelectChanged();
}

void DMDbusHandler::onShowPartition(const QString &showMessage)
{
    emit showPartitionMessage(showMessage);
    emit curSelectChanged();
}

void DMDbusHandler::onCreatePartitionTable(const bool &flag)
{
    emit createPartitionTableMessage(flag);
    emit curSelectChanged();
}

void DMDbusHandler::onUpdateUsb()
{
    emit updateUsb();
}

void DMDbusHandler::onSetCurSelect(const QString &devicePath, const QString &partitionPath, Sector start, Sector end, int level)
{
    //点击切换才触发
    if (DeviceType::PARTITION == level || DeviceType::DISK == level) {
        if (((level != m_curLevel) || (devicePath != m_curDevicePath) || (partitionPath != m_curPartitionPath)) && m_deviceMap.size() > 0) {
            m_curDevicePath = devicePath;
            m_curLevel = level;
            auto it = m_deviceMap.find(devicePath);
            if (it != m_deviceMap.end()) {
                for (PartitionInfo info : it.value().m_partition) {
                    if (info.m_sectorStart == start && info.m_sectorEnd == end) {
                        qDebug() << info.m_partitionNumber << info.m_path << Utils::fileSystemTypeToString(static_cast<FSType>(info.m_fileSystemType))
                                 << info.m_sectorStart << info.m_sectorEnd << info.m_sectorSize << info.m_devicePath;
                        m_curPartitionInfo = info;
                        break;
                    }
                }
            }
            m_dbus->setCurSelect(m_curPartitionInfo);
            emit curSelectChanged();
        }
    } else if (DeviceType::VOLUMEGROUP == level || DeviceType::LOGICALVOLUME == level) {
        QMap<QString, VGInfo> mapVGInfo = m_lvmInfo.m_vgInfo;
        if (((level != m_curLevel) || (devicePath != m_curVGName) || (partitionPath != m_curPartitionPath)) && mapVGInfo.size() > 0) {
            m_curLevel = level;
            m_curVGName = devicePath;
            m_curVGInfo = mapVGInfo.value(devicePath);
            if (DeviceType::LOGICALVOLUME == level) {
                for (int i = 0; i < m_curVGInfo.m_lvlist.count(); i++) {
                    LVInfo info = m_curVGInfo.m_lvlist.at(i);
                    if (partitionPath == info.m_lvPath) {
                        m_curLVInfo = info;
                        break;
                    }
                }
            }

            emit curSelectChanged();
        }
    }
}

void DMDbusHandler::startService()
{
    m_dbus->Start();
}

void DMDbusHandler::Quit()
{
    m_dbus->Quit();
}

void DMDbusHandler::refresh()
{
    emit showSpinerWindow(true, tr("Refreshing data..."));

    m_dbus->refreshFunc();
}

void DMDbusHandler::onRootLogin(const QString &loginMessage)
{
    m_loginMessage = loginMessage;

    emit rootLogin();
}

QString DMDbusHandler::getRootLoginResult()
{
    return m_loginMessage;
}

void DMDbusHandler::getDeviceInfo()
{
    emit showSpinerWindow(true, tr("Initializing data..."));

    m_dbus->getalldevice();
    qDebug() << __FUNCTION__ << "-------";
}

DeviceInfoMap &DMDbusHandler::probDeviceInfo()
{
    QFile file("/etc/mtab");
    QString mounts;
    if (file.open(QIODevice::ReadOnly)) {
        mounts = file.readAll();
        file.close();
    }
    if (!mounts.isEmpty()) {
        for (auto &disk : m_deviceMap) {
            for (auto &partition : disk.m_partition) {
                if (!mounts.contains(partition.m_path)) {
                    partition.m_mountPoints.clear();
                }
            }
        }
    }
    return m_deviceMap;
}

PartitionVec DMDbusHandler::getCurDevicePartitionArr()
{
    return m_deviceMap.find(m_curDevicePath).value().m_partition;
}

const PartitionInfo &DMDbusHandler::getCurPartititonInfo()
{
    return m_curPartitionInfo;
}

const DeviceInfo &DMDbusHandler::getCurDeviceInfo()
{
    return m_deviceMap.find(m_curDevicePath).value();
}

const Sector &DMDbusHandler::getCurDeviceInfoLength()
{
    return m_deviceMap.find(m_curDevicePath).value().m_length;
}

void DMDbusHandler::mount(const QString &mountPath)
{
    emit showSpinerWindow(true, tr("Mounting %1 ...").arg(m_curPartitionInfo.m_path));

    m_dbus->mount(mountPath);
}

void DMDbusHandler::unmount()
{
    emit showSpinerWindow(true, tr("Unmounting %1 ...").arg(m_curPartitionInfo.m_path));

    m_dbus->unmount();
}

QStringList DMDbusHandler::getAllSupportFileSystem()
{
    if (m_supportFileSystem.size() <= 0) {
        QDBusPendingReply<QStringList> reply = m_dbus->getallsupportfs();

        reply.waitForFinished();

        if (reply.isError()) {
            qDebug() << reply.error().message();
        } else {
            m_supportFileSystem = reply.value();
        }
    }

    return m_supportFileSystem;
}

void DMDbusHandler::format(const QString &fstype, const QString &name)
{
    //   bool success = false;
    //    QDBusPendingReply<bool> reply = m_dbus->format(fstype, name);
    //    reply.waitForFinished();
    //    if (reply.isError()) {
    //        qDebug() << reply.error().message();
    //    } else {
    //        success = reply.value();
    //    }
    emit showSpinerWindow(true);

    m_dbus->format(fstype, name);
}

void DMDbusHandler::resize(const PartitionInfo &info)
{
    emit showSpinerWindow(true, tr("Resizing %1 ...").arg(m_curPartitionInfo.m_path));

    m_dbus->resize(info);
}

void DMDbusHandler::create(const PartitionVec &infovec)
{
    emit showSpinerWindow(true, tr("Creating a new partition..."));

    m_dbus->create(infovec);
}

void DMDbusHandler::onMessageReport(const QString &msg)
{
    qDebug() << "MessageReport:" << msg;
}

void DMDbusHandler::onUpdateDeviceInfo(const DeviceInfoMap &infoMap, const LVMInfo &lvmInfo)
{
    m_deviceMap = infoMap;
    m_lvmInfo = lvmInfo;
    m_isExistUnallocated.clear();
    m_deviceNameList.clear();
    m_vgNameList.clear();

    for (auto it = infoMap.begin(); it != infoMap.end(); it++) {
        DeviceInfo info = it.value();
//        qDebug() << __FUNCTION__ << info.m_path << info.m_length << info.m_heads << info.m_sectors
//                 << info.m_cylinders << info.m_cylsize << info.m_model << info.m_serialNumber << info.m_disktype
//                 << info.m_sectorSize << info.m_maxPrims << info.m_highestBusy << info.m_readonly
//                 << info.m_maxPartitionNameLength << info.m_mediaType << info.m_interface << info.m_vgFlag;

        m_cryptSupport = info.m_crySupport;
        if (info.m_path.isEmpty() || info.m_path.contains("/dev/mapper")) {
            continue;
        }

        m_deviceNameList << info.m_path;
        QString isExistUnallocated = "false";
        QString isJoinAllVG = "true";
        QString isAllEncryption = "true";
        for (auto it = info.m_partition.begin(); it != info.m_partition.end(); it++) {
//                    qDebug() << it->m_path << it->m_devicePath << it->m_partitionNumber << it->m_sectorsUsed << it->m_sectorsUnused << it->m_sectorStart << it->m_sectorEnd;
            if (it->m_path == "unallocated") {
                isExistUnallocated = "true";
            }

            if (it->m_vgFlag == LVMFlag::LVM_FLAG_NOT_PV) {
                isJoinAllVG = "false";
            }

            if (it->m_luksFlag == LUKSFlag::NOT_CRYPT_LUKS) {
                isAllEncryption = "false";
            }
        }

        if (("unrecognized" == info.m_disktype || "none" == info.m_disktype) && (info.m_vgFlag != LVMFlag::LVM_FLAG_NOT_PV)) {
            isJoinAllVG = "true";
        }

        // 处理磁盘无分区时，磁盘图标显示错误
        if (info.m_partition.size() == 0 && info.m_vgFlag == LVMFlag::LVM_FLAG_NOT_PV) {
            isJoinAllVG = "false";
        }

        if (info.m_partition.size() == 0 && info.m_luksFlag == LUKSFlag::NOT_CRYPT_LUKS) {
            isAllEncryption = "false";
        }

        m_isExistUnallocated[info.m_path] = isExistUnallocated;
        m_isJoinAllVG[info.m_path] = isJoinAllVG;
        m_isAllEncryption[info.m_path] = isAllEncryption;
    }

    QMap<QString, VGInfo> lstVGInfo = lvmInfo.m_vgInfo;
    for (auto vgInfo = lstVGInfo.begin(); vgInfo != lstVGInfo.end(); vgInfo++) {
        VGInfo vgInformation = vgInfo.value();

        if (vgInformation.m_vgName.isEmpty()) {
            continue;
        }

        m_vgNameList << vgInformation.m_vgName;
        QString isExistUnallocated = "false";
        QString isAllEncryption = "true";
        for (int i = 0; i < vgInformation.m_lvlist.count(); i++) {
            LVInfo info = vgInformation.m_lvlist.at(i);
            if (info.m_lvName.isEmpty() && info.m_lvUuid.isEmpty()) {
                isExistUnallocated = "true";
            }

            if (info.m_luksFlag == LUKSFlag::NOT_CRYPT_LUKS) {
                isAllEncryption = "false";
            }
        }

        m_isExistUnallocated[vgInformation.m_vgName] = isExistUnallocated;
        m_isAllEncryption[vgInformation.m_vgName] = isAllEncryption;
    }

    emit updateDeviceInfo();
    emit showSpinerWindow(false, "");
}

void DMDbusHandler::onUpdateLUKSInfo(const LUKSMap &infomap)
{
    m_curLUKSInfoMap = infomap;
}

QMap<QString, QString> DMDbusHandler::getIsExistUnallocated()
{
    return m_isExistUnallocated;
}

QStringList DMDbusHandler::getDeviceNameList()
{
    return m_deviceNameList;
}

HardDiskInfo DMDbusHandler::getHardDiskInfo(const QString &devicePath)
{
    QDBusPendingReply<HardDiskInfo> reply = m_dbus->onGetDeviceHardInfo(devicePath);

    reply.waitForFinished();

    if (reply.isError()) {
        qDebug() << reply.error().message();
    } else {
        m_hardDiskInfo = reply.value();
    }

    return m_hardDiskInfo;
}

QString DMDbusHandler::getDeviceHardStatus(const QString &devicePath)
{
    QDBusPendingReply<QString> reply = m_dbus->onGetDeviceHardStatus(devicePath);

    reply.waitForFinished();

    if (reply.isError()) {
        qDebug() << reply.error().message();
    } else {
        m_deviceHardStatus = reply.value();
    }

    return m_deviceHardStatus;
}

HardDiskStatusInfoList DMDbusHandler::getDeviceHardStatusInfo(const QString &devicePath)
{
    QDBusPendingReply<HardDiskStatusInfoList> reply = m_dbus->onGetDeviceHardStatusInfo(devicePath);

    reply.waitForFinished();

    if (reply.isError()) {
        qDebug() << reply.error().message();
    } else {
        m_hardDiskStatusInfoList = reply.value();
    }

    return m_hardDiskStatusInfoList;
}

void DMDbusHandler::deletePartition()
{
    emit showSpinerWindow(true, tr("Deleting %1 ...").arg(m_curPartitionInfo.m_path));

    m_dbus->onDeletePartition();
}

void DMDbusHandler::hidePartition()
{
    emit showSpinerWindow(true);

    m_dbus->onHidePartition();
}

void DMDbusHandler::unhidePartition()
{
    emit showSpinerWindow(true);

    m_dbus->onShowPartition();
}

bool DMDbusHandler::detectionPartitionTableError(const QString &devicePath)
{
    QDBusPendingReply<bool> reply = m_dbus->onDetectionPartitionTableError(devicePath);

    reply.waitForFinished();

    if (reply.isError()) {
        qDebug() << reply.error().message();
    } else {
        m_partitionTableError = reply.value();
    }

    return m_partitionTableError;
}

void DMDbusHandler::checkBadSectors(const QString &devicePath, int blockStart, int blockEnd, int checkNumber, int checkSize, int flag)
{
    if (checkNumber > 16) {
        m_dbus->onCheckBadBlocksTime(devicePath, blockStart, blockEnd, QString::number(checkNumber), checkSize, flag);
    } else {
        m_dbus->onCheckBadBlocksCount(devicePath, blockStart, blockEnd, checkNumber, checkSize, flag);
    }

}

void DMDbusHandler::repairBadBlocks(const QString &devicePath, QStringList badBlocksList, int repairSize, int flag)
{
    m_dbus->onFixBadBlocks(devicePath, badBlocksList, repairSize, flag);
}

void DMDbusHandler::createPartitionTable(const QString &devicePath, const QString &length, const QString &sectorSize, const QString &diskLabel, const QString &curType)
{
    if (curType == "create") {
        emit showSpinerWindow(true, tr("Creating a partition table of %1 ...").arg(devicePath));
    } else {
        emit showSpinerWindow(true, tr("Replacing the partition table of %1 ...").arg(devicePath));
    }
    m_curCreateType = curType;

    m_dbus->onCreatePartitionTable(devicePath, length, sectorSize, diskLabel);
}

QString DMDbusHandler::getCurCreatePartitionTableType()
{
    return m_curCreateType;
}

int DMDbusHandler::getCurLevel()
{
    return m_curLevel;
}

QString DMDbusHandler::getCurDevicePath()
{
    return m_curDevicePath;
}

void DMDbusHandler::clear(const WipeAction &wipe)
{
    m_dbus->clear(wipe);
}

const LVMInfo &DMDbusHandler::probLVMInfo() const
{
    return m_lvmInfo;
}

const VGInfo &DMDbusHandler::getCurVGInfo()
{
    return m_curVGInfo;
}

const LVInfo &DMDbusHandler::getCurLVInfo()
{
    return m_curLVInfo;
}

QStringList DMDbusHandler::getVGNameList()
{
    return m_vgNameList;
}

QString DMDbusHandler::getCurVGName()
{
    return m_curVGName;
}

QMap<QString, QString> DMDbusHandler::getIsJoinAllVG()
{
    return m_isJoinAllVG;
}

bool DMDbusHandler::isExistMountLV(const VGInfo &info)
{
    for (int i = 0; i < info.m_lvlist.count(); i++) {
        if (lvIsMount(info.m_lvlist.at(i))) {
            return true;
        }
    }

    return false;
}

bool DMDbusHandler::lvIsMount(const LVInfo &lvInfo)
{
    if (lvInfo.m_luksFlag == LUKSFlag::IS_CRYPT_LUKS) {
        LUKS_INFO luksInfo = probLUKSInfo().m_luksMap.value(lvInfo.m_lvPath);
        if (luksInfo.isDecrypt) {
            if (!luksInfo.m_mapper.m_mountPoints.isEmpty()) {
                return true;
            }
        }
    } else {
        if (!lvInfo.m_mountPoints.isEmpty()) {
            return true;
        }
    }

    return false;
}

bool DMDbusHandler::isExistMountPartition(const DeviceInfo &info)
{
    for (int i = 0; i < info.m_partition.size(); i++) {
        if (partitionISMount(info.m_partition.at(i))) {
            return true;
        }
    }

    return false;
}

bool DMDbusHandler::partitionISMount(const PartitionInfo &info)
{
    if (info.m_luksFlag == LUKSFlag::IS_CRYPT_LUKS) {
        LUKS_INFO luksInfo = probLUKSInfo().m_luksMap.value(info.m_path);
        if (luksInfo.isDecrypt) {
            if (!luksInfo.m_mapper.m_mountPoints.isEmpty()) {
                return true;
            }
        }
    } else {
        if (!info.m_mountPoints.isEmpty()) {
            return true;
        }
    }

    return false;
}

void DMDbusHandler::createVG(const QString &vgName, const QList<PVData> &devList, const long long &size)
{
    m_dbus->onCreateVG(vgName, devList, size);
}

void DMDbusHandler::createLV(const QString &vgName, const QList<LVAction> &lvList)
{
    emit showSpinerWindow(true, tr("Creating..."));

    m_dbus->onCreateLV(vgName, lvList);
}

void DMDbusHandler::deleteLV(const QStringList &lvlist)
{
    emit showSpinerWindow(true, tr("Deleting..."));

    m_dbus->onDeleteLV(lvlist);
}

void DMDbusHandler::deleteVG(const QStringList &vglist)
{
    emit showSpinerWindow(true, tr("Deleting..."));

    m_dbus->onDeleteVG(vglist);
}

void DMDbusHandler::resizeVG(const QString &vgName, const QList<PVData> &devList, const long long &size)
{
    m_dbus->onResizeVG(vgName, devList, size);
}

void DMDbusHandler::resizeLV(LVAction act)
{
    emit showSpinerWindow(true, tr("Resizing space..."));

    m_dbus->onResizeLV(act);
}

void DMDbusHandler::onMountLV(LVAction act)
{
    emit showSpinerWindow(true, tr("Mounting %1 ...").arg(m_curLVInfo.m_lvName));

    m_dbus->onMountLV(act);
}

void DMDbusHandler::onUmountLV(LVAction act)
{
    emit showSpinerWindow(true, tr("Unmounting %1 ...").arg(m_curLVInfo.m_lvName));

    m_dbus->onUmountLV(act);
}

void DMDbusHandler::onClearLV(LVAction act)
{
    m_dbus->onClearLV(act);
}

void DMDbusHandler::onDeletePVList(QList<PVData> devList)
{
    m_dbus->onDeletePVList(devList);
}

QStringList DMDbusHandler::getEncryptionFormate(const QStringList &formateList)
{
    bool isSupportAES = m_cryptSupport.supportEncrypt(m_cryptSupport.aes_xts_plain64);
    bool isSupportSM4 = m_cryptSupport.supportEncrypt(m_cryptSupport.sm4_xts_plain64);

    QStringList lstFormate;
    for (int i = 0; i < formateList.count(); ++i) {
        lstFormate << formateList.at(i);
        if (isSupportAES) {
            lstFormate << QString("%1 (%2)").arg(formateList.at(i)).arg(tr("AES Encryption"));
            if (isSupportSM4) {
                lstFormate << QString("%1 (%2)").arg(formateList.at(i)).arg(tr("SM4 Encryption"));
            }
        }
    }

    return lstFormate;
}

const LUKSMap &DMDbusHandler::probLUKSInfo() const
{
    return m_curLUKSInfoMap;
}

void DMDbusHandler::updateLUKSInfo(const QString &devPath, const LUKS_INFO &luks)
{
    m_curLUKSInfoMap.m_luksMap[devPath] = luks;
}

bool DMDbusHandler::getIsSystemDisk(const QString &devName)
{
    if (m_curLevel == DeviceType::DISK || m_curLevel == DeviceType::PARTITION) {
        DeviceInfo devInfo = m_deviceMap.value(devName);
        for (int i = 0; i < devInfo.m_partition.size(); ++i) {
            PartitionInfo partitionInfo = devInfo.m_partition.at(i);
            if (partitionInfo.m_flag == 4) {
                return true;
            }

            for (int j = 0; j < partitionInfo.m_mountPoints.size(); ++j) {
                if (partitionInfo.m_mountPoints[j] == "/boot/efi" || partitionInfo.m_mountPoints[j] == "/"
                        || partitionInfo.m_mountPoints[j] == "/recovery" || partitionInfo.m_mountPoints[j] == "/boot") {
                    return true;
                }
            }
        }
    } else if (m_curLevel == DeviceType::VOLUMEGROUP || m_curLevel == DeviceType::LOGICALVOLUME) {
        VGInfo vgInfo = m_lvmInfo.m_vgInfo.value(devName);
        for (int i = 0; i < vgInfo.m_lvlist.size(); ++i) {
            LVInfo lvInfo = vgInfo.m_lvlist.at(i);
            for (int j = 0; j < lvInfo.m_mountPoints.size(); ++j) {
                if (lvInfo.m_mountPoints[j] == "/boot/efi" || lvInfo.m_mountPoints[j] == "/"
                        || lvInfo.m_mountPoints[j] == "/recovery" || lvInfo.m_mountPoints[j] == "/boot") {
                    return true;
                }
            }
        }
    }

    return false;
}

void DMDbusHandler::deCrypt(const LUKS_INFO &luks)
{
    m_dbus->deCrypt(luks);
}

void DMDbusHandler::cryptMount(const LUKS_INFO &luks, const QString &devName)
{
    emit showSpinerWindow(true, tr("Mounting %1 ...").arg(devName));

    m_dbus->cryptMount(luks);
}

void DMDbusHandler::cryptUmount(const LUKS_INFO &luks, const QString &devName)
{
    emit showSpinerWindow(true, tr("Unmounting %1 ...").arg(devName));

    m_dbus->cryptUmount(luks);
}

QMap<QString, QString> DMDbusHandler::getIsAllEncryption()
{
    return m_isAllEncryption;
}

void DMDbusHandler::refreshMainWindowData()
{
    emit curSelectChanged();
}

QString DMDbusHandler::getFailedMessage(const QString &key, const int &value, const QString &devPath)
{
    QString failedMessage = "";
    if (key == "CRYPTError") {
        switch (value) {
        case CRYPTError::CRYPT_ERR_ENCRYPT_FAILED: {
            failedMessage = tr("Failed to encrypt %1, please try again!").arg(devPath);
            break;
        }
        case CRYPTError::CRYPT_ERR_DECRYPT_FAILED: {
            failedMessage = tr("Failed to decrypt %1, please try again!").arg(devPath);
            break;
        }
        case CRYPTError::CRYPT_ERR_CLOSE_FAILED: {
            failedMessage = tr("%1 failed to close the crypto map").arg(devPath);
            break;
        }
        }
    } else if (key == "DISK_ERROR") {
        switch (value) {
        case DISK_ERROR::DISK_ERR_CREATE_PART_FAILED: {
            failedMessage = tr("Failed to create partitions, please try again!");
            break;
        }
        case DISK_ERROR::DISK_ERR_CREATE_FS_FAILED: {
            failedMessage = tr("Failed to create %1 file system, please try again!").arg(devPath);
            break;
        }
        case DISK_ERROR::DISK_ERR_UPDATE_KERNEL_FAILED: {
            failedMessage = tr("Failed to submit the request to the kernel");
            break;
        }
        case DISK_ERROR::DISK_ERR_DBUS_ARGUMENT: {
            failedMessage = tr("DBUS parameter error");
            break;
        }
        case DISK_ERROR::DISK_ERR_MOUNT_FAILED: {
            failedMessage = tr("Failed to mount %1").arg(devPath);
            break;
        }
        case DISK_ERROR::DISK_ERR_CREATE_MOUNTDIR_FAILED: {
            failedMessage = tr("%1 failed to create mounting folders").arg(devPath);
            break;
        }
        case DISK_ERROR::DISK_ERR_CHOWN_FAILED: {
            failedMessage = tr("%1 failed to change the owner of mounting folders").arg(devPath);
            break;
        }
        case DISK_ERROR::DISK_ERR_CREATE_PARTTAB_FAILED: {
            failedMessage = tr("Creating partition table failed");
            break;
        }
        }
    } else if (key == "LVMError") {
        switch (value) {
        case LVMError::LVM_ERR_LV_CREATE_FAILED: {
            failedMessage = tr("Failed to create a logical volume, please try again!");
            break;
        }
        case LVMError::LVM_ERR_LV_CREATE_FS_FAILED: {
            failedMessage = tr("Failed to create %1 file system, please try again!").arg(devPath);
            break;
        }
        case LVMError::LVM_ERR_LV_ARGUMENT: {
            failedMessage = tr("DBUS parameter error");
            break;
        }
        }
    }

    return failedMessage;
}

QString DMDbusHandler::getEncryptionFsType(const LUKS_INFO &luksInfo)
{
    if (luksInfo.isDecrypt) {
        if (luksInfo.m_crypt == CRYPT_CIPHER::AES_XTS_PLAIN64) {
            return QString("%1 (%2)").arg(Utils::fileSystemTypeToString(luksInfo.m_mapper.m_luksFs)).arg(tr("AES Encryption"));
        } else if (luksInfo.m_crypt == CRYPT_CIPHER::SM4_XTS_PLAIN64){
            return QString("%1 (%2)").arg(Utils::fileSystemTypeToString(luksInfo.m_mapper.m_luksFs)).arg(tr("SM4 Encryption"));
        }
    }

    return Utils::fileSystemTypeToString(FSType::FS_LUKS);
}

