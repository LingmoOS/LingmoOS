// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 项目自身文件
#include "DeviceComputer.h"

// Qt库文件
#include <QMap>
#include <QLoggingCategory>

DeviceComputer::DeviceComputer()
    : m_HomeUrl("")
    , m_OsDescription("")
    , m_OS("")
    , m_Type("")
    //, m_Driver("")
{

}

TomlFixMethod DeviceComputer::setInfoFromTomlOneByOne(const QMap<QString, QString> &mapInfo)
{
        TomlFixMethod ret = TOML_None;
//  must cover the  loadOtherDeviceInfo
    // 添加基本信息
    ret = setTomlAttribute(mapInfo, "HOME_URL", m_HomeUrl);
    ret = setTomlAttribute(mapInfo, "Type", m_Type);
    ret = setTomlAttribute(mapInfo, "vendor", m_Vendor);
    ret = setTomlAttribute(mapInfo, "OS", m_OS);
    ret = setTomlAttribute(mapInfo, "OsDescription", m_OsDescription);
//3. 获取设备的其它信息
    getOtherMapInfo(mapInfo);
    return ret;
}

const QString &DeviceComputer::name() const
{
    return m_Name;
}

const QString &DeviceComputer::vendor() const
{
    return m_Vendor;
}

const QString &DeviceComputer::driver() const
{
    return m_Driver;
}

void DeviceComputer::setHomeUrl(const QString &value)
{
    // 设置主页网站
    m_HomeUrl = value;
}

void DeviceComputer::setOsDescription(const QString &value)
{
    // 设置操作系统描述
    m_OsDescription = value;
}

void DeviceComputer::setOS(const QString &value)
{
    // 设置操作系统
    m_OS = value;
}

void DeviceComputer::setVendor(const QString &value)
{
    // 设置制造商
    m_Vendor = value;
}

void DeviceComputer::setName(const QString &value)
{
    // 设置计算机名称
    m_Name = value;
    if (m_Name.contains("None", Qt::CaseInsensitive))
        m_Name = "";
}

void DeviceComputer::setType(const QString &value)
{
    // 设置设备类型
    m_Type = value;
}

void DeviceComputer::setVendor(const QString &dm1Vendor, const QString &dm2Vendor)
{
    // 设置制造商
    if (dm1Vendor.contains("System manufacturer"))
        m_Vendor = dm2Vendor;
    else
        m_Vendor = dm1Vendor;
}

void DeviceComputer::setName(const QString &dm1Name, const QString &dm2Name, const QString &dm1Family, const QString &dm1Version)
{
    // name
    QString pname;
    if (dm1Name.contains("System Product Name"))
        pname = dm2Name;
    else
        pname = dm1Name;

    // family
    QString family = dm1Family;
    if (dm1Family.contains("unknown", Qt::CaseInsensitive) \
            || dm1Family.contains("System Version") \
            || dm1Family.contains("Not Specified", Qt::CaseInsensitive) \
            || dm1Family.contains("x.x", Qt::CaseInsensitive) \
            || dm1Family.contains("Not Applicable", Qt::CaseInsensitive)) {
        family = "";
    }

    // version
    QString version = dm1Version;
    if (dm1Version.contains("unknown", Qt::CaseInsensitive) \
            || dm1Version.contains("System Version") \
            || dm1Version.contains("Not Specified", Qt::CaseInsensitive) \
            || dm1Version.contains("x.x", Qt::CaseInsensitive) \
            || dm1Version.contains("Not Applicable", Qt::CaseInsensitive)) {
        version = "";
    }

    // 去除 name,vendor,family中的重复字段.再拼接
    pname = pname.remove(m_Vendor, Qt::CaseInsensitive);
    pname = pname.remove(version, Qt::CaseInsensitive);
    version = version.remove(m_Vendor, Qt::CaseInsensitive);
    version = version.remove(family, Qt::CaseInsensitive);
    family = family.remove(m_Vendor, Qt::CaseInsensitive);
    family = family.remove(version, Qt::CaseInsensitive);

    m_Name = family + " " + version + " " + pname;
}

const QString DeviceComputer::getOverviewInfo()
{
    // 获取概况信息
    QString model;
    model += m_Vendor + QString(" ");
    model += m_Name + QString(" ");
    model += m_Type + QString(" ");

    return model;
}

const QString DeviceComputer::getOSInfo()
{
    return m_OsDescription + " " + m_OS;
}

void DeviceComputer::initFilterKey()
{

}

void DeviceComputer::loadBaseDeviceInfo()
{
    // 添加基本信息
    addBaseDeviceInfo(tr("Name"), m_Name);
}

void DeviceComputer::loadOtherDeviceInfo()
{
    // 将QMap<QString, QString>内容转存为QList<QPair<QString, QString>>
    mapInfoToList();
}

void DeviceComputer::loadTableData()
{

}
