// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceOtherPCI.h"

DeviceOtherPCI::DeviceOtherPCI()
    : DeviceBaseInfo()
    , m_Model("")
    , m_BusInfo("")
    , m_Width("")
    , m_Clock("")
    , m_Capabilities("")
    , m_Irq("")
    , m_Memory("")
    , m_Latency("")
    , m_InputOutput("")
{

}

TomlFixMethod DeviceOtherPCI::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
    TomlFixMethod ret = TOML_None;
   // 添加基本信息
    ret = setTomlAttribute(mapInfo, "Model", m_Model);
    ret = setTomlAttribute(mapInfo, "Bus Info", m_BusInfo);

    // 添加其他信息,成员变量
    ret = setTomlAttribute(mapInfo, "Input/Output", m_InputOutput);
    ret = setTomlAttribute(mapInfo, "Memory", m_Memory);
    ret = setTomlAttribute(mapInfo, "IRQ", m_Irq);
    ret = setTomlAttribute(mapInfo, "Latency", m_Latency);
    ret = setTomlAttribute(mapInfo, "Capabilities", m_Version);
//3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
    return ret;
}

const QString &DeviceOtherPCI::name()const
{
    return m_Name;
}

const QString &DeviceOtherPCI::vendor() const
{
    return m_Vendor;
}

const QString &DeviceOtherPCI::driver()const
{
    return m_Driver;
}

QString DeviceOtherPCI::subTitle()
{
    return m_Model;
}

const QString DeviceOtherPCI::getOverviewInfo()
{
    return m_Name.isEmpty() ? m_Model : m_Name;
}

void DeviceOtherPCI::initFilterKey()
{

}

void DeviceOtherPCI::loadBaseDeviceInfo()
{
    // 添加基本信息
    addBaseDeviceInfo(tr("Name"), m_Name.isEmpty() ? m_Vendor + m_Model : m_Name);
    addBaseDeviceInfo(tr("Vendor"), m_Vendor);
    addBaseDeviceInfo(tr("Model"), m_Model);
    addBaseDeviceInfo(tr("Bus Info"), m_BusInfo);
    addBaseDeviceInfo(tr("Version"), m_Version);
}

void DeviceOtherPCI::loadOtherDeviceInfo()
{
    // 添加其他信息,成员变量
    addOtherDeviceInfo(tr("Input/Output"), m_InputOutput);
    addOtherDeviceInfo(tr("Memory"), m_Memory);
    addOtherDeviceInfo(tr("IRQ"), m_Irq);
    addOtherDeviceInfo(tr("Latency"), m_Latency);
//    addOtherDeviceInfo(tr("Clock"), m_Clock);
//    addOtherDeviceInfo(tr("Width"), m_Width);
    addOtherDeviceInfo(tr("Driver"), m_Driver);
    addOtherDeviceInfo(tr("Capabilities"), m_Version);

    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceOtherPCI::loadTableData()
{
    // 加载表格数据
    m_TableData.append(m_Name);
    m_TableData.append(m_Vendor);
    m_TableData.append(m_Model);
}
