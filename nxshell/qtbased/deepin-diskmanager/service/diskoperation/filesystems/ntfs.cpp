// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "ntfs.h"
#include "filesystem.h"
#include "utils.h"

#include <QDebug>

namespace DiskManager {

FS NTFS::getFilesystemSupport()
{
    FS fs(FS_NTFS);

    fs.busy = FS::GPARTED;

    if (!Utils::findProgramInPath("ntfsinfo").isEmpty())
        fs.read = FS::EXTERNAL;

    if (!Utils::findProgramInPath("ntfslabel").isEmpty()) {
        fs .read_label = FS::EXTERNAL ;
        fs .write_label = FS::EXTERNAL ;
        fs.write_uuid = FS::EXTERNAL;
    }

    if (!Utils::findProgramInPath("mkntfs").isEmpty()) {
        fs.create = FS::EXTERNAL;
        fs.create_with_label = FS::EXTERNAL;
    }

    //resizing is a delicate process ...
    if (!Utils::findProgramInPath("ntfsresize").isEmpty()) {
        fs.check = FS::EXTERNAL;
        fs.grow = FS::EXTERNAL;

        if (fs.read)  //needed to determine a min file system size..
            fs.shrink = FS::EXTERNAL;

        fs.move = FS::GPARTED;
    }

    if (!Utils::findProgramInPath("ntfsclone").isEmpty())
        fs.copy = FS::EXTERNAL;

    fs.online_read = FS::GPARTED;

    //Minimum NTFS partition size = (Minimum NTFS volume size) + (backup NTFS boot sector)
    //                            = (1 MiB) + (1 sector)
    // For GParted this means 2 MiB because smallest GUI unit is MiB.
    m_fsLimits.min_size = 2 * MEBIBYTE;

    return fs;
}

void NTFS::setUsedSectors( Partition & partition )
{
    QString output, error, strmatch;

    m_blocksSize = m_numOfFreeOrUsedBlocks = m_totalNumOfBlock = -1;
    QString cmd = QString("ntfsinfo --mft --force %1").arg(partition.getPath());

    if (!Utils::executCmd(cmd, output, error)) {
        strmatch = "Cluster Size:";
        int index = output.indexOf(strmatch);
        if (index >= 0 && index < output.length()) {
            m_blocksSize = Utils::regexpLabel(output, QString("(?<=Cluster Size:).*(?=\n)")).trimmed().toLong();
//             qDebug() << __FUNCTION__ << m_blocksSize << "22222222222" << endl;
        }

        strmatch = "Volume Size in Clusters:";
        index = output.indexOf(strmatch);
        if (index >= 0 && index < output.length()) {
            m_totalNumOfBlock = Utils::regexpLabel(output, QString("(?<=Volume Size in Clusters:).*(?=\n)")).trimmed().toLong();
//            qDebug() << __FUNCTION__ << m_totalNumOfBlock << "33333333333333" << endl;
        }

        strmatch = "Free Clusters:";
        index = output.indexOf(strmatch);
        if (index >= 0 && index < output.length()) {
            m_numOfFreeOrUsedBlocks = Utils::regexpLabel(output, QString("(?<=Free Clusters:).*(?=[(])")).trimmed().toLong();
//            qDebug() << __FUNCTION__ << m_numOfFreeOrUsedBlocks << "4444444444444444" << endl;
        }
    }

    if (m_blocksSize > -1 && m_totalNumOfBlock > -1 && m_numOfFreeOrUsedBlocks > -1) {
        m_totalNumOfBlock = m_totalNumOfBlock * (m_blocksSize / partition.m_sectorSize);
        m_numOfFreeOrUsedBlocks = m_numOfFreeOrUsedBlocks * (m_blocksSize / partition.m_sectorSize);

//        qDebug() << __FUNCTION__ << m_totalNumOfBlock << m_numOfFreeOrUsedBlocks << "1111111111111111111" << endl;
        partition.setSectorUsage(m_totalNumOfBlock, m_numOfFreeOrUsedBlocks);
        partition.m_fsBlockSize = m_blocksSize;
    }

}

void NTFS::readLabel( Partition & partition )
{
    QString output, error;
    if (!Utils::executCmd(QString("ntfslabel --force").arg(partition.getPath()), output, error)) {
        partition.setFilesystemLabel(output.trimmed());
    }
}

bool NTFS::writeLabel( const Partition & partition)
{
    QString output, error;
    int exitcode = Utils::executCmd(QString("ntfslabel --force %1 %2").arg(partition.getPath()).arg(partition.getFileSystemLabel()), output, error);
//    qDebug() << __FUNCTION__ << output << error;
    return exitcode == 0;
}

void NTFS::readUuid(Partition & partition)
{
}

bool NTFS::writeUuid( const Partition & partition)
{
    QString output, error;
    int exitcode = Utils::executCmd(QString("ntfslabel --new-serial ").arg(partition.getPath()), output, error);
    return exitcode == 0 || error.compare("Unknown error") == 0;
}

bool NTFS::create(const Partition & newPartition)
{
    QString output, error;
    int exitcode = -1;
    if (newPartition.getFileSystemLabel().isEmpty() || newPartition.getFileSystemLabel() == " ") {
        exitcode = Utils::executCmd(QString("mkntfs -Q -v -F %1").arg(newPartition.getPath()), output, error);
    } else {
        exitcode = Utils::executCmd(QString("mkntfs -Q -v -F %1 -L %2").arg(newPartition.getPath()).arg(newPartition.getFileSystemLabel()),output, error);
    }
    return exitcode == 0 && error.compare("Unknown error") == 0;
}

/*
Ntfsresize can be used to shrink or enlarge any NTFS filesystem located on an unmounted DEVICE (usually a disk partition).
The new filesystem will have SIZE bytes.
The SIZE parameter may have one of the optional modifiers k, M, G, which means the SIZE parameter is given in kilo-,mega- or gigabytes respectively.
Ntfsresize conforms to the SI, ATA, IEEE standards and the disk manufacturers by using k=10^3, M=10^6 and G=10^9.
Ntfsresize可用于缩小或扩大位于未安装设备（通常是磁盘分区）上的任何NTFS文件系统。
新的文件系统将具有SIZE字节。
尺寸参数可以具有可选修饰符K、M、G中的一个，这意味着尺寸参数分别以千字节、兆字节或千兆字节给出。
NTFSRESIZE使用K=10^3、M=10^6和G=10^9，符合Si、ATA、IEEE标准和磁盘制造商的要求。
*/

bool NTFS::resize(const Partition &partitionNew, bool fillPartition)
{
    double d = Utils::sectorToUnit(partitionNew.getSectorLength(), partitionNew.m_sectorSize, UNIT_BYTE);
    return  resize(partitionNew.getPath(), QString::number(((long long)d / 1000)), fillPartition);
}

bool NTFS::resize(const QString &path, const QString &sizeByte, bool fillPartition)
{
    //  bool success;
    QString output, error;
    QString size, cmd;
    if (!fillPartition) {
        size = QString(" -s %1k").arg(sizeByte);
    }

    //Utils::executCmd(QString("umount %1").arg(path));

    cmd = "ntfsresize --force --force" + size ;
    // Real resize
    cmd = QString("%1 %2").arg(cmd).arg(path);
    //    success = !Utils::executCmd(cmd, output, error);
    return !Utils::executCmd(cmd, output, error);
}

//bool ntfs::copy( const Partition & src_part, Partition & dest_part)
//{
//	return ! execute_command( "ntfsclone -f --overwrite " + Glib::shell_quote( dest_part.get_path() ) +
//	                          " " + Glib::shell_quote( src_part.get_path() ),
//	                          operationdetail,
//	                          EXEC_CHECK_STATUS|EXEC_CANCEL_SAFE|EXEC_PROGRESS_STDOUT,
//		                  static_cast<StreamSlot>( sigc::mem_fun( *this, &ntfs::clone_progress ) ) );
//}

bool NTFS::checkRepair(const Partition &partition)
{
    return checkRepair(partition.getPath());
}

bool NTFS::checkRepair(const QString &devpath)
{
    QString output, error;
    int exitcode = Utils::executCmd(QString("ntfsresize -i -f -v %1").arg(devpath), output, error);
//    qDebug() << QString("NTFS::check_repair---%1----%2").arg(output).arg(error);
    return exitcode == 0 || error.compare("Unknown error") == 0;
}

FS_Limits NTFS::getFilesystemLimits(const Partition &partition)
{
    return getFilesystemLimits(partition.getPath());
}

FS_Limits NTFS::getFilesystemLimits(const QString &path)
{
    m_fsLimits = FS_Limits {-1, -1};
    QString cmd, output, error;
    //Utils::executCmd(QString("umount %1").arg(path), output, error);
    cmd = QString("ntfsresize -m -f %1").arg(path);
    if (Utils::executCmd(cmd, output, error) != 0 && error.compare("Unknown error") != 0) {
        return m_fsLimits;
    }

    foreach (QString str, output.split("\n")) {
        // qDebug()<<"getFilesystemLimits ntfs"<<str;
        if (str.contains("Minsize (in MB):")) {
            auto list = str.split(":");
            //qDebug()<<"getFilesystemLimits ntfs list"<<list;
            if (list.count() == 2) {
                m_fsLimits.min_size = list[1].toLongLong() * 1000 * 1000; //转换为byte  
                //qDebug()<<"getFilesystemLimits ntfs min_size"<<tmp.min_size;
                m_fsLimits.max_size = 0;
                return m_fsLimits;
            }
        }
    }

    return m_fsLimits;
}

} //DiskManager
