// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "dnetdevice.h"
#include "dlsdevice.h"

#include "scan.h"
#include "hw.h"
#include "nl_link.h"
#include "nl_addr.h"
#include "netlink.h"

#include <QMultiMap>
#include <QMap>
#include <QDebug>
#include <QProcess>
#include <QRegularExpression>

DDEVICE_BEGIN_NAMESPACE
/*

*/

// using INetAddr = std::shared_ptr<struct inet_addr_t>;

using INet4Addr = std::shared_ptr<  DNetDevice::DInetAddr4 >;
using INet6Addr = std::shared_ptr<  DNetDevice::DInetAddr6 >;

class DNetDevicePrivate
{
public:
    explicit DNetDevicePrivate(DNetDevice *parent)
        : m_hwNode("computer", hw::sys_tem)
        , q_ptr(parent)
        , m_netlink(new Netlink())
    {
        m_listDeviceInfo.clear();
        scan_system(m_hwNode);
        addDeviceInfo(m_hwNode, m_listDeviceInfo);
        updateAddr();
        updateInfo();
    }
    hwNode                    m_hwNode ;
    QList< DlsDevice::DDeviceInfo >      m_listDeviceInfo;
    void addDeviceInfo(hwNode &node, QList< DlsDevice::DDeviceInfo >  &infoLst);
    Q_DECLARE_PUBLIC(DNetDevice)

protected:
    void updateAddr();

    void updateInfo();

    QMultiMap < int, int > m_portNumberMap;
    QMap < int, QString >    m_portStatus;

private:
    DNetDevice *q_ptr = nullptr;
    std::unique_ptr<Netlink> m_netlink;
    QMultiMap< int, INet4Addr > m_addrIpv4DB;
    QMultiMap< int, INet6Addr > m_addrIpv6DB;
};

void DNetDevicePrivate::updateInfo()
{
    QProcess process;
    process.start("netstat", QStringList() << "-p");
    process.waitForFinished(-1);
    QString cacheinfo = process.readAllStandardOutput();
    QStringList items = cacheinfo.split("\n", D_SPLIT_BEHAVIOR);
    process.close();

    for (int i = 0; i < items.count(); ++i) {
        if (!items.contains("[ ]")) {
            continue;
        } else {
            QStringList words = items[i].split("  ", D_SPLIT_BEHAVIOR);  //make sure  double space
            if (words.size() < 8)
                continue;

            QRegularExpression reg("([0-9]{1,})/(\\s\\S*)");
            auto match = reg.match(words[6]);
            if (match.hasMatch())  {
                int  pid = match.captured(1).toInt();
                QString  ProgramName = match.captured(2);
                QString state = words[4];
                int INode = words[5].toInt();

                m_portNumberMap.insert(pid,  INode);  //pid  , 端口信息
                m_portStatus.insert(INode,  state);
            }
        }
    }
}

void DNetDevicePrivate::updateAddr()
{
    AddrIterator iter = m_netlink->addrIterator();
    m_addrIpv4DB.clear();
    m_addrIpv6DB.clear();

    while (iter.hasNext()) {
        auto it = iter.next();
        if (it->family() == AF_INET) {
            std::shared_ptr< DNetDevice::DInetAddr4 > ip4net = std::make_shared< DNetDevice::DInetAddr4 >();
            // DNetDevice::DInetAddr4 ip4net;
            it->local();
            ip4net->family = it->family();
            ip4net->addr = it->netaddr();
            ip4net->mask = it->netmask();
            ip4net->bcast = it->broadcast();
            m_addrIpv4DB.insert(it->ifindex(), ip4net);
        } else if (it->family() == AF_INET6) {
            std::shared_ptr< DNetDevice:: DInetAddr6> ip6net = std::make_shared< DNetDevice:: DInetAddr6>();
            it->local();
            ip6net->family = it->family();
            ip6net->addr = it->netaddr();
            ip6net->scope = it->scope();
            ip6net->prefixlen = it->prefixlen();
            m_addrIpv6DB.insert(it->ifindex(), ip6net);
        }
    }
}

void DNetDevicePrivate::addDeviceInfo(hwNode &node, QList<DlsDevice::DDeviceInfo> &infoLst)
{
    struct DlsDevice::DDeviceInfo entry;

    entry.deviceInfoLstMap.clear();
    entry.devClass = DlsDevice::DtkNetwork;

    if (node.getBusInfo() != "") {
        entry.deviceBaseAttrisLst.append("SysFs_PATH");
        entry.deviceInfoLstMap["SysFs_PATH"] = QString::fromStdString(node.getBusInfo());
    }

    entry.deviceBaseAttrisLst.append("Description");
    entry.deviceInfoLstMap.insert("Description", QString::fromStdString(node.getDescription()));

    entry.deviceBaseAttrisLst.append("sysID");
    entry.deviceInfoLstMap.insert("sysID", QString::fromStdString(node.getId()));

    entry.deviceBaseAttrisLst.append("Vendor");
    entry.deviceInfoLstMap.insert("Vendor", QString::fromStdString(node.getVendor()));
    entry.vendorName = QString::fromStdString(node.getVendor());

    entry.deviceBaseAttrisLst.append("Name");
    entry.deviceInfoLstMap.insert("Name", QString::fromStdString(node.getProduct()));
    entry.productName = QString::fromStdString(node.getProduct());

    if (! node.getVendor_id().empty()) {
        entry.deviceBaseAttrisLst.append("Vendor_ID");
        entry.deviceInfoLstMap.insert("Vendor_ID", QString::fromStdString(node.getVendor_id()));
        entry.vendorID = QString::fromStdString(node.getVendor_id());
    }

    if (! node.getProduct_id().empty()) {
        entry.deviceBaseAttrisLst.append("Product_ID");
        entry.deviceInfoLstMap.insert("Product_ID", QString::fromStdString(node.getProduct_id()));
        entry.productID = QString::fromStdString(node.getProduct_id());
    }

    if (! node.getConfig("vid:pid").empty()) {
        entry.deviceBaseAttrisLst.append("VID:PID");
        entry.deviceInfoLstMap.insert("VID:PID", QString::fromStdString(node.getConfig("vid:pid")));
    }

    if (node.getModalias().length() > 53) {
        entry.deviceBaseAttrisLst.append("Modalias");
        entry.deviceInfoLstMap.insert("Modalias", QString::fromStdString(node.getModalias().substr(0, 53)));
    } else {
        entry.deviceBaseAttrisLst.append("Modalias");
        entry.deviceInfoLstMap.insert("Modalias", QString::fromStdString(node.getModalias()));
    }
    entry.deviceBaseAttrisLst.append("ClassName");
    entry.deviceInfoLstMap.insert("ClassName", QString::fromStdString(node.getClassName()));

    if (node.getLogicalName() != "") {
        vector<string> logicalnames = node.getLogicalNames();
        QString  tmps = "";

        for (unsigned int i = 0; i < logicalnames.size(); i++) {
            tmps.append(QString::fromStdString(logicalnames[i]));
            if (i >= logicalnames.size())
                break;
            tmps.append(" ");
        }
        entry.deviceBaseAttrisLst.append("LogicalName");
        entry.deviceInfoLstMap.insert("LogicalName", tmps);
    }

    if (! node.getConfig("driver").empty()) {
        entry.deviceBaseAttrisLst.append("driver");
        entry.deviceInfoLstMap.insert("driver", QString::fromStdString(node.getConfig("driver")));
    }

    if (! node.getConfig("driverversion").empty()) {
        entry.deviceBaseAttrisLst.append("driverversion");
        entry.deviceInfoLstMap.insert("driverversion", QString::fromStdString(node.getConfig("driverversion")));
    }
    if (! node.getSerial().empty()) {
        entry.deviceBaseAttrisLst.append("macAddress");
        entry.deviceInfoLstMap.insert("macAddress", QString::fromStdString(node.getSerial()));
    }

    if (! node.getConfig("proc_net_dev_data").empty()) {
        entry.deviceBaseAttrisLst.append("proc_net_dev_data");
        entry.deviceInfoLstMap.insert("proc_net_dev_data", QString::fromStdString(node.getConfig("proc_net_dev_data")));
    }
//------- ADD Children-------
    if ((hw::disk == node.getClass()) || (hw::storage == node.getClass()))
        infoLst.append(entry);
    for (int i = 0; i < node.countChildren(); i++) {
        addDeviceInfo(*node.getChild(i), infoLst);
    }
}

DNetDevice::DNetDevice(QObject *parent)
    : QObject(parent)
    , d_ptr(new DNetDevicePrivate(this))
{
}

DNetDevice::~DNetDevice()
{
}

int DNetDevice::count()
{
    Q_D(DNetDevice);
    return  d->m_listDeviceInfo.count();
}

QString DNetDevice::vendor(int index)
{
    Q_D(DNetDevice);
    if (index < d->m_listDeviceInfo.count()) {
        return  d->m_listDeviceInfo[index].vendorName;
    } else
        return QString();
}

QString DNetDevice::model(int index)
{
    Q_D(DNetDevice);
    if (index < d->m_listDeviceInfo.count()) {
        return  d->m_listDeviceInfo[index].productName;
    } else
        return QString();
}

QString DNetDevice::type(int index)
{
    Q_D(DNetDevice);
    if (index < d->m_listDeviceInfo.count())  {
        return  d->m_listDeviceInfo[index].deviceInfoLstMap.value("type");
    } else
        return QString();
}

QString DNetDevice::macAddress(int index)
{
    Q_D(DNetDevice);
    if (index < d->m_listDeviceInfo.count())  {
        return  d->m_listDeviceInfo[index].deviceInfoLstMap.value("macAddress");
    } else
        return QString();
}

QString DNetDevice::driver(int index)
{
    Q_D(DNetDevice);
    if (index < d->m_listDeviceInfo.count())  {
        return  d->m_listDeviceInfo[index].deviceInfoLstMap.value("driver");
    } else
        return QString();
}

QString DNetDevice::speed(int index)
{
    Q_D(DNetDevice);
    if (index < d->m_listDeviceInfo.count())  {
        return  d->m_listDeviceInfo[index].deviceInfoLstMap.value("speed");
    } else
        return QString();
}

QString DNetDevice::status(int index)
{
    Q_D(DNetDevice);
    if (index < d->m_listDeviceInfo.count())  {
        return  d->m_listDeviceInfo[index].deviceInfoLstMap.value("status");
    } else
        return QString();
}

DNetDevice::DInetAddr4 DNetDevice::inetAddr4(int index)
{
    Q_D(DNetDevice);
    if (d->m_addrIpv4DB.contains(index)) {
        QList< INet4Addr > values = d->m_addrIpv4DB.values(index);
        return *values.at(0);
    } else
        return DInetAddr4();
}

DNetDevice::DInetAddr6 DNetDevice::inetAddr6(int index)
{
    Q_D(DNetDevice);
    if (d->m_addrIpv6DB.contains(index)) {
        QList< INet6Addr > values = d->m_addrIpv6DB.values(index);
        return *values.at(0);
    } else
        return DInetAddr6();
}

DNetDevice::DNetifInfo DNetDevice::netifInfo(int index)
{
    //net/core/net-procfs.c  net/core/dev.c
    Q_D(DNetDevice);
    if (index < d->m_listDeviceInfo.count())  {
        struct DNetifInfo result =  DNetifInfo();

        QString line =  d->m_listDeviceInfo[index].deviceInfoLstMap.value("proc_net_dev_data");
        if (!line.isNull()) {
            QStringList deviceInfo = line.split(" ", D_SPLIT_BEHAVIOR);
            if (deviceInfo.size() > 16) { //1-17
                result.rxBytes      =   deviceInfo[1].toULongLong();     // total bytes received
                result.rxPackets    =   deviceInfo[2].toULongLong();   // total packets received
                result.rxErrors     =   deviceInfo[3].toULongLong();    // bad packets received
                result.rxDropped    =   deviceInfo[4].toULongLong();   // no space in linux buffers
                result.rxFifo       =   deviceInfo[5].toULongLong();      // FIFO overruns
                result.rxFrame      =   deviceInfo[6].toULongLong();     // frame alignment error
                // 7  compressed  //  8 multicast
                result.txBytes      =   deviceInfo[9].toULongLong();     // total bytes transmitted
                result.txPackets    =   deviceInfo[10].toULongLong();   // total packets transmitted
                result.txErrors     =   deviceInfo[11].toULongLong();    // packet transmit problems
                result.txDropped    =   deviceInfo[12].toULongLong();   // no space available in linux
                result.txFifo       =   deviceInfo[13].toULongLong();      // FIFO overruns
                // 14 collisions
                result.txCarrier    =   deviceInfo[15].toULongLong();   // loss of link pulse
            }
        }
        return result;
    } else
        return DNetifInfo();
}

QString DNetDevice::portStatus(int index, int port)
{
    Q_D(DNetDevice);
    if (index < d->m_listDeviceInfo.count())
        if (d->m_portStatus.contains(port)) {
            return d->m_portStatus.value(port);
        }
    return QString();
}

QList<int> DNetDevice::applicationPorts(int pid)
{
    Q_D(DNetDevice);
    if (d->m_portNumberMap.contains(pid)) {
        QList< int > values = d->m_portNumberMap.values(pid);
        return values;
    }
    return QList<int>();
}

DDEVICE_END_NAMESPACE
