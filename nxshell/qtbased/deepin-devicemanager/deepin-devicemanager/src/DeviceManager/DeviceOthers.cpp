// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceOthers.h"
#include "DBusEnableInterface.h"

DeviceOthers::DeviceOthers()
    : DeviceBaseInfo()
    , m_Model("")
    , m_BusInfo("")
    , m_Capabilities("")
    , m_MaximumPower("")
    , m_Speed("")
    , m_LogicalName("")
{
    initFilterKey();
    m_CanEnable = true;
    m_CanUninstall = true;
}

void DeviceOthers::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    if (!matchToLshw(mapInfo))
        return;
    QString            tmp_Name = m_Name;
    setAttribute(mapInfo, "product", m_Name, false);
    setAttribute(mapInfo, "vendor", m_Vendor, false);
    setAttribute(mapInfo, "product", m_Model, false);
    setAttribute(mapInfo, "version", m_Version);
    setAttribute(mapInfo, "bus info", m_BusInfo);
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "driver", m_Driver);
    setAttribute(mapInfo, "maxpower", m_MaximumPower);
    setAttribute(mapInfo, "speed", m_Speed);
    setAttribute(mapInfo, "logical name", m_LogicalName);

    if(m_Driver.isEmpty() && !m_Avail.compare("yes", Qt::CaseInsensitive)){
        setForcedDisplay(true);
        setCanEnale(false);
        if(!(tmp_Name.contains("fingerprint", Qt::CaseInsensitive) || tmp_Name. contains("MOH", Qt::CaseInsensitive)))
            setCanUninstall(false);
    }

    // 核内驱动不显示卸载菜单
    if (driverIsKernelIn(m_Driver)) {
        m_CanUninstall = false;
    }
}

TomlFixMethod DeviceOthers::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
    TomlFixMethod ret = TOML_None;
    // 添加基本信息
    ret = setTomlAttribute(mapInfo, "Model", m_Model);
    ret = setTomlAttribute(mapInfo, "Bus Info", m_BusInfo);
    ret = setTomlAttribute(mapInfo, "Capabilities", m_Capabilities);
    ret = setTomlAttribute(mapInfo, "Maximum Power", m_MaximumPower);
    ret = setTomlAttribute(mapInfo, "Speed", m_Speed);
////Others
    ret = setTomlAttribute(mapInfo, "Serial Number", m_SerialID);
//3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
    return ret;
}

void DeviceOthers::setInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    // 设置设备基本属性
    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "Revision", m_Version);
    setAttribute(mapInfo, "Driver", m_Driver);
    setAttribute(mapInfo, "Speed", m_Speed);
    setAttribute(mapInfo, "Serial ID", m_SerialID);
    setAttribute(mapInfo, "SysFS ID", m_SysPath);
    /* 禁用时提示获取序列号失败*/
    setAttribute(mapInfo, "Unique ID", m_UniqueID);
    // 防止Serial ID为空
    if (m_SerialID.isEmpty())
        m_SerialID = m_UniqueID;

    setAttribute(mapInfo, "Module Alias", m_Modalias);
    setAttribute(mapInfo, "VID_PID", m_VID_PID);
    m_PhysID = m_VID_PID;

    if (mapInfo["Hardware Class"] != "fingerprint") {
        m_HardwareClass = "others";
    } else {
        // 机器自带指纹模块不支持禁用,和卸载驱动
        setAttribute(mapInfo, "Hardware Class", m_HardwareClass);
        m_CanEnable = false;
        m_CanUninstall = false;
    }
    setAttribute(mapInfo, "cfg_avail", m_Avail);
    if(!m_Avail.compare("yes", Qt::CaseInsensitive))
        setForcedDisplay(true);

    getOtherMapInfo(mapInfo);
    // 核内驱动不显示卸载菜单
    if (driverIsKernelIn(m_Driver)) {
        m_CanUninstall = false;
    }

    m_BusID = mapInfo["SysFS BusID"];
    m_BusID.replace(QRegExp("\\.[0-9]*$"), "");

    // 获取映射到 lshw设备信息的 关键字
    //1-2:1.0
    setHwinfoLshwKey(mapInfo);
}

EnableDeviceStatus DeviceOthers::setEnable(bool e)
{
    if (m_SerialID.isEmpty()) {
        return EDS_NoSerial;
    }

    if (m_UniqueID.isEmpty() || m_SysPath.isEmpty()) {
        return EDS_Faild;
    }
    bool res  = DBusEnableInterface::getInstance()->enable(m_HardwareClass, m_Name, m_SysPath, m_UniqueID, e);
    if (res) {
        m_Enable = e;
    }
    // 设置设备状态
    return res ? EDS_Success : EDS_Faild;
}

const QString &DeviceOthers::name()const
{
    return m_Name;
}

const QString &DeviceOthers::vendor() const
{
    return m_Vendor;
}

const QString &DeviceOthers::busInfo()const
{
    return m_BusInfo;
}

const QString &DeviceOthers::driver()const
{
    return m_Driver;
}

const QString &DeviceOthers::logicalName()const
{
    return m_LogicalName;
}

QString DeviceOthers::subTitle()
{
    return m_Model;
}

const QString DeviceOthers::getOverviewInfo()
{
    return m_Name.isEmpty() ? m_Model : m_Name;
}

bool DeviceOthers::available()
{
    if (driver().isEmpty() && m_HardwareClass == "others") {
        m_Available = false;
    }
    return m_forcedDisplay ? m_forcedDisplay : m_Available;
}

void DeviceOthers::initFilterKey()
{
    addFilterKey(QObject::tr("Device File"));
    addFilterKey(QObject::tr("Hardware Class"));
}

void DeviceOthers::loadBaseDeviceInfo()
{
    // 添加基本信息
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Model"), m_Model);
    addBaseDeviceInfo(tr("Version"), m_Version);
    addBaseDeviceInfo(tr("Bus Info"), m_BusInfo);
    addBaseDeviceInfo(tr("Capabilities"), m_Capabilities);
    addBaseDeviceInfo(tr("Driver"), m_Driver);
    addBaseDeviceInfo(tr("Maximum Power"), m_MaximumPower);
    addBaseDeviceInfo(tr("Speed"), m_Speed);
}

void DeviceOthers::loadOtherDeviceInfo()
{
    if (m_SerialID != m_UniqueID)
        addOtherDeviceInfo(tr("Serial Number"), m_SerialID);
    mapInfoToList();
}

void DeviceOthers::loadTableData()
{
    // 加载表格数据
    QString tName = m_Name;
    if (!available()) {
        tName = "(" + tr("Unavailable") + ") " + m_Name;
    }
    if (!enable()) {
        tName = "(" + tr("Disable") + ") " + m_Name;
    }

    m_TableData.append(tName);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
}
