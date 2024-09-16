// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "diskmanagerservice.h"
#include "PolicyKitHelper.h"

#include <QCoreApplication>
#include <QDebug>

#include <unistd.h>

namespace DiskManager {

DiskManagerService::DiskManagerService(const QString &frontEndDBusName, QObject *parent)
    : QObject(parent)
    , m_partedcore(new PartedCore(this))
    , m_frontEndDBusName(frontEndDBusName)
{
    initConnection();
}

void DiskManagerService::initConnection()
{
    connect(m_partedcore, &PartedCore::updateDeviceInfo, this, &DiskManagerService::updateDeviceInfo);
    connect(m_partedcore, &PartedCore::updateLUKSInfo, this, &DiskManagerService::updateLUKSInfo);
    connect(m_partedcore, &PartedCore::deletePartitionMessage, this, &DiskManagerService::deletePartition);
    connect(m_partedcore, &PartedCore::hidePartitionInfo, this, &DiskManagerService::hidePartitionInfo);
    connect(m_partedcore, &PartedCore::showPartitionInfo, this, &DiskManagerService::showPartitionInfo);
    connect(m_partedcore, &PartedCore::usbUpdated, this, &DiskManagerService::usbUpdated);
    connect(m_partedcore, &PartedCore::checkBadBlocksCountInfo, this, &DiskManagerService::checkBadBlocksCountInfo);
    connect(m_partedcore, &PartedCore::fixBadBlocksInfo, this, &DiskManagerService::fixBadBlocksInfo);
    connect(m_partedcore, &PartedCore::checkBadBlocksFinished, this, &DiskManagerService::checkBadBlocksFinished);
    connect(m_partedcore, &PartedCore::fixBadBlocksFinished, this, &DiskManagerService::fixBadBlocksFinished);
    connect(m_partedcore, &PartedCore::unmountPartition, this, &DiskManagerService::unmountPartition);
    connect(m_partedcore, &PartedCore::createTableMessage, this, &DiskManagerService::createTableMessage);
    connect(m_partedcore, &PartedCore::vgCreateMessage, this, &DiskManagerService::vgCreateMessage);
    connect(m_partedcore, &PartedCore::pvDeleteMessage, this, &DiskManagerService::pvDeleteMessage);
    connect(m_partedcore, &PartedCore::vgDeleteMessage, this, &DiskManagerService::vgDeleteMessage);
    connect(m_partedcore, &PartedCore::lvDeleteMessage, this, &DiskManagerService::lvDeleteMessage);
    connect(m_partedcore, &PartedCore::clearMessage, this, &DiskManagerService::clearMessage);
    connect(m_partedcore, &PartedCore::deCryptMessage, this, &DiskManagerService::deCryptMessage);
    connect(m_partedcore, &PartedCore::createFailedMessage, this, &DiskManagerService::createFailedMessage);
    connect(this, &DiskManagerService::getAllDeviceInfomation, this, &DiskManagerService::onGetAllDeviceInfomation);
}

void DiskManagerService::Quit()
{
    if (!checkAuthorization())
        return;

    qDebug() << "DiskManagerService::Quit called";
    m_partedcore->delTempMountFile();
    QCoreApplication::exit(0);
}

//void DiskManagerService::Start(qint64 applicationPid)
//{
//    if (PolicyKitHelper::instance()->checkAuthorization("com.deepin.diskmanager.Start", applicationPid)) {
//       emit rootLogin("1");
//    } else {
//        emit rootLogin("0");
//        Quit();
//    }
//    QString msg = "DiskManagerService::Start called";
//    Q_EMIT MessageReport(msg);
////    qDebug() << msg;
//}
void DiskManagerService::Start()
{
    if (!checkAuthorization())
        return;

    QString msg = "DiskManagerService::Start called";
    Q_EMIT MessageReport(msg);
}

DeviceInfo DiskManagerService::getDeviceinfo()
{
    if (!checkAuthorization())
        return DeviceInfo();

    QString msg = "DiskManagerService::getDeviceinfo";
    Q_EMIT MessageReport(msg);
//    qDebug() << "DiskManagerService::getDeviceinfo success *******";
    return m_partedcore->getDeviceinfo();
}

//TODO： 这里感觉没有必要发送信号 等之后有时间测试一下 能否去掉多余的代码
void DiskManagerService::getalldevice()
{
    if (!checkAuthorization())
        return;

//    qDebug() << "DiskManagerService::getalldevice";
//    DeviceInfoMap infores = m_partedcore->getAllDeviceinfo();
//    Q_EMIT updateDeviceInfo(infores);
    emit getAllDeviceInfomation();
}

void DiskManagerService::onGetAllDeviceInfomation()
{
    DeviceInfoMap infores = m_partedcore->getAllDeviceinfo();
    LVMInfo lvmInfo = m_partedcore->getAllLVMinfo();
    LUKSMap luksInfo = m_partedcore->getAllLUKSinfo();
    Q_EMIT updateLUKSInfo(luksInfo);
    Q_EMIT updateDeviceInfo(infores, lvmInfo);
}

void DiskManagerService::setCurSelect(const PartitionInfo &info)
{
    if (!checkAuthorization())
        return;

    m_partedcore->setCurSelect(info);
}

bool DiskManagerService::unmount()
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->unmount();
}

bool DiskManagerService::mount(const QString &mountpath)
{
    if (!checkAuthorization())
        return false;

    QString invokerUid = QString::number(connection().interface()->serviceUid(message().service()).value());
    return m_partedcore->mountAndWriteFstab(mountpath, invokerUid);
}

bool DiskManagerService::deCrypt(const LUKS_INFO &luks)
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->deCrypt(luks);
}

bool DiskManagerService::cryptMount(const LUKS_INFO &luks)
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->cryptMount(luks);
}

bool DiskManagerService::cryptUmount(const LUKS_INFO &luks)
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->cryptUmount(luks);
}

QStringList DiskManagerService::getallsupportfs()
{
    if (!checkAuthorization())
        return QStringList();

    return m_partedcore->getallsupportfs();
}

bool DiskManagerService::format(const QString &fstype, const QString &name)
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->format(fstype, name);
}

bool DiskManagerService::clear(const WipeAction &wipe)
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->clear(wipe);
}

bool DiskManagerService::resize(const PartitionInfo &info)
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->resize(info);
}

bool DiskManagerService::create(const PartitionVec &infovec)
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->create(infovec);
}

HardDiskInfo DiskManagerService::onGetDeviceHardInfo(const QString &devicepath)
{
    if (!checkAuthorization())
        return HardDiskInfo();

    return m_partedcore->getDeviceHardInfo(devicepath);
}

QString DiskManagerService::onGetDeviceHardStatus(const QString &devicepath)
{
    if (!checkAuthorization())
        return QString();

    return m_partedcore->getDeviceHardStatus(devicepath);
}

HardDiskStatusInfoList DiskManagerService::onGetDeviceHardStatusInfo(const QString &devicepath)
{
    if (!checkAuthorization())
        return HardDiskStatusInfoList();

    return m_partedcore->getDeviceHardStatusInfo(devicepath);
}
bool DiskManagerService::onDeletePartition()
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->deletePartition();
}
bool DiskManagerService::onHidePartition()
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->hidePartition();
}
bool DiskManagerService::onShowPartition()
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->showPartition();
}
bool DiskManagerService::onDetectionPartitionTableError(const QString &devicePath)
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->detectionPartitionTableError(devicePath);
}
bool DiskManagerService::onCreatePartitionTable(const QString &devicePath, const QString &length, const QString &sectorSize, const QString &diskLabel)
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->createPartitionTable(devicePath, length, sectorSize, diskLabel);
}
bool DiskManagerService::onCheckBadBlocksCount(const QString &devicePath, int blockStart, int blockEnd, int checkConut, int checkSize, int flag)
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->checkBadBlocks(devicePath, blockStart, blockEnd, checkConut, checkSize, flag);
}
bool DiskManagerService::onCheckBadBlocksTime(const QString &devicePath, int blockStart, int blockEnd, const QString &checkTime, int checkSize, int flag)
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->checkBadBlocks(devicePath, blockStart, blockEnd, checkTime, checkSize, flag);
}
bool DiskManagerService::onFixBadBlocks(const QString &devicePath, QStringList badBlocksList, int checkSize, int flag)
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->fixBadBlocks(devicePath, badBlocksList, checkSize, flag);
}

bool DiskManagerService::onCreateVG(QString vgName, QList<PVData> devList, long long size)
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->createVG(vgName, devList, size);
}

bool DiskManagerService::onCreateLV(QString vgName, QList<LVAction> lvList)
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->createLV(vgName, lvList);
}

bool DiskManagerService::onDeleteVG(QStringList vglist)
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->deleteVG(vglist);
}

bool DiskManagerService::onDeleteLV(QStringList lvlist)
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->deleteLV(lvlist);
}

bool DiskManagerService::onResizeVG(QString vgName, QList<PVData> devList, long long size)
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->resizeVG(vgName, devList, size);
}

bool DiskManagerService::onResizeLV(LVAction lvAction)
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->resizeLV(lvAction);
}

bool DiskManagerService::onMountLV(LVAction lvAction)
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->mountLV(lvAction);
}

bool DiskManagerService::onUmountLV(LVAction lvAction)
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->umountLV(lvAction);
}

bool DiskManagerService::onClearLV(LVAction lvAction)
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->clearLV(lvAction);
}

bool DiskManagerService::onDeletePVList(QList<PVData> devList)
{
    if (!checkAuthorization())
        return false;

    return m_partedcore->deletePVList(devList);
}
void DiskManagerService::updateUsb()
{
    if (!checkAuthorization())
        return;

    m_partedcore->updateUsb();
}
void DiskManagerService::updateUsbRemove()
{
    if (!checkAuthorization())
        return;

    m_partedcore->updateUsbRemove();
}
void DiskManagerService::refreshFunc()
{
    if (!checkAuthorization())
        return;

    m_partedcore->refreshFunc();
}
int DiskManagerService::test()
{
    if (!checkAuthorization())
        return 0;

    return m_partedcore->test();
}

bool DiskManagerService::checkAuthorization(void)
{
    QString actionId("com.deepin.diskmanager");

    if (message().service() == m_frontEndDBusName ||
        connection().interface()->serviceUid(message().service()).value() == 0 ||
        PolicyKitHelper::instance()->checkAuthorization(actionId, message().service())) {
        return true;
    } else {
        sendErrorReply(QDBusError::AccessDenied);
        return false;
    }
}

} // namespace DiskManager
