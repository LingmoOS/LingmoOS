// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "xfs.h"
#include "filesystem.h"
#include "utils.h"
#include <QDebug>
#define RFC4122_NONE_NIL_UUID_REGEXP "[[:xdigit:]]{8}-[[:xdigit:]]{4}-[1-9a-fA-F][[:xdigit:]]{3}-[[:xdigit:]]{4}-[[:xdigit:]]{12}"
namespace DiskManager {
FS XFS::getFilesystemSupport()
{
    FS fs(FS_XFS);
    fs.busy = FS::GPARTED;
    if (!Utils::findProgramInPath("xfs_db").isEmpty()) {
        fs.read = FS::EXTERNAL;
        fs.read_label = FS::EXTERNAL;
    }
    if (!Utils::findProgramInPath("xfs_admin").isEmpty()) {
        fs.write_label = FS::EXTERNAL;
        fs.read_uuid = FS::EXTERNAL;
        fs.write_uuid = FS::EXTERNAL;
    }
    if (!Utils::findProgramInPath("mkfs.xfs").isEmpty()) {
        fs.create = FS::EXTERNAL;
        fs.create_with_label = FS::EXTERNAL;
    }
    if (!Utils::findProgramInPath("xfs_repair").isEmpty())
        fs.check = FS::EXTERNAL;
    //Mounted operations require mount, umount and xfs support in the kernel
    if (!Utils::findProgramInPath("mount").isEmpty() && !Utils::findProgramInPath("umount").isEmpty()
        && fs.check /*&& Utils::kernel_supports_fs("xfs")*/) {
        //Grow
        if (!Utils::findProgramInPath("xfs_growfs").isEmpty())
            fs.grow = FS::EXTERNAL;
        //Copy using xfsdump, xfsrestore
        if (!Utils::findProgramInPath("xfsdump").isEmpty() && !Utils::findProgramInPath("xfsrestore").isEmpty() && fs.create)
            fs.copy = FS::EXTERNAL;
    }
    if (fs.check)
        fs.move = FS::GPARTED;
    fs.online_read = FS::GPARTED;

    return fs;
}


void XFS::setUsedSectors(Partition &partition)
{
    QString output, error, strmatch, strcmd;
    m_blocksSize = m_numOfFreeOrUsedBlocks = m_totalNumOfBlock = -1;
    strcmd = QString("xfs_db -r -c \"sb 0\" -c \"print blocksize\" -c \"print dblocks\""
                     " -c \"print fdblocks\" %1")
                 .arg(partition.getPath());
    if (Utils::executWithInputOutputCmd(strcmd, NULL, output, error) == 0) {
        auto strList = output.split("\n");
        foreach (auto &item, strList) {
            auto value = item.split("=", QString::SkipEmptyParts);
            if (value.size() != 2) {
                continue;
            }
            if (value[0].startsWith("blocksize")) {
                m_blocksSize = value[1].trimmed().toULongLong();
            } else if (value[0].startsWith("dblocks")) {
                m_totalNumOfBlock = value[1].trimmed().toULongLong();
            } else if (value[0].startsWith("fdblocks")) {
                m_numOfFreeOrUsedBlocks = value[1].trimmed().toULongLong();
            } else {
                continue;
            }
        }

        if (m_totalNumOfBlock > -1 && m_numOfFreeOrUsedBlocks > -1 && m_blocksSize > -1) {
            m_totalNumOfBlock = qRound64(m_totalNumOfBlock * (m_blocksSize / double(partition.m_sectorSize)));
            m_numOfFreeOrUsedBlocks = qRound64(m_numOfFreeOrUsedBlocks * (m_blocksSize / double(partition.m_sectorSize)));
            partition.setSectorUsage(m_totalNumOfBlock, m_numOfFreeOrUsedBlocks);
            partition.m_fsBlockSize = m_blocksSize;
        }
    } else {
        qDebug() << __FUNCTION__ << "dumpe2fs -h failed :" << output << error;
    }
}
void XFS::readLabel(Partition &partition)
{
    QString output, error;
    if (!Utils::executCmd(QString("xfs_db -r -c label %1").arg(partition.getPath()), output, error)) {
        auto items = output.split("=");
        if (items.size() == 2)
            partition.setFilesystemLabel(items[1].replace("\"", "").trimmed());
    }
}
bool XFS::writeLabel(const Partition &partition)
{
    QString cmd = "";
    if (partition.getFileSystemLabel().isEmpty())
        cmd = QString("xfs_admin -L -- %1").arg(partition.getPath());
    else
        cmd = QString("xfs_admin -L %1 %2").arg(partition.getFileSystemLabel()).arg(partition.getPath());
    QString output, error;
    int exitcode = Utils::executCmd(cmd, output, error);
    return exitcode == 0;
}
void XFS::readUuid(Partition &partition)
{
    QString output, error;
    if (!Utils::executCmd(QString("xfs_admin -u %1").arg(partition.getPath()), output, error)) {
        partition.m_uuid = Utils::regexpLabel(output, "^UUID[[:blank:]]*=[[:blank:]]*(" RFC4122_NONE_NIL_UUID_REGEXP ")");
    }
}
bool XFS::writeUuid(const Partition &partition)
{
    QString output, error;
    int exitcode = Utils::executCmd(QString("xfs_admin -U generate %1").arg(partition.getPath()), output, error);
    return exitcode == 0 || error.compare("Unknown error") == 0;
}
bool XFS::create(const Partition &newPartition)
{
    QString output, error;
    int exitcode = -1;
    if (newPartition.getFileSystemLabel().isEmpty() || newPartition.getFileSystemLabel().trimmed().isEmpty()) {
        exitcode = Utils::executCmd(QString("mkfs.xfs -f %1").arg(newPartition.getPath()), output, error);
    } else {
        exitcode = Utils::executCmd(QString("mkfs.xfs -f -L %1 %2").arg(newPartition.getFileSystemLabel()).arg(newPartition.getPath()), output, error);
    }
    return exitcode == 0 && error.compare("Unknown error") == 0;
}
/*
XFSresize can be used to shrink or enlarge any XFS filesystem located on an unmounted DEVICE (usually a disk partition).
The new filesystem will have SIZE bytes.
The SIZE parameter may have one of the optional modifiers k, M, G, which means the SIZE parameter is given in kilo-,mega- or gigabytes respectively.
XFSresize conforms to the SI, ATA, IEEE standards and the disk manufacturers by using k=10^3, M=10^6 and G=10^9.
XFSresize可用于缩小或扩大位于未安装设备（通常是磁盘分区）上的任何XFS文件系统。
新的文件系统将具有SIZE字节。
尺寸参数可以具有可选修饰符K、M、G中的一个，这意味着尺寸参数分别以千字节、兆字节或千兆字节给出。
XFSRESIZE使用K=10^3、M=10^6和G=10^9，符合Si、ATA、IEEE标准和磁盘制造商的要求。
*/
bool XFS::resize(const Partition &partitionNew, bool fillPartition)
{
    Q_UNUSED(fillPartition);
    bool success = true ;

    QString mountPoint;
    QString output, error;
    if (!partitionNew.m_busy) {
        mountPoint = Utils::mkTempDir("");
        if (mountPoint.isEmpty())
            return false ;
        success &= Utils::executCmd(QString("mount -v -t xfs %1 %2")
                .arg(partitionNew.getPath())
                .arg(mountPoint),output, error) == 0;
    } else {
        mountPoint = partitionNew.getMountPoint();
    }

    if (success) {
        success &= Utils::executCmd(QString("xfs_growfs %1").arg(mountPoint),
                                    output, error) == 0;

        if (partitionNew.m_busy) {
            success &= Utils::executCmd(QString("umount -v %1").arg(mountPoint), output, error) == 0;
        }

    }

    if (!partitionNew.m_busy) {
        Utils::rmTempDir(mountPoint) ;
    }
    return success ;
}

bool XFS::checkRepair(const Partition &partition)
{
    return checkRepair(partition.getPath());
}
bool XFS::checkRepair(const QString &devpath)
{
    QString output, error;
    int exitcode = Utils::executCmd(QString("xfs_repair -v %1").arg(devpath), output, error);
    return exitcode == 0 || error.compare("Unknown error") == 0;
}

FS_Limits XFS::getFilesystemLimits(const Partition &partition)
{
    // Official minsize = 16MB, but the smallest xfs_repair can handle is 32MB.
    //m_fsLimits.min_size = 32 * MEBIBYTE;
    m_fsLimits.min_size = -1;
    m_fsLimits.max_size = -1;
    return m_fsLimits;
}
} // namespace DiskManager
