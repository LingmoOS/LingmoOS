#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QString>
#include <QFileInfo>
#include <QSharedPointer>
#include "datastruct.h"
#include "partition.h"

namespace KInstaller {
namespace Partman {
class Device
{
public:
    explicit Device();
    QString getDevicePath();
    qint64 getByteLength();
    qint64 getDiskUsedPercent();

public:
    typedef QSharedPointer<Device> Ptr;

    QString m_model;//硬盘设备信息
    QString m_osName;//硬盘中已安装操作系统信息
    QString m_path;// /dev/sda
    qint64 m_sectorSize;//每个扇区大小512
    qint64 m_sectors;//

    qint64 m_length;//扇区的总数量
    qint64 m_heads;//开头的位置
    qint64 m_cylinders;//圆柱
    qint64 m_cysize;//
    qint64 m_diskFreeSpace;
    qint64 m_diskTotal;
    int m_readyOnly;//
    PartTableType m_partTableType;//分区表类型

    PartitionList partitions;//分区数量和信息

    int m_maxPrims;//主分区数量



};
typedef QList<Device::Ptr> DeviceList;
int deviceIndex(const DeviceList& devices, const QString& devPath);
}
}
#endif // DEVICE_H
