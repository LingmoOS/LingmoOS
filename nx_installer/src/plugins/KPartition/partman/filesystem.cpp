#include "filesystem.h"
#include <QMap>
#include "../PluginService/sysInfo/machine.h"
#include "../delegate/customize_device_model.h"
#include "partition_unit.h"
#include <QDebug>
namespace KInstaller {
namespace Partman {
static const QMap<FSType, QString> FSTypeMap {
    {FSType::Empty, ""},
    {FSType::BtrFS, "btrfs"},
    {FSType::EFI, "efi"},
    {FSType::Ext2, "ext2"},
    {FSType::Ext3, "ext3"},
    {FSType::Ext4, "ext4"},
    {FSType::Fat12, "fat12"},
    {FSType::Fat16, "fat16"},
    {FSType::Fat32, "fat32"},
    {FSType::NtFS, "ntfs"},
    {FSType::ReiserFS, "reiserfs"},
    {FSType::Reiser4, "reiser4"},
    {FSType::XFS, "xfs"},
    {FSType::Jfs, "jfs"},
    {FSType::LinuxSwap, "swap"},
    {FSType::LingmoData, "lingmodata"}, //2021-4-7 add lingmodata 添加用户数据分区
    {FSType::Extended, "extend"}
};

FSType findFSTypeByName(QString name)
{
    QString lowerName = name.toLower();
    if(lowerName.contains("linux-swap")) {
        return FSType::LinuxSwap;
    } else if(lowerName.contains("lvm2_pv")) {
        return FSType::Lvm2_PV;
    } else if(lowerName.contains("lingmo-data")) {
        return FSType::LingmoData;
    } else {
        return FSTypeMap.key(lowerName);

    }

}

QString findNameByFSType(FSType type)
{
    switch (type) {
    case FSType::LinuxSwap:
        return "linux-swap";
        break;
    case FSType::Lvm2_PV:
        return "lvm2_pv";
        break;
    case FSType::LingmoData:
        return "lingmo-data";
        break;
    case FSType::EFI:{
        QString tmp = getDeviceStrByType(getDeviceModel());
        tmp.remove(QRegExp("\\s"));//去除字符串中间空格
        if(tmp.contains("Kirin990") || tmp.contains("Kirin9006C")) {
            return "fat16";
        } else {
            return FSTypeMap.value(type);
            break;
        }
    }
    case FSType::Empty:
        return "";
        break;
    default:
        return FSTypeMap.value(type);
        break;
    }
}

QStringList getFSListNameString()
{
    QStringList fslist;
    for(int i = 0; i < FSTypeMap.values().length(); i++) {
        fslist.append(FSTypeMap.values().at(i));
    }
    return  fslist;
}
}
}
