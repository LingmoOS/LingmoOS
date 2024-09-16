// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceNetwork.h"
#include "DBusEnableInterface.h"
#include "commonfunction.h"

#include <QFileInfo>
#include <QProcess>

DeviceNetwork::DeviceNetwork()
    : DeviceBaseInfo()
    , m_Model("")//, m_Version("")
    , m_BusInfo("")
    , m_LogicalName("")
    , m_MACAddress("")
    , m_Irq("")
    , m_Memory("")
    , m_Width("")
    , m_Clock("")
    , m_Capabilities("")
    , m_Autonegotiation("")
    , m_Broadcast("")
    , m_DriverModules("")
    , m_DriverVersion("")
    , m_Duplex("")
    , m_Firmware("")
    , m_Port("")
    , m_Link("")
    , m_Ip("")
    , m_Speed("")
    , m_Capacity("")
    , m_Latency("")
    , m_Multicast("")
    , m_IsWireless(false)
{
    // 初始化可显示属性
    initFilterKey();

    // 设备可禁用
    m_CanEnable = true;
    m_CanUninstall = true;
}

void DeviceNetwork::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    if (!matchToLshw(mapInfo)
        && Common::boardVendorType() != "KLVV" && Common::boardVendorType() != "KLVU"
        && Common::boardVendorType() != "PGUW" && Common::boardVendorType() != "PGUV") {
        return;
    }
    // 设置由lshw获取的信息
    setAttribute(mapInfo, "description", m_Model);
    setAttribute(mapInfo, "product", m_Name);
    setAttribute(mapInfo, "vendor", m_Vendor);
    if (m_SysPath.contains("usb")) {
        QProcess process;
        QString vendorId = getVendorOrModelId(m_SysPath, true).trimmed();
        QString deviceId = getVendorOrModelId(m_SysPath, false).trimmed();
        process.start("lsusb -v -d " + vendorId + ":" + deviceId);
        process.waitForFinished(-1);

        QString output = process.readAllStandardOutput();

        foreach (QString out, output.split("\n")) {
            if (!m_Vendor.isEmpty() && !m_Name.isEmpty())
                break;
            // 从USB设备获取制造商和设备名称
            if (m_Vendor.isEmpty() && out.contains("idVendor", Qt::CaseSensitive)) {
                m_Vendor = out.remove(0, out.indexOf(vendorId) + 4).trimmed();
            } else if (m_Name.isEmpty() && out.contains("idProduct", Qt::CaseSensitive)) {
                m_Name = out.remove(0, out.indexOf(deviceId) + 4).trimmed();
            }
        }
    } else {
        setAttribute(mapInfo, "description", m_Name, false);
    }
    setAttribute(mapInfo, "version", m_Version);
    setAttribute(mapInfo, "bus info", m_BusInfo);
    setAttribute(mapInfo, "logical name", m_LogicalName);
    setAttribute(mapInfo, "serial", m_MACAddress);
    setAttribute(mapInfo, "irq", m_Irq);
    setAttribute(mapInfo, "memory", m_Memory);
    setAttribute(mapInfo, "width", m_Width);
    setAttribute(mapInfo, "clock", m_Clock);
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "autonegotiation", m_Autonegotiation);
    setAttribute(mapInfo, "broadcast", m_Broadcast);
    setAttribute(mapInfo, "driver", m_Driver);
    setAttribute(mapInfo, "driverversion", m_DriverVersion);
    setAttribute(mapInfo, "duplex", m_Duplex);
    setAttribute(mapInfo, "firmware", m_Firmware);
    setAttribute(mapInfo, "port", m_Port);
    setAttribute(mapInfo, "link", m_Link);
    setAttribute(mapInfo, "ip", m_Ip);
    setAttribute(mapInfo, "size", m_Speed);
    setAttribute(mapInfo, "capacity", m_Capacity);
    setAttribute(mapInfo, "latency", m_Latency);
    setAttribute(mapInfo, "multicast", m_Multicast);
    if (driverIsKernelIn(m_DriverModules) || driverIsKernelIn(m_Driver)) {
        m_CanUninstall = false;
    }
    if (m_Vendor.isEmpty() && !m_Name.isEmpty()) {
        if (m_Name.contains("ARM", Qt::CaseInsensitive))
            m_Vendor = "ARM Ltd";
         else
            m_Vendor = m_Name;
    }

    // 加载其他信息
    getOtherMapInfo(mapInfo);
}

TomlFixMethod DeviceNetwork::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
    TomlFixMethod ret = TOML_None;
    // 添加基本信息
    ret = setTomlAttribute(mapInfo, "Type", m_Model);
    ret = setTomlAttribute(mapInfo, "Bus Info", m_BusInfo);
    ret = setTomlAttribute(mapInfo, "Capabilities", m_Capabilities);
    ret = setTomlAttribute(mapInfo, "Driver Version", m_DriverVersion);
    // 添加其他信息,成员变量
    ret = setTomlAttribute(mapInfo, "Maximum Rate", m_Capacity);
    ret = setTomlAttribute(mapInfo, "Negotiation Rate", m_Speed);
    ret = setTomlAttribute(mapInfo, "Port", m_Port);
    ret = setTomlAttribute(mapInfo, "Multicast", m_Multicast);
    ret = setTomlAttribute(mapInfo, "Link", m_Link);
    ret = setTomlAttribute(mapInfo, "Latency", m_Latency);
    ret = setTomlAttribute(mapInfo, "IP", m_Ip);
    ret = setTomlAttribute(mapInfo, "Firmware", m_Firmware);
    ret = setTomlAttribute(mapInfo, "Duplex", m_Duplex);
    ret = setTomlAttribute(mapInfo, "Broadcast", m_Broadcast);
    ret = setTomlAttribute(mapInfo, "Auto Negotiation", m_Autonegotiation);
//    ret = setTomlAttribute(mapInfo, "Clock", m_Clock);
//    ret = setTomlAttribute(mapInfo, "Width", m_Width);
    ret = setTomlAttribute(mapInfo, "Memory Address", m_Memory);
    ret = setTomlAttribute(mapInfo, "IRQ", m_Irq);
    ret = setTomlAttribute(mapInfo, "MAC Address", m_MACAddress);
    ret = setTomlAttribute(mapInfo, "Logical Name", m_LogicalName);
//3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
    return ret;
}

bool DeviceNetwork::setInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    if (mapInfo.find("path") != mapInfo.end()) {
        setAttribute(mapInfo, "name", m_Name);
        setAttribute(mapInfo, "unique_id", m_UniqueID);
        setAttribute(mapInfo, "path", m_SysPath);
        setAttribute(mapInfo, "Hardware Class", m_HardwareClass);
        setAttribute(mapInfo, "driver", m_Driver);
        m_Enable = false;
        //设备禁用的情况，没必要再继续向下执行(可能会引起不必要的问题)，直接return
        m_CanUninstall = !driverIsKernelIn(m_Driver);
        return true;
    }
    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Device File", m_LogicalName);
    setAttribute(mapInfo, "HW Address", m_MACAddress);
    setAttribute(mapInfo, "Permanent HW Address", m_UniqueID);
    setAttribute(mapInfo, "SysFS Device Link", m_SysPath);
    setAttribute(mapInfo, "Driver", m_Driver);
    setAttribute(mapInfo, "Driver Modules", m_DriverModules);
    setAttribute(mapInfo, "Module Alias", m_Modalias);
    setAttribute(mapInfo, "VID_PID", m_VID_PID);
    m_PhysID = m_VID_PID;

    if (driverIsKernelIn(m_DriverModules) || driverIsKernelIn(m_Driver)) {
        m_CanUninstall = false;
    }

    // 获取设备路径
    if (m_SysPath.isEmpty() && !mapInfo.contains("SysFS Device Link")
            && mapInfo["SysFS ID"].startsWith("/devices")) {
        m_SysPath = mapInfo["SysFS ID"];
    }
    // 判断是否是无线网卡
    setIsWireless(mapInfo["SysFS ID"]);

    setHwinfoLshwKey(mapInfo);

    return true;
}

bool DeviceNetwork::setInfoFromWifiInfo(const QMap<QString, QString> &mapInfo)
{
    // 机器自身蓝牙
    if (m_Name.contains("Huawei", Qt::CaseInsensitive)) {
        setAttribute(mapInfo, "Chip Type", m_Name);
        setAttribute(mapInfo, "Vendor", m_Vendor);
        setAttribute(mapInfo, "Type", m_Model);

        return true;
    } else {
        return false;
    }
}

void DeviceNetwork::setIsWireless(const QString &sysfs)
{
    // 路径下包含 phy80211 或 wireless 是无线网卡
    QFileInfo fileInfo(QString("/sys") + sysfs);
    if (fileInfo.exists("phy80211") || fileInfo.exists("wireless")) {
        m_IsWireless = true;
    }
}

const QString &DeviceNetwork::name()const
{
    return m_Name;
}

const QString &DeviceNetwork::vendor() const
{
    return m_Vendor;
}

const QString &DeviceNetwork::driver()const
{
    if (! m_DriverModules.isEmpty())
        return m_DriverModules;
    return m_Driver;
}

QString DeviceNetwork::subTitle()
{
    return m_Name;
}

const QString DeviceNetwork::getOverviewInfo()
{
    // 获取概况信息
    return m_Name.isEmpty() ? m_Model : m_Name;
}

EnableDeviceStatus DeviceNetwork::setEnable(bool e)
{
    m_HardwareClass = "network interface";
    // 设置设备状态
    if (m_SysPath.isEmpty()) {
        return EDS_Faild;
    } else if (m_SysPath.contains("usb") && m_UniqueID.isEmpty()) {
        return EDS_Faild;
    }

    bool res  = DBusEnableInterface::getInstance()->enable(m_HardwareClass, m_Name, m_LogicalName, m_UniqueID, e, m_Driver);
    if (res) {
        m_Enable = e;
    }
    // 设置设备状态
    return res ? EDS_Success : EDS_Faild;
}

bool DeviceNetwork::enable()
{
    // 通过ifconfig配置网卡禁用启用
    return m_Enable;
}

void DeviceNetwork::correctCurrentLinkStatus(QString linkStatus)
{
    if (m_Link != linkStatus)
        m_Link = linkStatus;
}

QString DeviceNetwork::logicalName()
{
    return m_LogicalName;
}

bool DeviceNetwork::isWireless()
{
    return m_IsWireless;
}

QString DeviceNetwork::hwAddress()
{
    return m_MACAddress;
}

void DeviceNetwork::initFilterKey()
{
    // 初始化可显示属性
    addFilterKey(QObject::tr("ioport"));
    addFilterKey(QObject::tr("network"));
}

void DeviceNetwork::loadBaseDeviceInfo()
{
    // 添加基本信息
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Type"), m_Model);
    addBaseDeviceInfo(tr("Version"), m_Version);
    addBaseDeviceInfo(tr("Bus Info"), m_BusInfo);
    addBaseDeviceInfo(tr("Capabilities"), m_Capabilities);
    addBaseDeviceInfo(tr("Driver"), m_Driver);
    addBaseDeviceInfo(tr("Driver Version"), m_DriverVersion);
}

void DeviceNetwork::loadOtherDeviceInfo()
{
    // 添加其他信息,成员变量
    addOtherDeviceInfo(tr("Module Alias"), m_Modalias);
    addOtherDeviceInfo(tr("Physical ID"), m_PhysID);
    addOtherDeviceInfo(tr("Maximum Rate"), m_Capacity);        // 1050需求 容量改为最大速率
    addOtherDeviceInfo(tr("Negotiation Rate"), m_Speed);       // 1050需求 速度改为协商速率
    addOtherDeviceInfo(tr("Port"), m_Port);
    addOtherDeviceInfo(tr("Multicast"), m_Multicast);
    addOtherDeviceInfo(tr("Link"), m_Link);
    addOtherDeviceInfo(tr("Latency"), m_Latency);
    addOtherDeviceInfo(tr("IP"), m_Ip);
    addOtherDeviceInfo(tr("Firmware"), m_Firmware);
    addOtherDeviceInfo(tr("Duplex"), m_Duplex);
    addOtherDeviceInfo(tr("Broadcast"), m_Broadcast);
    addOtherDeviceInfo(tr("Auto Negotiation"), m_Autonegotiation);
//    addOtherDeviceInfo(tr("Clock"), m_Clock);
//    addOtherDeviceInfo(tr("Width"), m_Width);
    addOtherDeviceInfo(tr("Memory Address"), m_Memory);        // 1050需求 内存改为内存地址
    addOtherDeviceInfo(tr("IRQ"), m_Irq);
    addOtherDeviceInfo(tr("MAC Address"), m_MACAddress);
    addOtherDeviceInfo(tr("Logical Name"), m_LogicalName);

    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceNetwork::loadTableHeader()
{
    m_TableHeader.append(tr("Name"));
    m_TableHeader.append(tr("Vendor"));
    m_TableHeader.append(tr("Type"));
}

void DeviceNetwork::loadTableData()
{
    // 根据是否禁用设置设备名称
    QString tName = m_Name;

    if (!available()) {
        tName = "(" + tr("Unavailable") + ") " + m_Name;
    }

    if (!enable()) {
        tName = "(" + tr("Disable") + ") " + m_Name;
    }

    // 加载表格数据信息
    m_TableData.append(tName);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
}
