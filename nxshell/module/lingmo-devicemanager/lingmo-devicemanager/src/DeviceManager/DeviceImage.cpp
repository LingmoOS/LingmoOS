// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceImage.h"
#include "DeviceManager.h"
#include "DBusEnableInterface.h"
#include "DBusInterface.h"

DeviceImage::DeviceImage()
    : DeviceBaseInfo()
    , m_Model("")
    , m_BusInfo("")
    , m_Capabilities("")
    , m_MaximumPower("")
    , m_Speed("")
{
    m_CanEnable = true;
    m_CanUninstall = true;
}

void DeviceImage::setInfoFromLshw(const QMap<QString, QString> &mapInfo)
{
    if (!matchToLshw(mapInfo))
        return;

    setAttribute(mapInfo, "product", m_Name, false);
    setAttribute(mapInfo, "vendor", m_Vendor);
    setAttribute(mapInfo, "version", m_Version);
    setAttribute(mapInfo, "bus info", m_BusInfo);
    setAttribute(mapInfo, "capabilities", m_Capabilities);
    setAttribute(mapInfo, "driver", m_Driver, false);
    setAttribute(mapInfo, "maxpower", m_MaximumPower);
    setAttribute(mapInfo, "speed", m_Speed);
    if (driverIsKernelIn(m_Driver)) {
        m_CanUninstall = false;
    }
}

TomlFixMethod DeviceImage::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
    TomlFixMethod ret = TOML_None;

    // 添加基本信息
    ret = setTomlAttribute(mapInfo, "Model", m_Model);
    ret = setTomlAttribute(mapInfo, "Bus Info", m_BusInfo);
    ret = setTomlAttribute(mapInfo, "Version", m_Version);
    // 添加其他信息,成员变量
    ret = setTomlAttribute(mapInfo, "Speed", m_Speed);
    ret = setTomlAttribute(mapInfo, "Maximum Power", m_MaximumPower);
    ret = setTomlAttribute(mapInfo, "Capabilities", m_Capabilities);
    ret = setTomlAttribute(mapInfo, "Serial Number", m_SerialID);
//3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
    return ret;
}

void DeviceImage::setInfoFromHwinfo(const QMap<QString, QString> &mapInfo)
{
    if (mapInfo.find("unique_id") != mapInfo.end()) {
        m_UniqueID = mapInfo["unique_id"];
        m_Name = mapInfo["name"];
        m_SysPath = mapInfo["path"];
        m_HardwareClass = mapInfo["Hardware Class"];
        m_Enable = false;
        setAttribute(mapInfo, "driver", m_Driver);
        //设备禁用的情况，没必要再继续向下执行，直接return
        m_CanUninstall = !driverIsKernelIn(m_Driver);
        return;
    }
    if (mapInfo.find("Enable") != mapInfo.end()) {
        m_Enable = false;
    }
    setAttribute(mapInfo, "Serial ID", m_SerialID);
    setAttribute(mapInfo, "Unique ID", m_UniqueID);
    // 防止Serial ID为空
    if (m_SerialID.isEmpty())
        m_SerialID = m_UniqueID;
    setAttribute(mapInfo, "SysFS ID", m_SysPath);
    setAttribute(mapInfo, "Device", m_Name);
    setAttribute(mapInfo, "Vendor", m_Vendor);
    setAttribute(mapInfo, "Model", m_Model);
    setAttribute(mapInfo, "Revision", m_Version);
    setAttribute(mapInfo, "SysFS BusID", m_BusInfo);
    setAttribute(mapInfo, "Driver", m_Driver, true);//
    setAttribute(mapInfo, "Driver Modules", m_Driver, true);
    setAttribute(mapInfo, "Speed", m_Speed);
    setAttribute(mapInfo, "Module Alias", m_Modalias);
    setAttribute(mapInfo, "VID_PID", m_VID_PID);
    m_PhysID = m_VID_PID;
    if (driverIsKernelIn(m_Driver)) {
        m_CanUninstall = false;
    }

    // 获取映射到 lshw设备信息的 关键字
    //1-2:1.0
    setHwinfoLshwKey(mapInfo);
}

const QString &DeviceImage::name()const
{
    return m_Name;
}

const QString &DeviceImage::vendor() const
{
    return m_Vendor;
}

const QString &DeviceImage::driver()const
{
    return m_Driver;
}

QString DeviceImage::subTitle()
{
    return m_Name;
}

const QString DeviceImage::getOverviewInfo()
{
    QString ov = QString("%1 (%2)") \
                 .arg(m_Name) \
                 .arg(m_Model);

    return ov;
}

EnableDeviceStatus DeviceImage::setEnable(bool e)
{
    if (m_SerialID.isEmpty()) {
        return EDS_NoSerial;
    }

    if (m_UniqueID.isEmpty() || m_SysPath.isEmpty()) {
        return EDS_Faild;
    }
    bool res  = DBusEnableInterface::getInstance()->enable("camera", m_Name, m_SysPath, m_UniqueID, e, m_Driver);
    if (res) {
        m_Enable = e;
        if(e)
            DBusInterface::getInstance()->refreshInfo();
    }
    // 设置设备状态
    return res ? EDS_Success : EDS_Faild;
}

bool DeviceImage::enable()
{
    // 获取设备状态
    return m_Enable;
}

void DeviceImage::initFilterKey()
{

}

void DeviceImage::loadBaseDeviceInfo()
{
    // 添加基本信息
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Version"), m_Version);
    addBaseDeviceInfo(tr("Model"), m_Model);
    addBaseDeviceInfo(tr("Bus Info"), m_BusInfo);
}

void DeviceImage::loadOtherDeviceInfo()
{
    // 添加其他信息,成员变量
    addOtherDeviceInfo(tr("Module Alias"), m_Modalias);
    addOtherDeviceInfo(tr("Physical ID"), m_PhysID);
    addOtherDeviceInfo(tr("Speed"), m_Speed);
    addOtherDeviceInfo(tr("Maximum Power"), m_MaximumPower);
    addOtherDeviceInfo(tr("Driver"), m_Driver);
    addOtherDeviceInfo(tr("Capabilities"), m_Capabilities);
    if (m_SerialID != m_UniqueID)
        addOtherDeviceInfo(tr("Serial Number"), m_SerialID);

    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceImage::loadTableData()
{
    // 记载表格内容
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
