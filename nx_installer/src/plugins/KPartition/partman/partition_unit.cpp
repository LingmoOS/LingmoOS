#include "partition_unit.h"
#include <QMap>
#include <QString>
#include <QDebug>
#include <math.h>
#include <QDir>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include "../PluginService/kcommand.h"
#include "../PluginService/sysInfo/mount.h"
#include "../PluginService/sysInfo/meminfo.h"
#include "../PluginService/kcommand.h"
#include "../PluginService/ksystemsetting_unit.h"
#include "filesystem.h"
using namespace KServer;

bool isMips = false;//boot partition is the first partition

#include "proberos.h"

namespace KInstaller {

namespace Partman {

static QMap<QString, QString> device_OSDesc;

const int PARTITION_LABEL_MAXLEN = 25;

void appendToDevice(OSProberItemStr &ositem)
{
    device_OSDesc[ositem.path] = ositem.OSdescription;
}

void removeByOSPath(const QString& path)
{
    device_OSDesc.remove(path);
}

QString getOSDesc(const QString& path)
{
    return device_OSDesc[path];
}

void refreshOSProberItems()
{
    device_OSDesc.clear();

    const OSProberItemList& ositems = getOsProberItems();

    for(auto item : ositems) {
        appendToDevice(item);
    }

}

QString trimText(const QString &text, int max_len)
{
    if(text.length() > max_len) {
        return text.left(max_len) + "..";
    } else {
        return text;
    }
}

void removeOSProberDataByPath(const QString &path)
{
    removeByOSPath(path);
}

QString getPartitionName(const QString &path)
{
    return QFileInfo(path).fileName();
}

QString getPartitionLabel(const Partition::Ptr partition)
{
    QString tmp = "";
    switch (partition->m_type) {
    case PartitionType::Unallocated:
        return QObject::tr("Freespace");
    case PartitionType::Normal:
    case PartitionType::Logical: {
        if(partition->m_status == PartitionStatus::New) {
            return getPartitionName(partition->m_partitionPath);
        }
        const QString OSDesc = getOSDesc(partition->m_partitionPath);
        if(!OSDesc.isEmpty()) {
            return trimText(OSDesc, PARTITION_LABEL_MAXLEN);
        }
        if(!partition->m_FSlabel.isEmpty()) {
            return trimText(OSDesc, PARTITION_LABEL_MAXLEN);
        }
        return getPartitionName(partition->m_partitionPath);
    }
    default:
        return tmp;

    }
}

void alignPartition(Partition::Ptr partition)
{
    const qint64 oneMebiByteSerctor = 1 * kMebiByte / partition->m_sectorSize;

    const int startsize = static_cast<int>(ceil(partition->m_sectorStart * 1.0 / oneMebiByteSerctor));
    const int endsize = static_cast<int>(floor((partition->m_sectorEnd + 1) * 1.0 / oneMebiByteSerctor));
    partition->m_sectorStart = startsize * oneMebiByteSerctor;
    partition->m_sectorEnd = endsize * oneMebiByteSerctor -1;
}

int allocLogicalPartitionNumber(Device::Ptr dev)
{
    int num = dev->m_maxPrims;
    for(const Partition::Ptr part : dev->partitions) {
        if(part->m_num >= num) {
            num = part->m_num;
        }
    }
    return num + 1;
}

int allocPrimaryPartitionNumber(Device::Ptr dev)
{
    QList<int> partitionNum = {};
    for(Partition::Ptr part : dev->partitions) {
        partitionNum << part->m_num;
    }

    for(int num = 1; num <= dev->m_maxPrims; num++) {
        if(partitionNum.contains(num)) {
            continue;
        }
        return num;
    }
    return -1;
}

const QStringList getIgnoredDeviceList()
{
    QStringList list = {};
    const QString cmd{"/bin/bash"};
    const QString arg{"cat /proc/sys/dev/cdrom/info 2 > /dev/null |grep \"drive name\" | xargs | tr \"\"\"\n\"|grep -v -E \"(drive)|(name)\"|xargs"};
    QString output, error, dir;
    int exitcode = 0;
    if(KServer::KCommand::getInstance()->RunScripCommand(cmd, {"-c", arg}, dir, output, error, exitcode)){
        list = output.replace("\n", "").split(" ", QString::SkipEmptyParts);
    } else {
        qWarning() << QString("getIgnoreDeviceList: Failed:%1").arg(error);
    }

    for(int i = 0; i < list.length(); i++) {
        list[i] = QString("/dev/%1").arg(list[i]);
    }
    qInfo() << QString("getIgnoreDeviceList:deteced:%1").arg(list.join(","));

    QString name;
    for(int j = 0; j < 2; j++) {
        name = QString("/dev/sr%1").arg(j);
        if(!list.contains(name)) {
            list.append(name);
        }
        name = QString("/dev/cdrom%1").arg(j);
        if(!list.contains(name)) {
            list.append(name);
        }
    }
    qInfo() << QString("getIgnoreDeviceList:%1").arg(list.join(","));
    return list;

}

QString getInstallerDevicePath()
{
    const MountItemList list(parseMountItems());
    // /cdrom
    // /lib/live/mount/medium
    QString casperPath("/cdrom");
    QFileInfo casperInfo("/cdrom");
    if(casperInfo.exists()) {
        casperPath = casperInfo.canonicalFilePath();
    }
//    QString livePath("/lib/live/mount/medium");
//    QFileInfo liveInfo("/lib/live/mount/medium");
//    if(liveInfo.exists()) {
//        livePath = liveInfo.canonicalFilePath();
//    }

    for(const MountItemStr& item : list) {
        if(item.mount == casperPath /*|| item.mount == livePath*/) {
            return item.path;
        }
    }
    return QString();

}

DeviceList filterInstallerDevice(const DeviceList &devs)
{
    DeviceList devices = {};
    const QStringList ignoreList = getIgnoredDeviceList();
    /*!GetSettingsBool(PartitionHideInstallationDevice)=true;*/
//    bool blsetting = true;
//    if(!blsetting) {
//        for(auto device : devs) {
//            if(ignoreList.contains(device->m_path)) {
//                qInfo() << QString("IgnoreDevice::Device:%1 is ignored!").arg(device->m_path);
//                continue;
//            }
//            Device::Ptr ptr(new Device(*device));
//            PartitionList list;
//            for(auto partition : device->partitions) {
//                list << QSharedPointer<Partition>(new Partition(*partition));
//            }
//            ptr->partitions = list;
//            devices << ptr;
//        }
//        return devices;
//    }
    const QString installDevicePath(getInstallerDevicePath());
    qInfo()<<"InstallPath="+ installDevicePath;
    for(Device::Ptr device : devs) {

        if(ignoreList.contains(device->m_path)) {
            qInfo() << QString("IgnoreDevice::Device:%1 is ignored!").arg(device->m_path);
            continue;
        }

        if(!installDevicePath.startsWith(device->m_path)) {
             qInfo()<<"DeviceinstallPath="+ device->m_path;
            Device::Ptr ptr(new Device(*device));
            PartitionList list = {};
            for(auto partition : device->partitions) {
                list << QSharedPointer<Partition>(new Partition(*partition));
            }
            ptr->partitions = list;
            devices << ptr;
        } else {
            qDebug() << "Filterd device:" << device<<':'+device->m_path;

        }
    }
    return (DeviceList)devices;
}

PartitionList filterFragmentationPartition(PartitionList partitions)
{
    PartitionList list = {};
    for(Partition::Ptr partition : partitions) {
        if(partition->m_type == PartitionType::Normal ||
                partition->m_type == PartitionType::Logical ||
                partition->m_type == PartitionType::Extended) {
            list.append(partition);
        } else if(partition->m_type == PartitionType::Unallocated) {
            if(partition->getByteLength() >= 2 *kMebiByte) {
                list.append(partition);
            }
        }/* else if(partition->m_type == PartitionType::Freespace) {
            list.append(partition);
        }*/
    }
    return (PartitionList)list;
    qDebug() << Q_FUNC_INFO;
}

FSType getDefaultFSType()
{
    //默认文件系统ext4
    const QString defaultFSName = "ext4";
    return findFSTypeByName(defaultFSName);
}
//	硬件描述（制造商，型号）
QString getDeviceModelAndCap(Device *dev)
{
    const int gibiSize = toGigByte(dev->getByteLength());
    return QString("%1 (%2G)").arg(dev->m_model).arg(gibiSize);

}

QString getDeviceModelCapAndPath(Device *dev)
{
    const int gibiSize = toGigByte(dev->getByteLength());
    const QString name(QFileInfo(dev->m_path).fileName());
    return QString("%1 %2G(%3)").arg(dev->m_model).arg(gibiSize).arg(name);
}

QString getLocalFSTypeName(FSType fs)
{
    switch (fs) {
    case FSType::BtrFS: return "btrfs";
    case FSType::EFI: return "efi";
    case FSType::Ext2: return "ext2";
    case FSType::Ext3: return "ext3";
    case FSType::Ext4: return "ext4";
    case FSType::LingmoData: QObject::tr("lingmo data partition");
    case FSType::Fat16: return "fat16";
    case FSType::Fat32: return "fat32";
    case FSType::Jfs: return "jfs";
    case FSType::NtFS: return "ntfs";
    case FSType::Reiser4: return "reiser4";
    case FSType::ReiserFS: return "reiserfs";
    case FSType::LinuxSwap: return QObject::tr("Swap partition");
    case FSType::XFS: return "xfs";
    default: return QObject::tr("Unknown");
    }
}

QString getPartitionLabelAndPath(const Partition *partition)
{
    QString label, label1;
    QString osDesc;
    QString name;
    switch (partition->m_type) {
    case PartitionType::Unallocated:
        return QObject::tr("Freespace");
    case PartitionType::Normal:
    case PartitionType::Logical:
        if(partition->m_status == PartitionStatus::New) {
            return getPartitionName(partition->m_partitionPath);
        }
            osDesc = getOSDesc(partition->m_partitionPath);
            name = getPartitionName(partition->m_partitionPath);
        if(!osDesc.isEmpty()) {
            label = trimText(osDesc, PARTITION_LABEL_MAXLEN);
            return QString("%1(%2)").arg(label).arg(name);
        }
        if(!partition->m_FSlabel.isEmpty()) {
            label1 = trimText(partition->m_FSlabel, PARTITION_LABEL_MAXLEN);
            return QString("%1(%2)").arg(label1).arg(name);
        }
        if(!partition->m_name.isEmpty()) {
            label = trimText(partition->m_name, PARTITION_LABEL_MAXLEN);
            return QString("%1(%2)").arg(label).arg(name);
        }
        return name;
    default:
        return QString();
    }
}

QString getPartitionUsage(Partition *partition)
{
    qint64 total, used;
    if(partition->m_type != PartitionType::Unallocated &&
            (partition->m_total > 0) &&
            (partition->m_total >= partition->m_freespace)) {
        total = partition->m_total;
        used = total - partition->m_freespace;
    } else {
        total = partition->getByteLength();
        used = 0;
    }
    if(total < 1 * kGibiByte) {
        return QString("%1/%2M").arg(toMebiByte(used)).arg(toMebiByte(total));
    } else {
        return QString("%1/%2G").arg(toGigByte(used)).arg(toGigByte(total));
    }
}

int getPartitionUsageValue(Partition *partition)
{
    qint64 total, used;
    if(partition->m_type != PartitionType::Unallocated &&
            (partition->m_total > 0) &&
            (partition->m_total >= partition->m_freespace)) {
        total = partition->m_total;
        used = total - partition->m_freespace;
    } else {
        total = partition->getByteLength();
        used = 0;
    }
    if(total > 0 && used >= 0 && total >= used) {
        return int(100.0 * used / total);
    } else {
        return 0;
    }
}

int checkSpecialModel()
{
//    QString pcdevice = KServer::ReadSettingIni("specialmodel", "computer");
//    QStringList list = pcdevice.split(";");
    QString error = "";
    QString output = "";
    QString path = "";
    int excid = -1;
    QStringList args;
    args.append("");
    bool isok = KServer::KCommand::getInstance()->RunScripCommand("archdetect", args, path, output, error, excid);
    output = output.simplified();
    if(isok) {
        /*
        *是否是EFI启动架构判断：
        *archdetect 命令查看结果中
        *	包含有EFI字眼就判定为EFI启动方式
        *	包含loongarch64也默认为EFI启动方式
        *其他情况则默认为legacy启动方式
        *	当archdetect中包含mips时，则必须要分boot分区必须是第一个分区。
        * 已通过降低文件系统版本解决boot分区为ext3问题
        */
        if(output.contains("alpha", Qt::CaseInsensitive) || output.contains("sw64", Qt::CaseInsensitive)) {
            return 0;
        }
        if(output.contains("efi", Qt::CaseInsensitive) || output.contains("loongarch64", Qt::CaseInsensitive)) {
            return 1;
        } else if(output.contains("mips", Qt::CaseInsensitive)){
            isMips = true;
            return 0;
        } else {
            return 0;
        }
    }

   return -1;
}

bool getMips()
{
    return isMips;
}

bool isEFIEnabled()
{
    //适配龙芯的boot分区必须ext3才能启动的问题
    int check = checkSpecialModel();
    if(check == 0) {
        return false;
    }
    //龙芯有PMON固件的bug
    if(QFile::exists("/proc/boardinfo")) {
        QFile file("/proc/boardinfo");
        if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            if(file.readAll().simplified().contains("PMON")) {
                return false;
            }
        }
    }
    //用于强制Legacy或UEFI引导的,自动检测硬件引导方式

    if(check == 1) {
        return true;
    }
    QDir dir("/sys/firmware/efi");
    return dir.exists();
}

bool checkLoaderAndPartitionTable(Device::Ptr dev)
{
    int res = checkSpecialModel();
    switch (dev->m_partTableType) {
    case PartTableType::GPT:
       if(res == 1) {
            return true;
       }
    case PartTableType::MsDos:
       if(res == 0) {
            return true;
       }
    default:
        return false;

    }


}
bool isMBRPreferred(const DeviceList &devices)
{
    return !isEFIEnabled();
}

bool isMountPointSupported(FSType fs)
{
    return (fs != FSType::EFI &&
            fs != FSType::LinuxSwap &&
            fs != FSType::Empty&&
            fs != FSType::LingmoData);
}

bool isPartitionTableMatch(PartTableType type)
{
    if(!isEFIEnabled()) {
        return true;
    }

    if(type == PartTableType::Empty) {
        return true;
    }

    if(type == PartTableType::MsDos){
        return true;
    }

    return type == PartTableType::GPT;
}


bool isPartitionTableMatch(const DeviceList &devices, const QString &devpath)
{
    const int devIndex = deviceIndex(devices, devpath);
    if(devIndex == -1) {
        qCritical() << "Failed to find device:" << devpath;
        return false;
    }
    PartTableType tabletype = devices.at(devIndex)->m_partTableType;
    return isPartitionTableMatch(tabletype);
}

Partition::Ptr createEmptyPartition(QString devpath, qint64 sectorsize, qint64 sectorstart, qint64 sectorend)
{
    Partition::Ptr partition(new Partition);
    partition->m_devPath = devpath;
    partition->m_sectorStart = sectorstart;
    partition->m_sectorEnd = sectorend;
    partition->m_sectorSize = sectorsize;
    partition->m_type = PartitionType::Unallocated;
    partition->fs = FSType::Empty;
    partition->m_status = PartitionStatus::Real;
    return partition;
}

bool isSwapAreaNeeded()
{
    const MemInfo memInfo = getMemInfo();
    //交换分区内存阈值
    const qint64 memThreshold = 20 * kGibiByte;
    return memInfo.memTotal <= memThreshold;
}

qint64 parsePartitionSize(const QString &size, qint64 devlength)
{
    QRegularExpression pattern("(\\d+)(mbi|gib|%)", QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = pattern.match(size);

    if(match.hasMatch()) {
        bool isOK;
        const int num = match.captured(1).toInt(&isOK, 10);
        if(!isOK || num < 0) {
            qCritical() << "Invalid size" << num;
            return -1;
        }
        const QString unit = match.captured(2).toLower();
        if(unit == "mib") {
            return num * kMebiByte;
        } else if(unit == "gib") {
            return num * kGibiByte;
        } else if(unit == "%") {
            return num / 100.0 * devlength;
        } else {
            qCritical() << Q_FUNC_INFO << "Invalid unit found:" << num;
            return -1;
        }
    }
    return -1;
}

QString getVentoyDevPath()
{
    QString error = "";
    QString devpath = "";
    QString output = "";
    QString path = "";
    int excid = -1;
    bool isok = KServer::KCommand::getInstance()->RunScripCommand("dmsetup", {"deps", "-o", "devname"}, path, output, error, excid);
    if(isok) {
        if(!output.isEmpty()) {
            QStringList list = output.split("\n");
            for(int i = 0; i < list.count(); i++) {
                QString str = list.at(i);
                if(str.startsWith("ventoy")) {
                    QStringList tmp = str.split(":");
                    QString name = tmp.at(2);
                    // 针对/dev开头的启动盘
                    devpath = "/dev/" + name.mid(2, name.length() - 4);
                    return devpath;
                } else {
                    continue;
                }
            }
        }
    }
    return devpath;

}
}

}
