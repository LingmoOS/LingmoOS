#include "partition_usage.h"
#include "datastruct.h"
#include "../PluginService/kcommand.h"
#include "../PluginService/sysInfo/swaps.h"
#include "../PluginService/base_unit/string_unit.h"

#include <QDebug>

using namespace KServer;
namespace KInstaller {
namespace Partman {
qint64 parseBtrFSUnit(const QString& value)
{
    const float pref = regexpLabel("^(\\d+\\.?\\d+", value).toFloat();
    if(value.contains("KiB")) {
        return static_cast<qint64>(pref * kKibiByte);
    }
    if(value.contains("MiB")) {
        return static_cast<qint64>(pref * kMebiByte);
    }
    if(value.contains("GiB")) {
        return static_cast<qint64>(pref * kGibiByte);
    }
    if(value.contains("TiB")) {
        return static_cast<qint64>(pref * kTebiByte);
    }
    if(value.contains("PiB")) {
        return static_cast<qint64>(pref * kPebiByte);
    }
    return -1;
}

bool readBtrFSUsage(const QString& partitionpath, qint64& freeface, qint64& total)
{
    QString output = "";
    QString dir = "";
    QString error = "";
    int exitcode = 0;
    if(!KServer::KCommand::getInstance()->RunScripCommand(
                "btrfs", {"filesystem", "show", partitionpath}, dir, output, error, exitcode)) {
        return false;
    }
    QString totalStr = "";
    QString usedStr = "";
    for(const QString&line : output.split('\n')) {
        if(line.contains(partitionpath)) {
            totalStr = regexpLabel("size\\s*([^\\s]*)\\s", line);
        } else if(line.contains("Total devices")) {
            usedStr = regexpLabel("used\\s*([^\\s]*)", line);
        }
    }
    total = parseBtrFSUnit(totalStr);
    freeface = total - parseBtrFSUnit(usedStr);
    return (total > -1 && freeface >-1);
}

bool readExt2Usage(const QString& partitionpath, qint64& freeface, qint64& total)
{
    QString output = "";
    QString dir = "";
    QString error = "";
    int exitcode = 0;
    if(!KServer::KCommand::getInstance()->RunScripCommand(
                "dumpe2fs", {"-h", partitionpath}, dir, output, error, exitcode)) {
        return false;
    }
    int blockSize = 0;
    qint64 totalBlocks = 0;
    qint64 freeBlocks = 0;

    for(const QString& line : output.split('\n')) {
        if(line.contains("Block count:")) {
            const QString m = regexpLabel("Block count:\\s+(\\d+)", line);
            if(!m.isEmpty()) {
                totalBlocks = m.toLongLong();
            }
        } else if(line.contains("Free blocks:")) {
            const QString m = regexpLabel("Free blocks:\\s+(\\d+)", line);
            if(!m.isEmpty()) {
                freeBlocks = m.toLongLong();
            }
        } else if(line.contains("Block size:")) {
            const QString m =regexpLabel("Block size:\\s+(\\d+)", line);
            if(!m.isEmpty()) {
                blockSize = m.toInt();
            }
        }
    }
    freeface = blockSize * freeBlocks;
    total = blockSize * totalBlocks;
    return true;
}

bool readFat16Usage(const QString& partitionpath, qint64& freeface, qint64& total)
{
    QString output = "";
    QString dir = "";
    QString error = "";
    int exitcode = 0;
    KServer::KCommand::getInstance()->RunScripCommand(
                    "dosfsck", {"-n", "-v", partitionpath}, dir, output, error, exitcode);
    if(!error.isEmpty()) {
        qWarning() << "dosfsck failed:" << error;
        return false;
    }

    int clusterSize = 0;
    qint64 startByte = 0;
    qint64 totalCluster = 0;
    qint64 usedCluster = 0;

    for(const QString& line : output.split('\n')) {
        if(line.contains("bytes per cluster")) {
            clusterSize =  (line.trimmed().split(' ').at(0)).trimmed().toInt();
        } else if(line.contains("Data area starts at")) {
            startByte = line.split(' ').at(5).toLongLong();
        } else if(line.contains(partitionpath)) {
            const QStringList parts = line.split(' ').at(3).split('/');
            totalCluster = parts.at(1).toLongLong();
            usedCluster = parts.at(0).toLongLong();
        }
    }

   total =totalCluster * clusterSize;
   freeface = total - startByte - usedCluster * clusterSize;
   return true;
}

bool readLinuxSwapUsage(const QString& partitionpath, qint64& freeface, qint64& total)
{
    const SwapItemList swapItems = parseSwaps();
    for(const SwapItem& item : swapItems) {
        if(item.filename == partitionpath) {
            total = item.size;
            freeface = item.size - item.used;
            return true;
        }
    }
    freeface = 0;
    total = 0;
    return true;
}

bool readJfsUsage(const QString& partitionpath, qint64& freeface, qint64& total)
{
    QString output = "";
    QString dir = "";
    QString error = "";
    int exitcode = 0;
    const QString param(QString("echo dm | jfs_debugfs %1").arg(partitionpath));
    if(!KServer::KCommand::getInstance()->RunScripCommand("sh", {"-c", param}, dir, output, error, exitcode)) {
        return false;
    }

    int blockSize = 0;
    qint64 totalBlocks = 0;
    qint64 freeBlocks = 0;
    for(const QString& line : output.split('\n')) {
        if(line.startsWith("Aggregate Block Size:")) {
            blockSize = line.split(':').at(1).trimmed().toInt();
        } else if(line.contains("dn_mapsize:")) {
            const QString item = regexpLabel("dn_mapsize:\\s*([^\\s]+)", line);
            totalBlocks = item.toLongLong(nullptr, 16);
        } else if(line.contains("dn_nfree:")) {
            const QString item = regexpLabel("dn_nfree:\\s*([^\\s]+)", line);
            freeBlocks = item.toLongLong(nullptr, 16);
        }
    }
    if(freeBlocks > 0 && totalBlocks > 0 && blockSize > 0) {
        freeface = freeBlocks * blockSize;
        total = totalBlocks * blockSize;
        return true;
    } else {
        return false;
    }
    return true;
}

bool readReiser4Usage(const QString& partitionpath, qint64& freeface, qint64& total)
{
    QString output = "";
    QString dir = "";
    QString error = "";
    int exitcode = 0;
    if(!KServer::KCommand::getInstance()->RunScripCommand("debugfs.reiser4",
                     {"--force", "--yes", partitionpath}, dir, output, error, exitcode)) {
        return false;
    }
    int blockSize = 0;
    qint64 freeBlocks = 0;
    qint64 totalBlocks =0;

    for(const QString& line : output.split('\n')) {
        if(line.startsWith("blksize:")) {
            blockSize = line.split(':').at(1).trimmed().toInt();
        } else if(line.startsWith("blocks:")) {
            totalBlocks = line.split(':').at(1).trimmed().toLongLong();
        } else if(line.startsWith("free blocks:")) {
            freeBlocks = line.split(':').at(1).trimmed().toLongLong();
        }
    }
    if(freeBlocks > 0 && totalBlocks > 0 && blockSize > 0) {
        total = totalBlocks * blockSize;
        freeface = freeBlocks * blockSize;
        return true;
    } else {
        return false;
    }
    return true;
}


bool readReiserFSUsage(const QString& partitionpath, qint64& freeface, qint64& total)
{
    QString output = "";
    QString dir = "";
    QString error = "";
    int exitcode = 0;
    if(!KServer::KCommand::getInstance()->RunScripCommand("debugreiserfs",
                     {"-d", partitionpath}, dir, output, error, exitcode)) {
        return false;
    }
    int blockSize = 0;
    qint64 freeBlocks = 0;
    qint64 totalBlocks =0;

    for(const QString& line : output.split('\n')) {
        if(line.startsWith("Count of blocks on the device:")) {
            totalBlocks = line.split(':').last().trimmed().toLongLong();
        } else if(line.startsWith("Blocksize:")) {
            blockSize = line.split(':').last().trimmed().toInt();
        } else if(line.startsWith("Free blocks (count of blocks)")) {
            freeBlocks = line.split(':').last().trimmed().toLongLong();
        }
    }
    if(freeBlocks > 0 && totalBlocks > 0 && blockSize > 0) {
        total = totalBlocks * blockSize;
        freeface = freeBlocks * blockSize;
        return true;
    } else {
        return false;
    }
    return true;
}

bool readXFSUsage(const QString& partitionpath, qint64& freeface, qint64& total)
{
    QString output = "";
    QString dir = "";
    QString error = "";
    int exitcode = 0;
    if(!KServer::KCommand::getInstance()->RunScripCommand("xfs_db",
                     {"-c sb", "-c print", "-r", partitionpath}, dir, output, error,exitcode)) {
        return false;
    }

    if(output.isEmpty()) {
        return false;
    }

    int blockSize = 0;
    qint64 freeBlocks = 0;
    qint64 totalBlocks =0;

    for(const QString& line : output.split('\n')) {
        if(line.startsWith("fdblocks")) {
            freeBlocks = line.split('=').last().trimmed().toLongLong();
        } else if(line.startsWith("dblocks")) {
            totalBlocks = line.split('=').last().trimmed().toLongLong();
        } else if(line.startsWith("blocksize")) {
            blockSize = line.split('=').last().trimmed().toInt();
        }
    }
    if(freeBlocks > 0 && totalBlocks > 0 && blockSize > 0) {
        total = totalBlocks * blockSize;
        freeface = freeBlocks * blockSize;
        return true;
    } else {
        return false;
    }
    return true;
}

bool readNtFSUsage(const QString& partitionpath, qint64& freeface, qint64& total)
{
    QString output = "";
    QString dir = "";
    QString error = "";
    int exitcode = 0;
    if(!KServer::KCommand::getInstance()->RunScripCommand(
                    "ntfsinfo", {"-mf", partitionpath}, dir, output, error, exitcode))
    {
        return false;
    }

    int clusterSize = 0;
    qint64 totalCluster = 0;
    qint64 freeCluster = 0;

    for(const QString& line : output.split('\n')) {
        if(line.contains("Cluster Size")) {
            const QString m =  regexpLabel("Cluster Size:\\s+(\\d+)", line);
            if(!m.isEmpty()) {
                clusterSize = m.toInt();
            }
        } else if(line.contains("Volume Size in Clusters:")) {
            const QString m =  regexpLabel("Volume Size in Clusters:\\s+(\\d+)", line);
            if(!m.isEmpty()) {
                totalCluster = m.toLongLong();
            }
        } else if(line.contains("Free Clusters:")) {
            const QString m =  regexpLabel("Free Clusters:\\s+(\\d+)", line);
            if(!m.isEmpty()) {
                freeCluster = m.toLongLong();
            }
        }
    }

    if(freeCluster > 0 && totalCluster > 0 && clusterSize > 0) {
        freeface = freeCluster * clusterSize;
        total = totalCluster * clusterSize;
        return true;
    } else {
        return false;
    }
    return true;
}

bool readUsage(const QString& partitionpath, FSType fs, qint64& freeface, qint64& total)
{
    bool isOK = false ;
    switch (fs) {
    case FSType::BtrFS:
        isOK = readBtrFSUsage(partitionpath, freeface, total);
        break;
    case FSType::Ext2:
    case FSType::Ext3:
    case FSType::Ext4:
    case FSType::LingmoData:
        isOK = readExt2Usage(partitionpath, freeface, total);
        break;
    case FSType::EFI:
    case FSType::Fat16:
    case FSType::Fat32:
        isOK = readFat16Usage(partitionpath, freeface, total);
        break;
    case FSType::Jfs:
        isOK = readJfsUsage(partitionpath, freeface, total);
        break;
    case FSType::LinuxSwap:
        isOK = readLinuxSwapUsage(partitionpath, freeface, total);
        break;
    case FSType::Reiser4:
        isOK = readReiser4Usage(partitionpath, freeface, total);
        break;
    case FSType::ReiserFS:
        isOK = readReiserFSUsage(partitionpath, freeface, total);
        break;
    case FSType::XFS:
        isOK = readXFSUsage(partitionpath, freeface, total);
        break;
    case FSType::NtFS:
        isOK = readNtFSUsage(partitionpath, freeface, total);
        break;
    default:
        break;
    }
    if(!isOK) {
        freeface = -1;
        total = -1;
        qWarning() << "failed to read usage:" << partitionpath;
    }
    return isOK;
}
}
}
