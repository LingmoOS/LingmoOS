// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "devicehelper.h"
#include "unionimage/baseutils.h"
#include <dfm-mount/base/dmount_global.h>
#include <DSysInfo>

#include <QDBusReply>
#include <QDebug>
#include <QRegularExpression>

DCORE_USE_NAMESPACE

DeviceHelper *DeviceHelper::m_instance = nullptr;
using namespace dfmmount;

/*!
   \class DeviceHelper::DeviceHelper
   \brief 设备管理辅助类
   \details 用于管理可移除设备，包括U盘、手机等外界硬件设备
   \note 强关联文管DBus接口，需注意对应接口更新
 */
DeviceHelper::DeviceHelper(QObject *parent)
    : QObject(parent)
{
    // DFM 设备管理接口，访问文件挂载信息
    if (DSysInfo::majorVersion() == "25") {
        m_dfmDeviceManager.reset(new QDBusInterface(QStringLiteral(V25_FILEMANAGER_DAEMON_SERVICE),
                                                    QStringLiteral(V25_FILEMANAGER_DAEMON_PATH),
                                                    QStringLiteral(V25_FILEMANAGER_DAEMON_INTERFACE)));
    } else {
        m_dfmDeviceManager.reset(new QDBusInterface(QStringLiteral(V23_FILEMANAGER_DAEMON_SERVICE),
                                                    QStringLiteral(V23_FILEMANAGER_DAEMON_PATH),
                                                    QStringLiteral(V23_FILEMANAGER_DAEMON_INTERFACE)));
    }

    qInfo() << "m_dfmDeviceManager: majorVersion:" << DSysInfo::majorVersion()
             << "dbus service:" << m_dfmDeviceManager.data()->service()
             << "interface:" << m_dfmDeviceManager.data()->interface()
             << "object:" << m_dfmDeviceManager.data()->objectName()
             << "path:" << m_dfmDeviceManager.data()->path();
}

DeviceHelper::~DeviceHelper()
{

}

DeviceHelper *DeviceHelper::instance()
{
    if (!m_instance) {
        m_instance = new DeviceHelper();
    }

    return m_instance;
}

QStringList DeviceHelper::getAllMountPoints()
{
    loadAllDeviceInfos();
    QStringList allMountPoints;
    for (const auto &deviceId : m_mapDevicesInfos.keys()) {
        QString mountPoint = m_mapDevicesInfos.value(deviceId).value("MountPoint").toString();
        if (!mountPoint.isEmpty())
            allMountPoints.push_back(mountPoint);
    }
    return allMountPoints;
}

QString DeviceHelper::getMountPointByDeviceId(const QString &deviceId)
{
    if (deviceId.isEmpty())
        return QString("");

    QVariantMap infos = loadDeviceInfo(deviceId);
    if (!infos.isEmpty()) {
        return infos.value("MountPoint").toString();
    }

    return QString("");
}

QString DeviceHelper::getDeviceIdByMountPoint(const QString &mnp)
{
    if (mnp.isEmpty())
        return QString("");

    for (const auto &deviceId : m_mapDevicesInfos.keys()) {
        QString mountPoint = m_mapDevicesInfos.value(deviceId).value("MountPoint").toString();
        if (!mountPoint.isEmpty() && mnp == mountPoint)
            return deviceId;
    }

    return QString("");
}

void DeviceHelper::loadAllDeviceInfos()
{
    m_mapDevicesInfos.clear();
    QStringList blockDeviceIds = getBlockDeviceIds();
    QStringList protocalDeviceIds = getProtocalDeviceIds();
    QStringList deviceIds;
    deviceIds << blockDeviceIds << protocalDeviceIds;
    QMap<QString, QVariantMap> deviceInfos;
    for (auto id : deviceIds) {
        loadDeviceInfo(id, true);
    }
}

QStringList DeviceHelper::getAllDeviceIds()
{
    return m_mapDevicesInfos.keys();
}

QStringList DeviceHelper::getBlockDeviceIds()
{
    // 调用 DBus 接口查询可被卸载设备信息
    // GetBlockDevicesIdList(int opts)
    QDBusReply<QStringList> deviceListReply = m_dfmDeviceManager->call("GetBlockDevicesIdList", kRemovable);
    if (!deviceListReply.isValid()) {
        qWarning() << qPrintable("DBus call GetBlockDevicesIdList failed") << deviceListReply.error().message();
        return QStringList();
    }

    return deviceListReply.value();
}

QStringList DeviceHelper::getProtocalDeviceIds()
{
    // 调用 DBus 接口查询可被卸载设备信息
    // GetBlockDevicesIdList(int opts)
    QDBusReply<QStringList> protocalDeviceListReply = m_dfmDeviceManager->call("GetProtocolDevicesIdList");
    if (!protocalDeviceListReply.isValid()) {
        qWarning() << qPrintable("DBus call GetProtocolDevicesIdList failed") << protocalDeviceListReply.error().message();
        return QStringList();
    }

    return protocalDeviceListReply.value();
}

QVariantMap DeviceHelper::loadDeviceInfo(const QString &deviceId, bool reload)
{
    if (deviceId.isEmpty()) {
        return QVariantMap();
    }

    // 过滤samba网络路径
    if (isSamba(deviceId))
        return QVariantMap();

    if (m_mapDevicesInfos.find(deviceId) != m_mapDevicesInfos.end() && !reload) {
        return m_mapDevicesInfos[deviceId];
    }

    QDBusReply<QVariantMap> deviceReply;
    QVariantMap deviceInfo;
    if (deviceId.startsWith("/org/freedesktop/")) {
        deviceReply = m_dfmDeviceManager->call("QueryBlockDeviceInfo", deviceId, false);
        if (!deviceReply.isValid()) {
            qWarning() << qPrintable("DBus call QueryBlockDeviceInfo failed") << deviceReply.error().message();
            return QVariantMap();
        }

        deviceInfo = deviceReply.value();
        if (QString("usb") != deviceInfo.value("ConnectionBus").toString()) {
            qWarning() << QString("deviceId:%1 is not usb device").arg(deviceId);
            return QVariantMap();
        }
    } else {
        deviceReply = m_dfmDeviceManager->call("QueryProtocolDeviceInfo", deviceId, false);
        if (!deviceReply.isValid()) {
            qWarning() << qPrintable("DBus call QueryProtocolDeviceInfo failed") << deviceReply.error().message();
            return QVariantMap();
        }
        deviceInfo = deviceReply.value();
    }

    if (deviceInfo.isEmpty()) {
        if (m_mapDevicesInfos.find(deviceId) != m_mapDevicesInfos.end())
            m_mapDevicesInfos.remove(deviceId);
        return QVariantMap();
    }

    // 过滤为空的挂载点
    if (deviceInfo.value("MountPoint").toString().isEmpty())
        return QVariantMap();

    // 过滤光驱
    if (deviceInfo.contains("Optical") && deviceInfo.value("Optical").toBool())
        return QVariantMap();

    if (deviceId.startsWith("/org/freedesktop/"))
        deviceInfo["DeviceType"] = static_cast<int>(DeviceType::kBlockDevice);
    else
        deviceInfo["DeviceType"] = static_cast<int>(DeviceType::kProtocolDevice);

    m_mapDevicesInfos[deviceId] = deviceInfo;

    return deviceInfo;
}

bool DeviceHelper::detachDevice(const QString &deviceId)
{
    if (deviceId.isEmpty()) {
        qWarning() << "detach fail, deviceId is null.";
        return false;
    }

    qDebug() << "detach device id:" << deviceId;
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(deviceId);
    if (deviceId.startsWith("/org/freedesktop/")) {
        m_dfmDeviceManager->asyncCallWithArgumentList(QStringLiteral("DetachBlockDevice"), argumentList);
    } else {
        m_dfmDeviceManager->asyncCallWithArgumentList(QStringLiteral("DetachProtocolDevice"), argumentList);
    }
    return true;
}

bool DeviceHelper::isExist(const QString &deviceId)
{
    if (deviceId.isEmpty())
        return false;

    return m_mapDevicesInfos.find(deviceId) != m_mapDevicesInfos.end();
}

/*!
   \return 判断 \a url 是否为远程挂载路径
 */
bool DeviceHelper::isSamba(const QUrl &url)
{
    if (!url.isValid())
        return false;

    const QString &path = url.toLocalFile();
    static const QString gvfsMatch { "(^/run/user/\\d+/gvfs/|^/root/.gvfs/|^/media/[\\s\\S]*/smbmounts)" };
    QRegularExpression re { gvfsMatch };
    QRegularExpressionMatch match { re.match(path) };
    return match.hasMatch();
}
