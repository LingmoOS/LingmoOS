// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "exfat.h"
#include <QUuid>
namespace DiskManager {

FS ExFat::getFilesystemSupport()
{
    FS fs(FS_EXFAT);

    fs.busy = FS::GPARTED;
    fs.copy = FS::GPARTED;
    fs.move = FS::GPARTED;
    fs.online_read = FS::GPARTED;

    if (!Utils::findProgramInPath("dump.exfat").isEmpty())
        fs.read = FS::EXTERNAL;

    QString output, error;
    if (!Utils::findProgramInPath("mkfs.exfat").isEmpty()) {
        if (Utils::executCmd("mkfs.exfat -V", output, error) == 0)
            fs.create = FS::EXTERNAL;
    }

    if (!Utils::findProgramInPath("tune.exfat").isEmpty()) {
        fs.read_label = FS::EXTERNAL;
        fs.write_label = FS::EXTERNAL;

        // Get/set exFAT Volume Serial Number support was added to exfatprogs
        // 1.1.0.  Check the help text for the feature before enabling.
        Utils::executCmd("tune.exfat", output, error);
        if (error.indexOf("Set volume serial") < error.length()) {
            fs.read_uuid = FS::EXTERNAL;
            fs.write_uuid = FS::EXTERNAL;
        }
    }

    if (!Utils::findProgramInPath("fsck.exfat").isEmpty()) {
        Utils::executCmd("fsck.exfat -V", output, error);
        if (output.contains("exfatprogs version"))
            fs.check = FS::EXTERNAL;
    }
    fs.grow = FS::NONE;
    fs.shrink = FS::NONE;

    return fs;
}

void ExFat::setUsedSectors(DiskManager::Partition &partition)
{
    QString output, error;
    auto errCode = Utils::executCmd("dump.exfat ", output, error);
    // dump.exfat returns non-zero status for both success and failure.  Instead use
    // non-empty stderr to identify failure.
    if (!error.isEmpty() || errCode != 0) {
        return;
    }

    // Example output (lines of interest only):
    //     $ dump.exfat /dev/sdb1
    //     Volume Length(sectors):                  524288
    //     Sector Size Bits:                        9
    //     Sector per Cluster bits:                 3
    //     Free Clusters: 	                        23585
    //
    // "exFAT file system specification"
    // https://docs.microsoft.com/en-us/windows/win32/fileio/exfat-specification
    // Section 3.1.5 VolumeLength field
    // Section 3.1.14 BytesPerSectorShift field
    // Section 3.1.15 SectorsPerClusterShift field

    // FS size in [FS] sectors
    long long volume_length = -1;
    int index = output.indexOf("Volume Length(sectors):");
    if (index < output.length())
        sscanf(output.mid(index).toStdString().c_str(), "Volume Length(sectors): %lld", &volume_length);

    // FS sector size = 2^(bytes_per_sector_shift)
    long long bytes_per_sector_shift = -1;
    index = output.indexOf("Sector Size Bits:");
    if (index < output.length())
        sscanf(output.mid(index).toStdString().c_str(), "Sector Size Bits: %lld", &bytes_per_sector_shift);

    // Cluster size = sector_size * 2^(sectors_per_cluster_shift)
    long long sectors_per_cluster_shift = -1;
    index = output.indexOf("Sector per Cluster bits:");
    if (index < output.length())
        sscanf(output.mid(index).toStdString().c_str(), "Sector per Cluster bits: %lld",
               &sectors_per_cluster_shift);

    // Free clusters
    long long free_clusters = -1;
    index = output.indexOf("Free Clusters:");
    if (index < output.length())
        sscanf(output.mid(index).toStdString().c_str(), "Free Clusters: %lld", &free_clusters);

    if (volume_length > -1 && bytes_per_sector_shift > -1 &&
        sectors_per_cluster_shift > -1 && free_clusters > -1) {
        Byte_Value sector_size = 1 << bytes_per_sector_shift;
        Byte_Value cluster_size = sector_size * (1 << sectors_per_cluster_shift);
        Sector fs_free = free_clusters * cluster_size / partition.m_sectorSize;
        Sector fs_size = volume_length * sector_size / partition.m_sectorSize;
        partition.setSectorUsage(fs_size, fs_free);
        partition.m_fsBlockSize = cluster_size;
    }
}

void ExFat::readLabel(DiskManager::Partition &partition)
{
    QString output, error;
    auto errCode = Utils::executCmd(QString("tune.exfat -l %1").arg(partition.getPath()), output, error);
    if (errCode != 0) {
        return;
    }

    partition.setFilesystemLabel(Utils::regexpLabel(output, "^label: ([^\n]*)"));
}

bool ExFat::writeLabel(const DiskManager::Partition &partition)
{
    QString cmd, output, error;
    if (!partition.getFileSystemLabel().isEmpty() && partition.getFileSystemLabel() != " ") {
        cmd = QString("tune.exfat -L %1 %2").arg(partition.getFileSystemLabel()).arg(partition.getPath());
        auto errCode = Utils::executCmd(cmd, output, error);
        return errCode == 0;
    }
    return true;
}

void ExFat::readUuid(DiskManager::Partition &partition)
{
    QString output, error;
    auto errCode = Utils::executCmd(QString("tune.exfat -i %1").arg(partition.getPath()), output, error);
    if (errCode != 0) {
        return;
    }
    partition.m_uuid = serial2BlkidUuid(
            Utils::regexpLabel(output, "volume serial : (0x[[:xdigit:]][[:xdigit:]]*)"));
}

bool ExFat::writeUuid(const DiskManager::Partition &partition)
{
    QString output, error;
    auto errCode = Utils::executCmd(QString("tune.exfat -I %1 %2")
                                            .arg(randomSerial())
                                            .arg(partition.getPath()), output, error);
    return errCode == 0;
}

bool ExFat::create(const DiskManager::Partition &new_partition)
{
    QString cmd, output, error;
    if (new_partition.getFileSystemLabel().isEmpty() || new_partition.getFileSystemLabel() == " ") {
        cmd = QString("mkfs.exfat %1").arg(new_partition.getPath());
    } else {
        cmd = QString("mkfs.exfat -n %1 %2").arg(new_partition.getFileSystemLabel()).arg(new_partition.getPath());
    }
    auto errCode = Utils::executCmd(cmd, output, error);
    return errCode == 0;
}

bool ExFat::resize(const DiskManager::Partition &partitionNew, bool fillPartition)
{
    return FileSystem::resize(partitionNew, fillPartition);
}

bool DiskManager::ExFat::resize(const QString &path, const QString &size, bool fillPartition)
{
    return FileSystem::resize(path, size, fillPartition);
}

bool DiskManager::ExFat::checkRepair(const DiskManager::Partition &partition)
{
    QString path = partition.getPath();
    return checkRepair(path);
}

bool DiskManager::ExFat::checkRepair(const QString &devpath)
{
    QString output, error;
    auto errCode = Utils::executCmd(QString("fsck.exfat %1").arg(devpath), output, error);
    return errCode == 0;
}

FS_Limits DiskManager::ExFat::getFilesystemLimits(const DiskManager::Partition &partition)
{
    m_fsLimits.min_size = -1;
    m_fsLimits.max_size = -1;
    return m_fsLimits;
}

QString DiskManager::ExFat::serial2BlkidUuid(QString serial)
{
    QString verifiedSerial = Utils::regexpLabel(serial, "^(0x[[:xdigit:]][[:xdigit:]]*)$");
    if (verifiedSerial.isEmpty())
        return verifiedSerial;

    QString canonicalUuid = verifiedSerial.mid(2, 4).toUpper() + "-" +
                            verifiedSerial.mid(6, 4).toUpper();
    return canonicalUuid;
}

QString DiskManager::ExFat::randomSerial()
{
    return "0x" + QUuid::createUuid().toString().mid(0, 8);
}

}