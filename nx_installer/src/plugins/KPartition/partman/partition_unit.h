#ifndef PARTITION_UNIT_H
#define PARTITION_UNIT_H

#include <QObject>
#include <QMap>
#include "device.h"
#include "filesystem.h"
#include "partition.h"

namespace KInstaller {

namespace Partman {
enum Status {
    None,
    New,
    Copy,
    Restore
};
static const QMap<FSType, QString> FSFormatCmdMap {
    {FSType::BtrFS, QString("mkfs.btrfs")},
    {FSType::EFI, QString("mkfs.vfat")},
    {FSType::Ext2, QString("mkfs.ext2")},
    {FSType::Ext3, QString("mkfs.ext3")},
    {FSType::Ext4, QString("mkfs.ext4")},
    {FSType::LingmoData, QString("mkfs.ext4")},
    {FSType::Fat12, QString("mkfs.fat")},
    {FSType::Fat16, QString("mkfs.fat")},
    {FSType::Fat32, QString("mkfs.fat")},
    {FSType::NtFS, QString("mkfs.ntfs")},
    {FSType::ReiserFS, QString("mkfs.reiserfs")},
    {FSType::Reiser4, QString("mkfs.reiser4")},
    {FSType::XFS, QString("mkfs.xfs")},
    {FSType::LinuxSwap, QString("mkswap")},
    {FSType::Lvm2_PV, QString("lvm")},
    {FSType::Jfs, QString("mkfs.jfs")}
};

void refreshOSProberItems();
void removeOSProberDataByPath(const QString& path);


QString getPartitionLabel(const Partition::Ptr partition);
QString getPartitionName(const QString& path);
QString getPartitionLabelAndPath(const Partition::Ptr partition);

QString trimText(const QString& text, int max_len);

void alignPartition(Partition::Ptr partition);
int allocLogicalPartitionNumber(Device::Ptr dev);
int allocPrimaryPartitionNumber(Device::Ptr dev);
DeviceList filterInstallerDevice(const DeviceList& devices);
PartitionList filterFragmentationPartition(PartitionList partitions);

FSType getDefaultFSType();
QString getDeviceModelAndCap(Device::Ptr dev);
QString getDeviceModelCapAndPath(Device::Ptr dev);

QString getInstallerDevicePath();
QString getLocalFSTypeName(FSType fs);

QString getPartitionUsage(Partition::Ptr partition);
int getPartitionUsageValue(Partition::Ptr partition);

int checkSpecialModel();
bool checkLoaderAndPartitionTable(Device::Ptr dev);

bool isEFIEnabled();
bool isMBRPreferred(const DeviceList& devices);

bool isMountPointSupported(FSType fs);

bool isPartitionTableMatch(PartTableType type);
bool isPartitionTableMatch(const DeviceList& devices, const QString& devpath);

bool isSwapAreaNeeded();
int toGigByte(qint64 size);
int toMebiByte(qint64 size);

Partition::Ptr createEmptyPartition(QString devpath, qint64 sectorsize, qint64 sectorstart, qint64 sectorend);
qint64 parsePartitionSize(const QString& size, qint64 devlength);

bool getMips();
////针对不同的硬件设备定制机器需要判断的情况
//DeviceList getDeviceListByDeviceModel(DeviceList devs);
//针对ventoy进行特殊判断 2021-3-9
QString getVentoyDevPath();
}

}


#endif // PARTITION_UNIT_H
