// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "dmotherboarddevice.h"
#include "dlsdevice.h"

#include "scan.h"
#include "hw.h"

#include <QDebug>

DDEVICE_BEGIN_NAMESPACE

class DMotherboardDevicePrivate
{
public:
    explicit DMotherboardDevicePrivate(DMotherboardDevice *parent)
        : m_hwNode("computer", hw::sys_tem)
        , q_ptr(parent)
    {

        m_listsystemInfo.clear();
        scan_system(m_hwNode);
        addDeviceInfo(m_hwNode, m_listsystemInfo, hw::sys_tem);
        addDeviceInfo(m_hwNode, m_listMotherboardInfo, hw::motherboard);
        addDeviceInfo(m_hwNode, m_listbiosInfo, hw::bios);
    }
    hwNode                    m_hwNode ;

    void addDeviceInfo(hwNode &node, QList< DlsDevice::DDeviceInfo >  &infoLst, hw::hwClass cls);
    Q_DECLARE_PUBLIC(DMotherboardDevice)

private:
    DMotherboardDevice *q_ptr = nullptr;
    QList< DlsDevice::DDeviceInfo >      m_listsystemInfo;
    QList< DlsDevice::DDeviceInfo >      m_listMotherboardInfo;
    QList< DlsDevice::DDeviceInfo >      m_listbiosInfo;
};

void DMotherboardDevicePrivate::addDeviceInfo(hwNode &node, QList< DlsDevice::DDeviceInfo > &infoLst, hw::hwClass cls)
{
    struct DlsDevice::DDeviceInfo entry;

    entry.deviceInfoLstMap.clear();
    entry.devClass = DlsDevice::DtkProductSystem;

    entry.deviceBaseAttrisLst.append("Vendor");
    entry.deviceInfoLstMap.insert("Vendor", QString::fromStdString(node.getVendor()));
    entry.vendorName = QString::fromStdString(node.getVendor());

    entry.deviceBaseAttrisLst.append("Name");
    entry.deviceInfoLstMap.insert("Name", QString::fromStdString(node.getProduct()));
    entry.productName = QString::fromStdString(node.getProduct());

    entry.deviceBaseAttrisLst.append("data");
    entry.deviceInfoLstMap.insert("data", QString::fromStdString(node.getDate()));

    if (! node.getConfig("chassis").empty()) {
        entry.deviceBaseAttrisLst.append("chassis");
        entry.deviceInfoLstMap.insert("chassis", QString::fromStdString(node.getConfig("chassis")));
    }

//--------------------------------ADD Children---------------------
    if (cls == node.getClass())
        infoLst.append(entry);
    for (int i = 0; i < node.countChildren(); i++) {
        addDeviceInfo(*node.getChild(i), infoLst, cls);
    }
}

DMotherboardDevice::DMotherboardDevice(QObject *parent)
    : QObject(parent)
    , d_ptr(new DMotherboardDevicePrivate(this))
{
}

DMotherboardDevice::~DMotherboardDevice()
{
}

QString DMotherboardDevice::vendor()
{
    Q_D(DMotherboardDevice);
    if (d->m_listMotherboardInfo.count() > 0)
        return  d->m_listMotherboardInfo[0].vendorName;

    return QString();
}

QString DMotherboardDevice::model()
{
    Q_D(DMotherboardDevice);
    if (d->m_listMotherboardInfo.count() > 0)
        return  d->m_listMotherboardInfo[0].productName;

    return QString();
}

QString DMotherboardDevice::date()
{
    Q_D(DMotherboardDevice);
    if (d->m_listbiosInfo.count() > 0)
        return  d->m_listbiosInfo[0].deviceInfoLstMap.value("data");
    return QString();
}

QString DMotherboardDevice::biosInformation()
{
    Q_D(DMotherboardDevice);
    if (d->m_listbiosInfo.count() > 0)
        return  d->m_listbiosInfo[0].vendorName + d->m_listbiosInfo[0].productName;
    return QString();
}

QString DMotherboardDevice::chassisInformation()
{
    Q_D(DMotherboardDevice);
    if (d->m_listsystemInfo.count() > 0)
        return  d->m_listsystemInfo[0].deviceInfoLstMap.value("chassis");
    return QString();
}

QString DMotherboardDevice::PhysicalMemoryArrayInformation()
{
    return QString();
}

QString DMotherboardDevice::temperature()
{ //to do
    return QString();
}

DDEVICE_END_NAMESPACE
