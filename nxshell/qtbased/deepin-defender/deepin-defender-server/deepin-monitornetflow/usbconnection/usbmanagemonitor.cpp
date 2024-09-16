// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "usbmanagemonitor.h"
#include "../localcache/settingssql.h"
#include "../localcache/usbconnlogsql.h"
#include "../localcache/usbwhitelistsql.h"

#include <QDebug>
#include <QCoreApplication>
#include <QFile>
#include <QProcess>
#include <QThread>
#include <QDir>
#include <QDateTime>

#include <libudev.h>

#include <stdlib.h> // exit(3)
#include <fstream>

#include <stdio.h> // printf(3)
#include <unistd.h> // fork(3), chdir(3), sysconf(3)
#include <signal.h> // signal(3)
#include <sys/stat.h> // umask(3)
#include <syslog.h> // syslog(3), openlog(3), closelog(3)
#include <string.h> // strncmp
#include <aio.h> // async I/O
#include <sys/types.h> // aiocb datatype
#include <errno.h> // EINPROGRESS
#include <fcntl.h> // open() file descriptor
#include <string.h> // strsep(3)
#include <proc/readproc.h>

// 设备识别关键字
#define DEV_ACTION_ADD "add" // 设备行为关键字
#define DEV_TYPE_DISK "disk" // 设备类型关键字-磁盘
#define DEV_TYPE_PARTITION "partition" // 设备类型关键字-分区
#define CD_ROM_DEV_NODE_HEAD "/dev/sr" // 光驱设备节点开头字符

// 设备信息关键字
#define UDEVADM_INFO_KEY_DEV_PATH "E: DEVPATH=" // 设备信息-键字符-设备路径
#define UDEVADM_INFO_KEY_DEV_NAME "E: DEVNAME=" // 设备信息-键字符-设备名
#define UDEVADM_INFO_KEY_DEV_TYPE "E: DEVTYPE=" // 设备信息-键字符-设备类型
#define UDEVADM_INFO_KEY_DEV_PRODUCT "E: ID_MODEL=" // 设备信息-键字符-型号
#define UDEVADM_INFO_KEY_DEV_VENDOR_ID "E: ID_VENDOR_ID=" // 设备信息-键字符-设备经销商id
#define UDEVADM_INFO_KEY_DEV_PRODUCT_ID "E: ID_MODEL_ID=" // 设备信息-键字符-设备制造商id
#define UDEVADM_INFO_KEY_DEV_SERIAL "E: ID_SERIAL_SHORT=" // 设备信息-键字符-设备序列号
#define UDEVADM_INFO_KEY_DEV_MTP "E: ID_MTP_DEVICE=" // 设备信息-键字符-设备mtp信息
#define UDEVADM_INFO_KEY_DEV_PTP "E: GPHOTO2_DRIVER=PTP" // 设备信息-键字符-设备ptp信息

// 设备连接信息关键字
#define CONNECTION_STATUS_CONN "Connected" // 设备管控状态-已连接
#define CONNECTION_STATUS_BLOCK "Blocked" // 设备管控状态-已限制
#define USB_CONN_TIME_FORMATION "yyyy-MM-dd hh:mm:ss" // 时间爱你格式化规则

UsbManageMonitor::UsbManageMonitor(QObject *parent)
    : QObject(parent)
    , m_udev(nullptr)
    , m_sLimitModel("0")
    , m_settingsSql(new SettingsSql("UsbManageMonitor_Settings", this))
    , m_usbConnLogSql(new UsbConnLogSql("UsbManageMonitor_Log", this))
    , m_usbWhitelistSql(new UsbWhitelistSql("UsbManageMonitor_Whitelist", this))
    , m_whiteList()
{
    loadConfig();
    readConnectionWhiteLst();
    readConnectionRecord();
    //清除超过30天记录
    cleanUsbConnectionLog();

    m_status = 1;

    preManage();
}

UsbManageMonitor::~UsbManageMonitor()
{
    this->quit();
    this->deleteLater();

    this->thread()->quit();
    connect(this->thread(), &QThread::finished, this->thread(), &QThread::deleteLater);
}

void UsbManageMonitor::init()
{
}

void UsbManageMonitor::run()
{
    // 首先调用udev_new，创建一个udev library context。
    // udev library context采用引用记数机制，创建的context默认引用记数为1，
    // 使用udev_ref和udev_unref增加或减少引用记数，如果引用记数为0，则释放内部资源
    m_udev = udev_new();
    if (!m_udev) {
        qDebug() << "Can't create udev.";
    }

    // 监控设备插拔 udev的设备插拔基于netlink实现
    monitorDevices(m_udev);
}

void UsbManageMonitor::quit()
{
    m_status = 0;
}

void UsbManageMonitor::loadConfig()
{
    m_isbSaveRecord = m_settingsSql->getValue(USB_STORAGE_CONN_LOG_SWITCH).toBool();
    m_sLimitModel = m_settingsSql->getValue(USB_STORAGE_LIMIT_MODE).toString();
}

void UsbManageMonitor::readConnectionRecord()
{
    m_usbConnectionInfoLst = m_usbConnLogSql->readConnectionRecord();
}

void UsbManageMonitor::readConnectionWhiteLst()
{
    m_whiteList = m_usbWhitelistSql->readConnectionWhiteLst();
}

void UsbManageMonitor::disconnectAllDevices()
{
    QProcess *proc = new QProcess;
    QString cmd = "lsblk -nro TYPE,PATH,MOUNTPOINT,HOTPLUG";
    proc->start(cmd);
    proc->waitForFinished();

    QString allInfoStr = proc->readAllStandardOutput().trimmed();

    QList<BlkInfo> blkInfoList = {};
    QStringList infoFirstCut = allInfoStr.split("\n");
    //添加结束辅助分区信息，用于管控最后一个分区
    infoFirstCut.append("disk 0 0 0");
    QString parentDevPath = "";
    bool isbHaveSysMountPoint = false;
    QString sCanRemove = "0";
    for (int i = 0; i < infoFirstCut.count(); i++) {
        QStringList infoSecondCut = infoFirstCut.at(i).split(" ");
        // 获取设备类型-"disk" "part"
        QString type = infoSecondCut.at(0);
        // 如果是硬盘则获取主设备dev节点和是否可移除属性
        if ("disk" == type) {
            if (!parentDevPath.isEmpty() && !isbHaveSysMountPoint && "1" == sCanRemove) {
                // 如果该设备分区中没有挂载系统路径并且可移除，则卸载并弹出
                for (int i = 0; i < blkInfoList.count(); i++) {
                    QString cmd = "sudo udisksctl unmount -b " + blkInfoList.at(i).devPath;
                    proc->start(cmd);
                    proc->waitForFinished();
                }

                // 判断当前设备信息列表是否为空
                if (!blkInfoList.isEmpty()) {
                    QString cmd = "sudo udisksctl power-off -b " + blkInfoList.last().parentDevPath;
                    proc->start(cmd);
                    proc->waitForFinished();
                }
            }

            blkInfoList.clear();
            isbHaveSysMountPoint = false;
            parentDevPath = infoSecondCut.at(1);
            sCanRemove = infoSecondCut.at(3);
        }

        if (4 != infoSecondCut.count())
            continue;

        // 获取分区dev节点
        QString devPath = infoSecondCut.at(1);
        //将分区信息存入列表
        BlkInfo blkInfo = {};
        blkInfo.type = type;
        // 获取分区dev节点
        blkInfo.devPath = infoSecondCut.at(1);
        // 获取主设备节点
        blkInfo.parentDevPath = parentDevPath;
        // 获取挂载节点
        blkInfo.mountPoint = infoSecondCut.at(2);
        // 判断该设备分区挂载路径中是否存在系统路径
        if (!blkInfo.mountPoint.startsWith("/media/") && !blkInfo.mountPoint.isEmpty()) {
            // 不在/media目录 和 非空，判定为系统路径
            isbHaveSysMountPoint = true;
        }

        // 获取是否可移除属性，与主设备保持一致
        blkInfo.canRemove = sCanRemove;
        blkInfoList.append(blkInfo);
    }

    proc->deleteLater();
}

void UsbManageMonitor::preManage()
{
    // 判断是否处于授权管理激活审核模式
    if (isActivatorInVerifingModel()) {
        return;
    }

    // 如果处于允许所有usb设备连接模式，则不进行u盘弹出操作
    if (USB_LIMIT_MODEL_ALL == m_sLimitModel) {
        return;
    }

    disconnectAllDevices();
}

// 判断是否处于授权管理激活审核模式
bool UsbManageMonitor::isActivatorInVerifingModel()
{
    bool ret = false;
    // 激活审核模式路径
    QString verifingModelFilePath = QString("/deepin-installer/lightdm-stop.sh");
    QFile f(verifingModelFilePath);

    // 如果存在，则为审核模式
    ret = f.exists();
    return ret;
}

void UsbManageMonitor::writeConnectionRecord(const QString &name, const QString &sDatetime, const QString &status,
                                             const QString &serial, const QString &vendorID, const QString &productID,
                                             const QString &sParentSize, const QString &sSize)
{
    // 如果记录开关未打开, 则不写入记录
    if (!m_isbSaveRecord) {
        return;
    }

    m_usbConnLogSql->writeConnectionRecord(name, sDatetime, status, serial, vendorID, productID, sParentSize, sSize);

    UsbConnectionInfo info;
    info.sName = name;
    info.sDatetime = sDatetime;
    info.sStatus = status;
    info.sSerial = serial;
    info.sVendorID = vendorID;
    info.sProductID = productID;
    info.sParentSize = sParentSize;
    info.sSize = sSize;
    m_usbConnectionInfoLst.append(info);
}

void UsbManageMonitor::clearUsbConnectionLog()
{
    m_usbConnLogSql->clearUsbConnectionLog();
    m_usbConnectionInfoLst.clear();
}

void UsbManageMonitor::cleanUsbConnectionLog()
{
    m_usbConnLogSql->cleanUsbConnectionLog();
    m_usbConnectionInfoLst = m_usbConnLogSql->readConnectionRecord();
}

void UsbManageMonitor::writeConnectionWhiteLst(const QString &name, const QString &sDatetime, const QString &status,
                                               const QString &serial, const QString &vendorID, const QString &productID,
                                               const QString &sParentSize, const QString &sSize)
{
    m_usbWhitelistSql->writeConnectionWhiteLst(name, sDatetime, status, serial, vendorID, productID, sParentSize, sSize);
    UsbConnectionInfo info;
    info.sName = name;
    info.sDatetime = sDatetime;
    info.sStatus = status;
    info.sSerial = serial;
    info.sVendorID = vendorID;
    info.sProductID = productID;
    info.sParentSize = sParentSize;
    info.sSize = sSize;
    m_whiteList.append(info);
}

void UsbManageMonitor::removeFromWhiteLst(const QString &sSerial, const QString &sVendorID, const QString &sProductID)
{
    m_usbWhitelistSql->removeFromWhiteLst(sSerial, sVendorID, sProductID);

    for (int i = 0; i < m_whiteList.size(); i++) {
        UsbConnectionInfo info = m_whiteList.at(i);
        if (sSerial == info.sSerial && sVendorID == info.sVendorID && sProductID == info.sProductID) {
            m_whiteList.removeAt(i);
            break;
        }
    }
}

void UsbManageMonitor::changeIsbSaveRecord(bool change)
{
    m_isbSaveRecord = change;
    m_settingsSql->setValue(USB_STORAGE_CONN_LOG_SWITCH, change);
}

void UsbManageMonitor::selectLimitationModel(const QString &modelStr)
{
    m_sLimitModel = modelStr;
    m_settingsSql->setValue(USB_STORAGE_LIMIT_MODE, modelStr);
}

QString UsbManageMonitor::getUsbConnectionRecords()
{
    QString sConnectionRecords;
    for (UsbConnectionInfo info : m_usbConnectionInfoLst) {
        sConnectionRecords += info.sName + (",") + info.sDatetime + (",") + info.sStatus + (",") + info.sSerial + (",") + info.sVendorID + (",") + info.sProductID + (",") + info.sParentSize + (",") + info.sSize + ("\n");
    }
    return sConnectionRecords;
}

QString UsbManageMonitor::getWhiteList()
{
    QString sWhiteList;
    for (UsbConnectionInfo info : m_whiteList) {
        sWhiteList += info.sName + (",") + info.sDatetime + (",") + info.sStatus + (",") + info.sSerial + (",") + info.sVendorID + (",") + info.sProductID + (",") + info.sParentSize + (",") + info.sSize + ("\n");
    }
    return sWhiteList;
}

void UsbManageMonitor::monitorDevices(struct udev *udev)
{
    // 使用udev_monitor_new_from_netlink创建一个新的monitor，
    // 函数的第二个参数是事件源的名称，可选"kernel"或"udev"。
    // 基于"kernel"的事件通知要早于"udev"，但相关的设备结点未必创建完成，
    // 所以一般应用的设计要基于"udev"进行监控
    struct udev_monitor *mon = udev_monitor_new_from_netlink(udev, "udev");

    // 使用udev_monitor_filter_add_match_subsystem_devtype增加一个基于设备类型的udev事件过滤器
    udev_monitor_filter_add_match_subsystem_devtype(mon, /*SUBSYSTEM*/ "block", nullptr);
    udev_monitor_filter_add_match_subsystem_devtype(mon, /*SUBSYSTEM*/ "usb", nullptr);
    // 使用udev_monitor_enable_receiving启动监控过程
    udev_monitor_enable_receiving(mon);

    // 监控可以使用udev_monitor_get_fd获取一个文件描述符，基于返回的fd可以执行poll操作，简化程序设计
    int fd = udev_monitor_get_fd(mon);

    while (m_status) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        struct timeval tv; //store timeout
        tv.tv_sec = 0;
        tv.tv_usec = 50000;

        // Wait for monitor to return
        int ret = select(fd + 1, &fds, nullptr, nullptr, &tv);
        if (ret > 0) {
            if (FD_ISSET(fd, &fds)) {
                m_mutex.lock();
                // 插拔事件到达后，可以使用udev_monitor_receive_device获取产生事件的设备映射
                struct udev_device *dev = udev_monitor_receive_device(mon);
                processDevice(dev);
                m_mutex.unlock();
            }
        }
    }
}

void UsbManageMonitor::processDevice(struct udev_device *dev)
{
    //设备行为
    const char *action = udev_device_get_action(dev);
    //设备类型
    const char *devType = udev_device_get_devtype(dev);
    //设备节点
    const char *devNode = udev_device_get_devnode(dev);

    if (DEV_ACTION_ADD != QString(action)) {
        return;
    }

    UsbDeviceInfo info;
    if (!getDevInfoFromDevNode(info, devNode)) {
        return;
    }

    // mtp、ptp管控
    if (info.isMtp || info.isPtp) {
        manageDevByPolicy(info);
        return;
    }
    // 分区管控
    if (DEV_TYPE_PARTITION == QString(devType)) {
        manageDevByPolicy(info);
        return;
    }

    // 光盘管控
    if (DEV_TYPE_DISK == QString(devType)) {
        if (info.sDevNode.startsWith(CD_ROM_DEV_NODE_HEAD)) {
            manageDevByPolicy(info);
            return;
        }
    }
}

int UsbManageMonitor::checkWhitelist(const QString &sSerial, const QString &sIdVendor, const QString &sIdProduct)
{
    if (sSerial.isEmpty() || sIdVendor.isEmpty() || sIdProduct.isEmpty()) {
        return 1;
    }

    for (UsbConnectionInfo info : m_whiteList) {
        if (sSerial == info.sSerial && sIdVendor == info.sVendorID && sIdProduct == info.sProductID) {
            //如果设备在白名单中返回false
            return 0;
        }
    }
    return 1;
}

QString UsbManageMonitor::getDevLabel(const QString &sDevNode)
{
    QString sLabelStr;

    QProcess getDevLabelProc;
    QString cmd = "sudo blkid -d -s LABEL -o value "; // blkid -d -s LABEL -o value, sudo lsblk -no LABEL
    cmd.append(sDevNode);
    getDevLabelProc.start(cmd);
    getDevLabelProc.waitForFinished();

    sLabelStr = getDevLabelProc.readAllStandardOutput().trimmed();

    getDevLabelProc.deleteLater();
    return sLabelStr;
}

QString UsbManageMonitor::getDevSize(const QString &sDevNode)
{
    QString sParentSizeStr;
    QProcess getDevNameProc;
    QString sCmd = "lsblk -no SIZE ";
    sCmd.append(sDevNode);
    getDevNameProc.start(sCmd);
    getDevNameProc.waitForFinished();

    sParentSizeStr = getDevNameProc.readAllStandardOutput().trimmed().split('\n').first();
    // 为空，直接返回
    if (sParentSizeStr.isEmpty()) {
        return sParentSizeStr;
    }
    sParentSizeStr.append("B");

    return sParentSizeStr;
}

// 获取设备文件系统类型
QString UsbManageMonitor::getDevFsType(const QString &sDevNode)
{
    QString sFsType;
    QProcess getDevFsTypeProc;
    QString cmd = "lsblk -no FSTYPE ";
    cmd.append(sDevNode);
    getDevFsTypeProc.start(cmd);
    getDevFsTypeProc.waitForFinished();

    QString sAllFsType = getDevFsTypeProc.readAllStandardOutput();

    // 只获取第一个数据
    sFsType = sAllFsType.split("\n").first().trimmed();

    getDevFsTypeProc.deleteLater();
    return sFsType;
}

// 将待发送设备信息整合成字符串
QString UsbManageMonitor::mergeDevSavingInfoToStr(const QString &sName, const QString &sStatus, const QString &sSerial, const QString &sVendorID,
                                                  const QString &sProductID, const QString &sParentSize, const QString &sSize)
{
    QString sDatetime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    return sName + (",") + sDatetime + (",") + sStatus + (",") + sSerial + (",") + sVendorID + (",") + sProductID + (",") + sParentSize + (",") + sSize + ("\n");
}

void UsbManageMonitor::updateNameInWhiteLst(const QString &sName, const QString &sSerial,
                                            const QString &sVendorID, const QString &sProductID)
{
    m_usbWhitelistSql->updateNameInWhiteLst(sName, sSerial, sVendorID, sProductID);
    for (int i = 0; i < m_whiteList.size(); i++) {
        UsbConnectionInfo info = m_whiteList.at(i);
        if (sSerial == info.sSerial && sVendorID == info.sVendorID && sProductID == info.sProductID) {
            info.sName = sName;
            m_whiteList.replace(i, info);
            break;
        }
    }
}

// 由节点路径获取设备信息
bool UsbManageMonitor::getDevInfoFromDevNode(UsbDeviceInfo &info, const QString &devNode)
{
    // 使用 udevadm info 读取设备信息
    QProcess proc;
    // 命令
    QString sCmd = "udevadm info " + devNode;
    proc.start(sCmd);
    // 查询等待限时50ms
    proc.waitForFinished(50);
    QString sDevInfos = proc.readAllStandardOutput().trimmed();
    if (sDevInfos.isEmpty() || !proc.readAllStandardError().isEmpty()) {
        return false;
    }

    // 分割每行数据
    QStringList sFirstCutStrList = sDevInfos.split("\n");
    // 获取mtp设备id
    for (QString sDevInfo : sFirstCutStrList) {
        if (sDevInfo.startsWith(UDEVADM_INFO_KEY_DEV_PATH)) {
            info.sSysPath = QString("/sys%1").arg(sDevInfo.split("=").last());
        } else if (sDevInfo.startsWith(UDEVADM_INFO_KEY_DEV_NAME)) {
            info.sDevNode = sDevInfo.split("=").last();
        } else if (sDevInfo.startsWith(UDEVADM_INFO_KEY_DEV_TYPE)) {
            info.sDevType = sDevInfo.split("=").last();
        } else if (sDevInfo.startsWith(UDEVADM_INFO_KEY_DEV_PRODUCT)) {
            info.sProduct = sDevInfo.split("=").last();
        } else if (sDevInfo.startsWith(UDEVADM_INFO_KEY_DEV_VENDOR_ID)) {
            info.sIdVendor = sDevInfo.split("=").last();
        } else if (sDevInfo.startsWith(UDEVADM_INFO_KEY_DEV_PRODUCT_ID)) {
            info.sIdProduct = sDevInfo.split("=").last();
        } else if (sDevInfo.startsWith(UDEVADM_INFO_KEY_DEV_SERIAL)) {
            info.sSerial = sDevInfo.split("=").last();
        } else if (sDevInfo.startsWith(UDEVADM_INFO_KEY_DEV_MTP)) {
            info.isMtp = true;
        } else if (sDevInfo.startsWith(UDEVADM_INFO_KEY_DEV_PTP)) {
            info.isPtp = true;
        }
    }

    info.sLabel = getDevLabel(info.sDevNode);

    info.sSize = getDevSize(info.sDevNode);
    info.sParentSize = info.sSize;
    return true;
}

// 断开设备
bool UsbManageMonitor::disconnectDevBySysPath(const QString &sysPath)
{
    QDir dir(sysPath);
    if (!dir.exists(sysPath)) {
        return false;
    }

    const QStringList &fileNameList = dir.entryList({"authorized"}, QDir::Filter::Files);
    if (0 < fileNameList.size()) {
        QFile file(sysPath + "/" + fileNameList.first());
        if (!file.open(QIODevice::WriteOnly)) {
            return false;
        }

        file.write("0");
        file.close();
        qDebug() << Q_FUNC_INFO << file.fileName();
        return true;
    }

    // 获取上层设备路径
    int sysPathTailLen = sysPath.split("/").last().length();
    QString parentSysPath = sysPath.left(sysPath.length() - sysPathTailLen - 1);
    if (parentSysPath.isEmpty()) {
        return false;
    }

    return disconnectDevBySysPath(parentSysPath);
}

// 根据策略管控设备
void UsbManageMonitor::manageDevByPolicy(const UsbDeviceInfo &info)
{
    QString sBlockedUsbDeviceInfoStr = mergeDevSavingInfoToStr(info.sProduct, CONNECTION_STATUS_BLOCK, info.sSerial,
                                                               info.sIdVendor, info.sIdProduct,
                                                               info.sParentSize, info.sSize);
    QString name = info.sLabel;
    if (name.isEmpty()) {
        name = info.sSize;
    }
    if (name.isEmpty()) {
        name = info.sProduct;
    }
    QString sConnectedUsbDeviceInfoStr = mergeDevSavingInfoToStr(name, CONNECTION_STATUS_CONN, info.sSerial,
                                                                 info.sIdVendor, info.sIdProduct,
                                                                 info.sParentSize, info.sSize);

    if (USB_LIMIT_MODEL_ALL == m_sLimitModel) {
        Q_EMIT sendConnectionRecord(sConnectedUsbDeviceInfoStr);
        // clean
        cleanUsbConnectionLog();
        // updateNameInWhiteLst
        updateNameInWhiteLst(name, info.sSerial, info.sIdVendor, info.sIdProduct);
        // 写入记录
        QString sDatetime = QDateTime::currentDateTime().toString(USB_CONN_TIME_FORMATION);
        writeConnectionRecord(name, sDatetime, CONNECTION_STATUS_CONN, info.sSerial,
                              info.sIdVendor, info.sIdProduct,
                              info.sParentSize, info.sSize);
    } else if (USB_LIMIT_MODEL_ZERO == m_sLimitModel) {
        bool success = disconnectDevBySysPath(info.sSysPath);
        if (success) {
            Q_EMIT sendConnectionRecord(sBlockedUsbDeviceInfoStr);
            Q_EMIT requestShowBlockAllSysNotify();
            // clean
            cleanUsbConnectionLog();
            // updateNameInWhiteLst
            updateNameInWhiteLst(info.sProduct, info.sSerial, info.sIdVendor, info.sIdProduct);
            // 写入记录
            QString sDatetime = QDateTime::currentDateTime().toString(USB_CONN_TIME_FORMATION);
            writeConnectionRecord(info.sProduct, sDatetime, CONNECTION_STATUS_BLOCK, info.sSerial,
                                  info.sIdVendor, info.sIdProduct,
                                  info.sParentSize, info.sSize);
        } else {
            Q_EMIT sendConnectionRecord(sConnectedUsbDeviceInfoStr);
            // clean
            cleanUsbConnectionLog();
            // updateNameInWhiteLst
            updateNameInWhiteLst(name, info.sSerial, info.sIdVendor, info.sIdProduct);
            // 写入记录
            QString sDatetime = QDateTime::currentDateTime().toString(USB_CONN_TIME_FORMATION);
            writeConnectionRecord(name, sDatetime, CONNECTION_STATUS_CONN, info.sSerial,
                                  info.sIdVendor, info.sIdProduct,
                                  info.sParentSize, info.sSize);
        }
    } else if (USB_LIMIT_MODEL_PARTY == m_sLimitModel) {
        int nFlag = checkWhitelist(info.sSerial, info.sIdVendor, info.sIdProduct);
        if (nFlag == 0) {
            Q_EMIT sendConnectionRecord(sConnectedUsbDeviceInfoStr);
            // clean
            cleanUsbConnectionLog();
            // updateNameInWhiteLst
            updateNameInWhiteLst(name, info.sSerial, info.sIdVendor, info.sIdProduct);
            // 写入记录
            QString sDatetime = QDateTime::currentDateTime().toString(USB_CONN_TIME_FORMATION);
            writeConnectionRecord(name, sDatetime, CONNECTION_STATUS_CONN, info.sSerial,
                                  info.sIdVendor, info.sIdProduct,
                                  info.sParentSize, info.sSize);
        } else {
            bool success = disconnectDevBySysPath(info.sSysPath);
            if (success) {
                Q_EMIT sendConnectionRecord(sBlockedUsbDeviceInfoStr);
                Q_EMIT requestShowOnlyWhiteListSysNotify();
                // clean
                cleanUsbConnectionLog();
                // updateNameInWhiteLst
                updateNameInWhiteLst(info.sLabel, info.sSerial, info.sIdVendor, info.sIdProduct);
                // 写入记录
                QString sDatetime = QDateTime::currentDateTime().toString(USB_CONN_TIME_FORMATION);
                writeConnectionRecord(info.sProduct, sDatetime, CONNECTION_STATUS_BLOCK, info.sSerial,
                                      info.sIdVendor, info.sIdProduct,
                                      info.sParentSize, info.sSize);
            } else {
                Q_EMIT sendConnectionRecord(sConnectedUsbDeviceInfoStr);
                // clean
                cleanUsbConnectionLog();
                // updateNameInWhiteLst
                updateNameInWhiteLst(name, info.sSerial, info.sIdVendor, info.sIdProduct);
                // 写入记录
                QString sDatetime = QDateTime::currentDateTime().toString(USB_CONN_TIME_FORMATION);
                writeConnectionRecord(name, sDatetime, CONNECTION_STATUS_CONN, info.sSerial,
                                      info.sIdVendor, info.sIdProduct,
                                      info.sParentSize, info.sSize);
            }
        }
    } else {
        Q_EMIT sendConnectionRecord(sConnectedUsbDeviceInfoStr);
        // clean
        cleanUsbConnectionLog();
        // updateNameInWhiteLst
        updateNameInWhiteLst(name, info.sSerial, info.sIdVendor, info.sIdProduct);
        // 写入记录
        QString sDatetime = QDateTime::currentDateTime().toString(USB_CONN_TIME_FORMATION);
        writeConnectionRecord(name, sDatetime, CONNECTION_STATUS_CONN, info.sSerial,
                              info.sIdVendor, info.sIdProduct,
                              info.sParentSize, info.sSize);
    }
}
