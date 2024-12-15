// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DevicePower.h"

// Qt库文件
#include<QFileInfo>

// Dtk头文件
#include <DApplication>

DWIDGET_USE_NAMESPACE

DevicePower::DevicePower()
    : DeviceBaseInfo()
    , m_Model("")
    , m_Type("")
    , m_SerialNumber("")
    , m_ElectricType("")
    , m_MaxPower("")
    , m_Status("")
    , m_Enabled("")
    , m_HotSwitch("")
    , m_Capacity("")
    , m_Voltage("")
    , m_Slot("")
    , m_DesignCapacity("")
    , m_DesignVoltage("")
    , m_SBDSChemistry("")
    , m_SBDSManufactureDate("")
    , m_SBDSSerialNumber("")
    , m_SBDSVersion("")
    , m_Temp("")

{
    // 初始化可显示属性
    initFilterKey();
}

TomlFixMethod DevicePower::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
    TomlFixMethod ret = TOML_None;
    // 添加基本信息    
    ret = setTomlAttribute(mapInfo, "Model", m_Model);    
    ret = setTomlAttribute(mapInfo, "Serial Number", m_SerialNumber);
    ret = setTomlAttribute(mapInfo, "Type", m_Type);
    ret = setTomlAttribute(mapInfo, "Status", m_Status);
    ret = setTomlAttribute(mapInfo, "Capacity", m_Capacity);
    ret = setTomlAttribute(mapInfo, "Voltage", m_Voltage);
    ret = setTomlAttribute(mapInfo, "Slot", m_Slot);
    ret = setTomlAttribute(mapInfo, "Design Capacity", m_DesignCapacity);
    ret = setTomlAttribute(mapInfo, "Design Voltage", m_DesignVoltage);
    ret = setTomlAttribute(mapInfo, "SBDS Version", m_SBDSVersion);
    ret = setTomlAttribute(mapInfo, "SBDS Serial Number", m_SBDSSerialNumber);
    ret = setTomlAttribute(mapInfo, "SBDS Manufacture Date", m_SBDSManufactureDate);
    ret = setTomlAttribute(mapInfo, "SBDS Chemistry", m_SBDSChemistry);
    ret = setTomlAttribute(mapInfo, "Temperature", m_Temp);
//3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
    return ret;
}

bool DevicePower::setInfoFromUpower(const QMap<QString, QString> &mapInfo)
{
    // 设置upower中获取的信息
    if (mapInfo["Device"].contains("line_power", Qt::CaseInsensitive)) {
        return false;
    }
    // m_Name = QObject::tr("battery");  

    setAttribute(mapInfo, "", m_Vendor);
    setAttribute(mapInfo, "", m_Model);
    setAttribute(mapInfo, "", m_Type);
    setAttribute(mapInfo, "serial", m_SerialNumber);
    setAttribute(mapInfo, "", m_ElectricType);
    setAttribute(mapInfo, "", m_MaxPower);
    setAttribute(mapInfo, "", m_Status);
    setAttribute(mapInfo, "", m_Enabled);
    setAttribute(mapInfo, "", m_HotSwitch);
    setAttribute(mapInfo, "capacity", m_Capacity);
    setAttribute(mapInfo, "voltage", m_Voltage);
    setAttribute(mapInfo, "", m_Slot);
//    setAttribute(mapInfo, "capacity", m_DesignCapacity);
//    setAttribute(mapInfo, "voltage", m_DesignVoltage);
    setAttribute(mapInfo, "", m_SBDSChemistry);
    setAttribute(mapInfo, "", m_SBDSManufactureDate);
    setAttribute(mapInfo, "", m_SBDSSerialNumber);
    setAttribute(mapInfo, "", m_SBDSVersion);

    // 添加电池温度
    setAttribute(mapInfo, "temperature", m_Temp);
    /*
     * 温度底层的获取方式是/sys/class/power_supply/BAT0下面的temp文件
     * 以前向temp文件写温度时如果是28度 会写2.8(需要做*10的处理)  但是现在会直接写28
     * 处理方法：取消*10的操作
     * if (!m_Temp.isEmpty()) {
        double temp = m_Temp.replace("degrees C", "").trimmed().toDouble();
        temp = temp * 10;
        m_Temp = QString("%1 degrees C").arg(temp);
    }*/

    getOtherMapInfo(mapInfo);
    return true;
}

void DevicePower::setDaemonInfo(const QMap<QString, QString> &mapInfo)
{
    // 设置守护进程信息
    if (m_Name == QObject::tr("battery"))
        getOtherMapInfo(mapInfo);
}

const QString &DevicePower::name()const
{
    return m_Name;
}

const QString &DevicePower::vendor() const
{
    return m_Vendor;
}

const QString &DevicePower::driver() const
{
    return m_Driver;
}

bool DevicePower::available()
{
    return true;
}

QString DevicePower::subTitle()
{
    return m_Name;
}

const QString DevicePower::getOverviewInfo()
{
    // 获取概况信息
    return DApplication::translate("ManulTrack", m_Name.trimmed().toStdString().data(), "");
}

void DevicePower::initFilterKey()
{
    // 初始化可显示属性
    addFilterKey(QObject::tr("native-path"));
    addFilterKey(QObject::tr("power supply"));
    addFilterKey(QObject::tr("updated"));
    addFilterKey(QObject::tr("has history"));
    addFilterKey(QObject::tr("has statistics"));
    addFilterKey(QObject::tr("rechargeable"));
    addFilterKey(QObject::tr("state"));
    addFilterKey(QObject::tr("warning-level"));
    addFilterKey(QObject::tr("energy"));
    addFilterKey(QObject::tr("energy-empty"));
    addFilterKey(QObject::tr("energy-full"));
    addFilterKey(QObject::tr("energy-full-design"));
    addFilterKey(QObject::tr("energy-rate"));
    addFilterKey(QObject::tr("voltage"));
    addFilterKey(QObject::tr("percentage"));
//    addFilterKey(QObject::tr("temperature"));    // 温度已经常规显示
    addFilterKey(QObject::tr("technology"));
    addFilterKey(QObject::tr("icon-name"));
    addFilterKey(QObject::tr("online"));
    addFilterKey(QObject::tr("daemon-version"));
    addFilterKey(QObject::tr("on-battery"));
    addFilterKey(QObject::tr("lid-is-closed"));
    addFilterKey(QObject::tr("lid-is-present"));
    addFilterKey(QObject::tr("critical-action"));
}

void DevicePower::loadBaseDeviceInfo()
{
    // 添加基本信息
    addBaseDeviceInfo(tr("Name"), m_Name);
    addBaseDeviceInfo(tr("Model"), m_Model);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Serial Number"), m_SerialNumber);
    addBaseDeviceInfo(tr("Type"), m_Type);
    addBaseDeviceInfo(tr("Status"), m_Status);
    addBaseDeviceInfo(tr("Capacity"), m_Capacity);
    addBaseDeviceInfo(tr("Voltage"), m_Voltage);
    addBaseDeviceInfo(tr("Slot"), m_Slot);
    addBaseDeviceInfo(tr("Design Capacity"), m_DesignCapacity);
    addBaseDeviceInfo(tr("Design Voltage"), m_DesignVoltage);
    addBaseDeviceInfo(tr("SBDS Version"), m_SBDSVersion);
    addBaseDeviceInfo(tr("SBDS Serial Number"), m_SBDSSerialNumber);
    addBaseDeviceInfo(tr("SBDS Manufacture Date"), m_SBDSManufactureDate);
    addBaseDeviceInfo(tr("SBDS Chemistry"), m_SBDSChemistry);
    addBaseDeviceInfo(tr("Temperature"), m_Temp);
}

void DevicePower::loadOtherDeviceInfo()
{
    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DevicePower::loadTableData()
{
    // 加载表格信息
    m_TableData.append(m_Name);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
}
