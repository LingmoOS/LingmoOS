// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later
#include "ddiskdevice.h"
#include "dlsdevice_p.h"

#include "scan.h"
#include "hw.h"


#include <QDebug>
#include <QtCore>
#include <QDebug>

DDEVICE_BEGIN_NAMESPACE

class DDiskDevicePrivate
{

public:
    explicit DDiskDevicePrivate(DDiskDevice *parent)
        : m_hwNode("computer", hw::sys_tem)
        , q_ptr(parent)
    {

        m_listDeviceInfo.clear();
        scan_system(m_hwNode);
        addDeviceInfo(m_hwNode, m_listDeviceInfo);
    }

    hwNode                    m_hwNode ;
    QList< DlsDevice::DDeviceInfo >      m_listDeviceInfo;
    void addDeviceInfo(hwNode &node, QList< DlsDevice::DDeviceInfo >  &infoLst);
    Q_DECLARE_PUBLIC(DDiskDevice)

private:
    DDiskDevice *q_ptr = nullptr;
};

void DDiskDevicePrivate::addDeviceInfo(hwNode &node, QList<DlsDevice::DDeviceInfo> &infoLst)
{
    DlsDevice::DDeviceInfo entry;

    entry.deviceInfoLstMap.clear();
    entry.devClass = DlsDevice::DtkDisk;

    entry.sysFsPath = QString::fromStdString(node.getSysFS_Path());
    entry.deviceBaseAttrisLst.append("Description");
    entry.deviceInfoLstMap.insert("Description", QString::fromStdString(node.getDescription()));

    entry.deviceBaseAttrisLst.append("PhysId");
    entry.deviceInfoLstMap.insert("PhysId", QString::fromStdString(node.getPhysId()));

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

    vector < string > config;
    config = node.getConfigValues("=");

    if (config.size() > 0) {
        QString  tmps = "";
        for (unsigned int i = 0; i < config.size(); i++) {
            tmps.append(QString::fromStdString(config[i]));
            if (i >= config.size())
                break;
            tmps.append(" ");
        }
        entry.deviceBaseAttrisLst.append("Configuration");
        entry.deviceInfoLstMap.insert("Configuration", tmps);
    }

    vector < string > resources;
    resources = node.getResources(":");
    if (resources.size() > 0) {
        QString  tmps = "";
        for (unsigned int i = 0; i < resources.size(); i++) {
            tmps.append(QString::fromStdString(resources[i]));
            if (i >= resources.size())
                break;
            tmps.append(" ");
        }
        entry.deviceBaseAttrisLst.append("Resources");
        entry.deviceInfoLstMap.insert("Resources", tmps);
    }

    if (! node.getConfig("interface").empty()) {
        entry.deviceBaseAttrisLst.append("interface");
        entry.deviceInfoLstMap.insert("interface", QString::fromStdString(node.getConfig("interface")));
    }

    if (! node.getConfig("serialNumber").empty()) {
        entry.deviceBaseAttrisLst.append("serialNumber");
        entry.deviceInfoLstMap.insert("serialNumber", QString::fromStdString(node.getConfig("serialNumber")));
    }

    if (! node.getConfig("Revison").empty()) {
        entry.deviceBaseAttrisLst.append("Revison");
        entry.deviceInfoLstMap.insert("Revison", QString::fromStdString(node.getVersion()));
    }

    if (! node.getConfig("SectorSize").empty()) {
        entry.deviceBaseAttrisLst.append("SectorSize");
        entry.deviceInfoLstMap.insert("SectorSize", QString::fromStdString(node.getConfig("SectorSize")));
    }

    if (! node.getConfig("Temperature").empty()) {
        entry.deviceBaseAttrisLst.append("Temperature");
        entry.deviceInfoLstMap.insert("Temperature", QString::fromStdString(node.getConfig("Temperature")));
    }


    if (! node.getConfig("rotation").empty()) {
        entry.deviceBaseAttrisLst.append("Rotation");
        entry.deviceInfoLstMap.insert("Rotation", QString::fromStdString(node.getConfig("rotation")));
    }

    if (! node.getConfig("mediatype").empty()) {
        entry.deviceBaseAttrisLst.append("mediaType");
        entry.deviceInfoLstMap.insert("mediaType", QString::fromStdString(node.getConfig("mediatype")));
    }
    if (node.getSize() > 0) {
        string size = to_string(node.getSize());

        entry.deviceBaseAttrisLst.append("Size");
        entry.deviceInfoLstMap.insert("Size",  QString::fromStdString(size));
    }
//--------------------------------ADD Children---------------------
    if ((hw::disk == node.getClass()) || (hw::storage == node.getClass()))
        infoLst.append(entry);
    for (int i = 0; i < node.countChildren(); i++) {
        addDeviceInfo(*node.getChild(i), infoLst);
    }
}

DDiskDevice::DDiskDevice(QObject *parent)
    : QObject(parent)
    , d_ptr(new DDiskDevicePrivate(this))
{
}

DDiskDevice::~DDiskDevice()
{
}

int DDiskDevice::count()
{
    Q_D(DDiskDevice);
    return  d->m_listDeviceInfo.count();
}

QString DDiskDevice::vendor(int index)
{
    Q_D(DDiskDevice);
    if (index < d->m_listDeviceInfo.count()) {
        return  d->m_listDeviceInfo[index].vendorName;
    } else
        return QString();
}

QString DDiskDevice::model(int index)
{
    Q_D(DDiskDevice);
    if (index < d->m_listDeviceInfo.count()) {
        return  d->m_listDeviceInfo[index].productName;
    } else
        return QString();
}

QString DDiskDevice::mediaType(int index)
{
    Q_D(DDiskDevice);
    if (index < d->m_listDeviceInfo.count())  {
        return  d->m_listDeviceInfo[index].deviceInfoLstMap.value("mediatype");
    } else
        return QString();
}

QString DDiskDevice::size(int index)
{
    Q_D(DDiskDevice);
    if (index < d->m_listDeviceInfo.count()) {
        return  d->m_listDeviceInfo[index].deviceInfoLstMap.value("Size");
    } else
        return QString();
}

QString DDiskDevice::interface(int index)
{
    Q_D(DDiskDevice);
    if (index < d->m_listDeviceInfo.count())  {
        return  d->m_listDeviceInfo[index].deviceInfoLstMap.value("interface");
    } else
        return QString();
}

QString DDiskDevice::serialNumber(int index)
{
    Q_D(DDiskDevice);
    if (index < d->m_listDeviceInfo.count()) {
        return  d->m_listDeviceInfo[index].deviceInfoLstMap.value("serialNumber");
    } else
        return QString();
}

QString DDiskDevice::deviceFile(int index)
{
    Q_D(DDiskDevice);
    if (index < d->m_listDeviceInfo.count()) {
        return  d->m_listDeviceInfo[index].deviceInfoLstMap.value("LogicalName");
    } else
        return QString();
}

QString DDiskDevice::rotationRate(int index)
{
    Q_D(DDiskDevice);
    if (index < d->m_listDeviceInfo.count())  {
        return  d->m_listDeviceInfo[index].deviceInfoLstMap.value("Rotation");
    } else
        return QString();
}

QString DDiskDevice::firmwareVersion(int index)
{
    Q_D(DDiskDevice);
    if (index < d->m_listDeviceInfo.count()) {
        return  d->m_listDeviceInfo[index].deviceInfoLstMap.value("Revison");
    } else
        return QString();
}

QString DDiskDevice::sectorSize(int index)
{
    Q_D(DDiskDevice);
    if (index < d->m_listDeviceInfo.count())  {
        return  d->m_listDeviceInfo[index].deviceInfoLstMap.value("SectorSize");
    } else
        return QString();
}

QString DDiskDevice::temperature(int index)
{
    Q_D(DDiskDevice);
    if (index < d->m_listDeviceInfo.count())  {
        return  d->m_listDeviceInfo[index].deviceInfoLstMap.value("Temperature");
    } else
        return QString();
}

DDiskDevice::DDiskIoStat DDiskDevice::diskIoStat(int index)
{
    Q_D(DDiskDevice);
    if (index < d->m_listDeviceInfo.count()) {
        QString statFile = (d->m_listDeviceInfo[index].sysFsPath) + QString("/stat");

        QFile file(statFile);
        if (file.exists() && file.isReadable()) {
            //open and read
            if (!file.open(QIODevice::ReadOnly)) {
                return DDiskIoStat();
            }

            struct DDiskIoStat result = DDiskIoStat();
            QTextStream stream(&file);
            QString line = stream.readLine();
            if (!line.isNull()) {
                QStringList deviceInfo = line.split(" ", D_SPLIT_BEHAVIOR);
                if (deviceInfo.size() > 16) { //1-17
                    result.readsCompletedSuccessfully = deviceInfo[0].toULongLong();     // # of reads completed successfully
                    result.readsMerged      = deviceInfo[1].toULongLong();                    // # of reads merged
                    result.readSectors      = deviceInfo[2].toULongLong();                    // # of sectors read
                    result.spentReadingTime = deviceInfo[3].toULongLong();               // # of time spent reading (ms)
                    result.writesCompleted  = deviceInfo[4].toULongLong();                // # of writes completed
                    result.writesMerged     = deviceInfo[5].toULongLong();                   // # of writes merged
                    result.writtenSectors   = deviceInfo[6].toULongLong();                 // # of sectors written
                    result.spentWritingTime = deviceInfo[7].toULongLong();               // # of time spent writing (ms)
                    result.currentProgressIOs           = deviceInfo[8].toULongLong();             // # of I/Os currently in progress
                    result.spentDoingIOsTime            = deviceInfo[9].toULongLong();              // # of time spent doing I/Os (ms)
                    result.spentDoingIOsWeightedTime    = deviceInfo[10].toULongLong();      // # of weighted time spent doing I/Os (ms)
                    result.discardsCompletedSuccessfully = deviceInfo[11].toULongLong();  // # of discards completed successfully
                    result.discardsMerged               = deviceInfo[12].toULongLong();                 // # of discards merged
                    result.discardedSectors             = deviceInfo[13].toULongLong();               // # of sectors discarded
                    result.spentDiscardingTime          = deviceInfo[14].toULongLong();            // # of time spent discarding
                                                                                        // number of flush I/Os processed 
                                                                                        // total wait time for flush requests 
                    /*
                //https://www.kernel.org/doc/html/latest/block/stat.html
                //https://www.kernel.org/doc/Documentation/iostats.txt
                // ~$ cat /proc/diskstats 
                //   8       0 sda 2571943 367281 23514413 1129819 18977 178405 1578864 341783 0 1034492 1481250 0 0 0 0 196 9647
                //~$ cat /sys/block/sda/stat
                //                 2571928 367281 23514404 1129656 18977 178405 1578864 341783 0 1034320 1481086 0 0 0 0 196 9647
                | Name            | units        | description                                     |
                | --------------- | ------------ | ----------------------------------------------- |
                | read I/Os       | requests     | number of read I/Os processed                   |0
                | read merges     | requests     | number of read I/Os merged with in-queue I/O    |1
                | read sectors    | sectors      | number of sectors read                          |2
                | read ticks      | milliseconds | total wait time for read requests               |3
                | write I/Os      | requests     | number of write I/Os processed                  |4
                | write merges    | requests     | number of write I/Os merged with in-queue I/O   |5
                | write sectors   | sectors      | number of sectors written                       |6
                | write ticks     | milliseconds | total wait time for write requests              |7
                | in_flight       | requests     | number of I/Os currently in flight              |8
                | io_ticks        | milliseconds | total time this block device has been active    |9
                | time_in_queue   | milliseconds | total wait time for all requests                |10
                | discard I/Os    | requests     | number of discard I/Os processed                |11
                | discard merges  | requests     | number of discard I/Os merged with in-queue I/O |12
                | discard sectors | sectors      | number of sectors discarded                     |13
                | discard ticks   | milliseconds | total wait time for discard requests            |14
                | flush I/Os      | requests     | number of flush I/Os processed                  |15
                | flush ticks     | milliseconds | total wait time for flush requests              |16
                    */
                }
                file.close();
            }
            return result;
        }
    }
    return DDiskIoStat();
}
    //https://www.kernel.org/doc/html/latest/block/stat.html
    //https://www.kernel.org/doc/Documentation/iostats.txt


DDEVICE_END_NAMESPACE
