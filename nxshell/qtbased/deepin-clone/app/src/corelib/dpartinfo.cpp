// Copyright (C) 2017 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#include "ddiskinfo.h"
#include "helper.h"
#include "dpartinfo_p.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

DPartInfoPrivate::DPartInfoPrivate(DPartInfo *qq)
    : q(qq)
{

}

void DPartInfoPrivate::refresh()
{

}

qint64 DPartInfoPrivate::getUsedSize() const
{
    return usedSize;
}

qint64 DPartInfoPrivate::getFreeSize() const
{
    return freeSize;
}

bool DPartInfoPrivate::getIsDeepinSystemRoot() const
{
    return isDeepinSystemRoot;
}

DPartInfo::FSType DPartInfoPrivate::toType(const QString &name)
{
    if (name.isEmpty())
        return DPartInfo::Invalid;

    if (name == "ext2" ) {
        return DPartInfo::EXT2;
    } else if (name == "ext3") {
        return DPartInfo::EXT3;
    } else if (name == "ext4") {
        return DPartInfo::EXT4;
    } else if (name == "fat12") {
        return DPartInfo::FAT12;
    } else if (name == "fat16") {
        return DPartInfo::FAT16;
    } else if (name == "fat32") {
        return DPartInfo::FAT32;
    } else if (name == "btrfs") {
        return DPartInfo::Btrfs;
    } else if (name == "f2fs") {
        return DPartInfo::F2FS;
    } else if (name == "hfs+") {
        return DPartInfo::HFS_Plus;
    } else if (name == "minix") {
        return DPartInfo::Minix;
    } else if (name == "nilfs2") {
        return DPartInfo::Nilfs2;
    } else if (name == "ntfs") {
        return DPartInfo::NTFS;
    } else if (name == "reiser4") {
        return DPartInfo::Reiser4;
    } else if (name == "vfat") {
        return DPartInfo::VFAT;
    } else if (name == "iso9660") {
        return DPartInfo::ISO9660;
    } else if (name == "jfs") {
        return DPartInfo::JFS;
    } else if (name == "xfs") {
        return DPartInfo::XFS;
    } else if (name == "swap") {
        return DPartInfo::Swap;
    } else if (name == "LVM2_member") {
        return DPartInfo::LVM2_member;
    }

    return DPartInfo::UnknowFS;
}

DPartInfo::DPartInfo()
{

}

DPartInfo::DPartInfo(const DPartInfo &other)
    : d(other.d)
{
    if (d)
        d->q = this;
}

DPartInfo::~DPartInfo()
{

}

void DPartInfo::swap(DPartInfo &other)
{
    qSwap(d, other.d);

    if (d)
        d->q = this;
}

QString DPartInfo::filePath() const
{
    return d->filePath;
}

QString DPartInfo::parentDiskFilePath() const
{
    return d->parentDiskFilePath;
}

DPartInfo &DPartInfo::operator=(const DPartInfo &other)
{
    d = other.d;
    return *this;
}

QString DPartInfo::name() const
{
    return d->name;
}

QString DPartInfo::kname() const
{
    return d->kname;
}

int DPartInfo::indexNumber() const
{
    return d->index;
}

QString DPartInfo::typeName() const
{
    return d->partTypeName;
}

DPartInfo::Type DPartInfo::type() const
{
    return d->partType;
}

bool DPartInfo::isExtended() const
{
    return d->partType == Extended || d->partType == Win95_Extended_LBA || d->partType == Linux_extended;
}

int DPartInfo::blockSize() const
{
    return d->blockSize;
}

qint64 DPartInfo::totalSize() const
{
    return d->size;
}

qint64 DPartInfo::sizeStart() const
{
    return d->sizeStart;
}

qint64 DPartInfo::sizeEnd() const
{
    return d->sizeEnd;
}

qint64 DPartInfo::usedSize() const
{
    return d->getUsedSize();
}

qint64 DPartInfo::freeSize() const
{
    return d->getFreeSize();
}

QString DPartInfo::fileSystemTypeName() const
{
    return d->fsTypeName;
}

DPartInfo::FSType DPartInfo::fileSystemType() const
{
    return d->fsType;
}

QString DPartInfo::mountPoint() const
{
    return d->mountPoint;
}

bool DPartInfo::isMounted() const
{
    return !mountPoint().isEmpty();
}

QString DPartInfo::uuid() const
{
    return d->uuid;
}

QString DPartInfo::label() const
{
    return d->label;
}

QString DPartInfo::partLabel() const
{
    return d->partLabel;
}

DPartInfo::GUIDType DPartInfo::guidType() const
{
    return d->guidType;
}

QString DPartInfo::transport() const
{
    return d->transport;
}

QString DPartInfo::partUUID() const
{
    return d->partUUID;
}

bool DPartInfo::isReadonly() const
{
    return d->readonly;
}

bool DPartInfo::isRemoveable() const
{
    return d->removeable;
}

bool DPartInfo::isDeepinSystemRoot() const
{
    return d->getIsDeepinSystemRoot();
}

void DPartInfo::refresh()
{
    d->refresh();
}

QByteArray DPartInfo::toJson() const
{
    QJsonObject root
    {
        {"filePath", filePath()},
        {"parentDiskFilePath", parentDiskFilePath()},
        {"name", name()},
        {"kname", kname()},
        {"typeName", typeName()},
        {"type", type()},
        {"typeDescription", typeDescription(type())},
        {"blockSize", blockSize()},
        {"totalSize", QString::number(totalSize())},
        {"sizeStart", QString::number(sizeStart())},
        {"sizeEnd", QString::number(sizeEnd())},
        {"usedSize", QString::number(usedSize())},
        {"freeSize", QString::number(freeSize())},
        {"fsTypeName", fileSystemTypeName()},
        {"fsType", fileSystemType()},
        {"mountPoint", mountPoint()},
        {"label", label()},
        {"partLabel", partLabel()},
        {"guidType", guidType()},
        {"guidTypeDescription", guidTypeDescription(guidType())},
        {"readonly", isReadonly()},
        {"removeable", isRemoveable()},
        {"transport", transport()},
        {"index", indexNumber()},
        {"isDeepinSystemRoot", isDeepinSystemRoot()},
        {"partUUID", partUUID()},
        {"uuid", uuid()}
    };

    QJsonDocument doc(root);

    return doc.toJson();
}

DPartInfo::Type DPartInfo::type(const QString &type)
{
    bool ok = false;
    int t = type.toInt(&ok, 16);

    if (!ok)
        return Unknow;

    return Type(t);
}

QString DPartInfo::typeDescription(DPartInfo::Type type)
{
    switch (type) {
    case Empty:
        return "Empty";
    case FAT12Type:
        return "FAT12";
    case XENIX_root:
        return "XENIX root";
    case XENIX_usr:
        return "XENIX usr";
    case FAT16_Less_32M:
        return "FAT16 <32M";
    case Extended:
        return "Extended";
    case FAT16Type:
        return "FAT16";
    case HPFS_NTFS:
        return "HPFS/NTFS";
    case AIX:
        return "AIX";
    case AIX_bootable:
        return "AIX bootable";
    case OS2_Boot_Manager:
        return "OS2 Boot Manager";
    case Win95_FAT32:
        return "Win95 FAT32";
    case Win95_FAT32_LBA:
        return "Win95 FAT32 (LBA)";
    case Win95_FAT16_LBA:
        return "Win95 FAT16 (LBA)";
    case Win95_Extended_LBA:
        return "Win95 Ext'd (LBA)";
    case OPUS:
        return "OPUS";
    case Hidden_FAT12:
        return "Hidden FAT12";
    case Compaq_diagnostics:
        return "Compaq diagnostics";
    case Hidden_FAT16_Less_32M:
        return "Hidden FAT16 <32M";
    case Hidden_FAT16:
        return "Hidden FAT16";
    case Hidden_HPFS_or_NTFS:
        return "Hidden HPFS/NTFS";
    case AST_SmartSleep:
        return "AST SmartSleep";
    case Hidden_Win95_FAT32:
        return "Hidden Win95 FAT32";
    case Hidden_Win95_FAT32_LBA:
        return "Hidden Win95 FAT32 (LBA)";
    case Hidden_Win95_FAT16_LBA:
        return "Hidden Win95 FAT16";
    case NEC_DOS:
        return "NEC DOS";
    case Plan9:
        return "Plan 9";
    case PartitionMagic_recovery:
        return "PartitionMagic recovery";
    case Venix_80286:
        return "Venix 80286";
    case PPC_PReP_Boot:
        return "PPC PReP Boot";
    case SFS:
        return "SFS";
    case QNX4_dot_x:
        return "QNX4.x";
    case QNX4_dot_x_2nd_part:
        return "QNX4.x.2nd part";
    case QNX4_dot_x_3rd_part:
        return "QNX4.x 3rd part";
    case OnTrack_DM:
        return "OnTrack DM";
    case OnTrack_DM6_Aux1:
        return "OnTrack DM6 Aux1";
    case CP_M:
        return "CP/M";
    case OnTrack_DM6_Aux3:
        return "OnTrack DM6 Aux3";
    case OnTrackDM6:
        return "OnTrackDM6";
    case EZ_Drive:
        return "EZ-Drive";
    case Golden_Bow:
        return "Golden Bow";
    case Priam_Edisk:
        return "Priam Edisk";
    case SpeedStor:
        return "SpeedStor";
    case GNU_HURD_or_SysV:
        return "GNU HURD or SysV";
    case Novell_Netware_286:
        return "Novell Netware 286";
    case Novell_Netware_386:
        return "Novell Netware 386";
    case DiskSecure_Multi_Boot:
        return "DiskSecure Multi-Boot";
    case PC_IX:
        return "PC/IX";
    case Old_Minix:
        return "Old Minix";
    case Minix_old_Linux:
        return "Minix / old Linux";
    case Linux_swap:
        return "Linux swap";
    case Linux:
        return "Linux";
    case OS2_hidden_C_drive:
        return "OS/2 hidden C: drive";
    case Linux_extended:
        return "Linux extended";
    case NTFS_volume_set_1:
        return "NTFS volume set";
    case NTFS_volume_set_2:
        return "NTFS volume set";
    case Linux_LVM:
        return "Linux LVM";
    case Amoeba:
        return "Amoeba";
    case Amoeba_BBT:
        return "Amoeba BBT";
    case BSD_OS:
        return "BSD/OS";
    case IBM_Thinkpad_hibernation:
        return "IBM Thinkpad hibernation";
    case FreeBSD:
        return "FreeBSD";
    case OpenBSD:
        return "OpenBSD";
    case NeXTSTEP:
        return "NeXTSTEP";
    case NetBSD:
        return "NetBSD";
    case BSDI_fs:
        return "BSDI fs";
    case BSDI_swap:
        return "BSDI swap";
    case Boot_Wizard_hidden:
        return "Boot Wizard hidden";
    case DRDOS_sec_FAT12:
        return "DRDOS/sec (FAT-12)";
    case DRDOS_sec_FAT16_Less_32M:
        return "DRDOS/sec (FAT-16 < 32M)";
    case DRDOS_sec_FAT16:
        return "DRDOS/sec (FAT-16)";
    case Syrinx:
        return "Syrinx";
    case Non_FS_data:
        return "Non-FS data";
    case CP_M_CTOS_dot_dot_dot:
        return "CP/M / CTOS / ...";
    case Dell_Utility:
        return "Dell Utility";
    case BootIt:
        return "BootIt";
    case DOS_access:
        return "DOS access";
    case DOS_R_O:
        return "DOS R/O";
    case SpeedStor_1:
        return "SpeedStor";
    case BeOS_fs:
        return "BeOS fs";
    case EFI_GPT:
        return "EFI GPT";
    case EFI_FAT12_16_32:
        return "EFI (FAT-12/16/32)";
    case Linux_PA_RISC_boot:
        return "Linux/PA-RISC boot";
    case SpeedStor_2:
        return "SpeedStor";
    case SeppdStor_3:
        return "SpeedStor";
    case DOS_secondary:
        return "DOS secondary";
    case Linux_raid_autodetect:
        return "Linux raid autodetect";
    case LANstep:
        return "LANstep";
    case BBT:
        return "BBT";
    default:
        break;
    }

    return QString();
}

DPartInfo::GUIDType DPartInfo::guidType(const QByteArray &guid)
{
    static QByteArrayList list;

    if (list.isEmpty()) {
        // None
        list << "00000000-0000-0000-0000-000000000000"
             << "024DEE41-33E7-11D3-9D69-0008C781F39F"
             << "C12A7328-F81F-11D2-BA4B-00A0C93EC93B"
             << "21686148-6449-6E6F-744E-656564454649"
             << "D3BFE2DE-3DAF-11DF-BA40-E3A556D89593"
             << "F4019732-066E-4E12-8273-346C5641494F"
             << "BFBFAFE7-A34F-448A-9A5B-6213EB736C22"
        // Windows
             << "E3C9E316-0B5C-4DB8-817D-F92DF00215AE"
             << "EBD0A0A2-B9E5-4433-87C0-68B6B72699C7"
             << "5808C8AA-7E8F-42E0-85D2-E1E90434CFB3"
             << "AF9B60A0-1431-4F62-BC68-3311714A69AD"
             << "DE94BBA4-06D1-4D40-A16A-BFD50179D6AC"
             << "37AFFC90-EF7D-4e96-91C3-2D7AE055B174"
             << "E75CAF8F-F680-4CEE-AFA3-B001E56EFC2D"
        // HP-UX
             << "75894C1E-3AEB-11D3-B7C1-7B03A0000000"
             << "E2A1E728-32E3-11D6-A682-7B03A0000000"
        // Linux
             << "0FC63DAF-8483-4772-8E79-3D69D8477DE4"
             << "A19D880F-05FC-4D3B-A006-743F0F84911E"
             << "44479540-F297-41B2-9AF7-D131D5F0458A"
             << "4F68BCE3-E8CD-4DB1-96E7-FBCAF984B709"
             << "69DAD710-2CE4-4E3C-B16C-21A1D49ABED3"
             << "B921B045-1DF0-41C3-AF44-4C6F280D3FAE"
             << "0657FD6D-A4AB-43C4-84E5-0933C84B4F4F"
             << "E6D6D379-F507-44C2-A23C-238F2A3DF928"
             << "933AC7E1-2EB4-4F13-B844-0E14E2AEF915"
             << "3B8F8425-20E0-4F3B-907F-1A25A76F98E8"
             << "7FFEC5C9-2D00-49B7-8941-3EA10A5586B7"
             << "CA7D7CCB-63ED-4C53-861C-1742536059CC"
             << "8DA63339-0007-60C0-C436-083AC8230908"
        // FreeBSD
             << "83BD6B9D-7F41-11DC-BE0B-001560B84F0F"
             << "516E7CB4-6ECF-11D6-8FF8-00022D09712B"
             << "516E7CB5-6ECF-11D6-8FF8-00022D09712B"
             << "516E7CB6-6ECF-11D6-8FF8-00022D09712B"
             << "516E7CB8-6ECF-11D6-8FF8-00022D09712B"
             << "516E7CBA-6ECF-11D6-8FF8-00022D09712B"
        // macOS Darwin
             << "48465300-0000-11AA-AA11-00306543ECAC"
             << "55465300-0000-11AA-AA11-00306543ECAC"
             << "6A898CC3-1DD2-11B2-99A6-080020736631"
             << "52414944-0000-11AA-AA11-00306543ECAC"
             << "52414944-5F4F-11AA-AA11-00306543ECAC"
             << "426F6F74-0000-11AA-AA11-00306543ECAC"
             << "4C616265-6C00-11AA-AA11-00306543ECAC"
             << "5265636F-7665-11AA-AA11-00306543ECAC"
             << "53746F72-6167-11AA-AA11-00306543ECAC"
             << "B6FA30DA-92D2-4A9A-96F1-871EC6486200"
             << "2E313465-19B9-463F-8126-8A7993773801"
             << "FA709C7E-65B1-4593-BFD5-E71D61DE9B02"
             << "BBBA6DF5-F46F-4A89-8F59-8765B2727503"
        // Solaris illumos
             << "6A82CB45-1DD2-11B2-99A6-080020736631"
             << "6A85CF4D-1DD2-11B2-99A6-080020736631"
             << "6A87C46F-1DD2-11B2-99A6-080020736631"
             << "6A8B642B-1DD2-11B2-99A6-080020736631"
//             << "6A898CC3-1DD2-11B2-99A6-080020736631"
             << "6A8EF2E9-1DD2-11B2-99A6-080020736631"
             << "6A90BA39-1DD2-11B2-99A6-080020736631"
             << "6A9283A5-1DD2-11B2-99A6-080020736631"
             // reserved partition
             << "6A945A3B-1DD2-11B2-99A6-080020736631"
             << "6A9630D1-1DD2-11B2-99A6-080020736631"
             << "6A980767-1DD2-11B2-99A6-080020736631"
             << "6A96237F-1DD2-11B2-99A6-080020736631"
             << "6A8D2AC7-1DD2-11B2-99A6-080020736631"
        // NetBSD
             << "49F48D32-B10E-11DC-B99B-0019D1879648"
             << "49F48D5A-B10E-11DC-B99B-0019D1879648"
             << "49F48D82-B10E-11DC-B99B-0019D1879648"
             << "49F48DAA-B10E-11DC-B99B-0019D1879648"
             << "2DB519C4-B10F-11DC-B99B-0019D1879648"
             << "2DB519EC-B10F-11DC-B99B-0019D1879648"
        // ChromeOS
             << "FE3A2A5D-4F32-41A7-B725-ACCC3285A309"
             << "3CB8E202-3B7E-47DD-8A3C-7FF2A13CFCEC"
             << "2E0A753D-9E48-43B0-8337-B15192CB1B5E"
        // HaiKu
             << "42465331-3BA3-10F1-802A-4861696B7521"
        // MidnightBSD
             << "85D5E45E-237C-11E1-B4B3-E89A8F7FC3A7"
             << "85D5E45A-237C-11E1-B4B3-E89A8F7FC3A7"
             << "85D5E45B-237C-11E1-B4B3-E89A8F7FC3A7"
             << "0394EF8B-237E-11E1-B4B3-E89A8F7FC3A7"
             << "85D5E45C-237C-11E1-B4B3-E89A8F7FC3A7"
             << "85D5E45D-237C-11E1-B4B3-E89A8F7FC3A7"
        // Ceph
             << "45B0969E-9B03-4F30-B4C6-B4B80CEFF106"
             << "45B0969E-9B03-4F30-B4C6-5EC00CEFF106"
             << "4FBD7E29-9D25-41B8-AFD0-062C0CEFF05D"
             << "4FBD7E29-9D25-41B8-AFD0-5EC00CEFF05D"
             << "89C57F98-2FE5-4DC0-89C1-F3AD0CEFF2BE"
             << "89C57F98-2FE5-4DC0-89C1-5EC00CEFF2BE"
        // OpenBSD
             << "824CC7A0-36A8-11E3-890A-952519AD3F61"
        // QNX
             << "CEF5A9AD-73BC-4601-89F3-CDEEEEE321A1"
        // Plan 9
             << "C91818F9-8025-47AF-89D2-F030D7000C2C"
        // VMware ESX
             << "9D275380-40AD-11DB-BF97-000C2911D1B8"
             << "AA31E02A-400F-11DB-9590-000C2911D1B8"
             << "9198EFFC-31C0-11DB-8F78-000C2911D1B8"
        // Android-IA
             << "2568845D-2332-4675-BC39-8FA5A4748D15"
             << "114EAFFE-1552-4022-B26E-9B053604CF84"
             << "49A4D17F-93A3-45C1-A0DE-F50B2EBE2599"
             << "4177C722-9E92-4AAB-8644-43502BFD5506"
             << "EF32A33B-A409-486C-9141-9FFB711F6266"
             << "20AC26BE-20B7-11E3-84C5-6CFDB94711E9"
             << "38F428E6-D326-425D-9140-6E0EA133647C"
             << "A893EF21-E428-470A-9E55-0668FD91A2D9"
             << "DC76DDA9-5AC1-491C-AF42-A82591580C0D"
             << "EBC597D0-2053-4B15-8B64-E0AAC75F4DB1"
             << "8F68CC74-C5E5-48DA-BE91-A0C8C15E9C80"
             << "767941D0-2085-11E3-AD3B-6CFDB94711E9"
             << "AC6D7924-EB71-4DF8-B48D-E267B27148FF"
        // Open Network Install Environment (ONIE)
             << "7412F7D5-A156-4B13-81DC-867174929325"
             << "D4E6E2CD-4469-46F3-B5CB-1BFF57AFC149"
        // PowerPC
             << "9E1A2D38-C612-4316-AA26-8B49521E5A8B"
        // freedesktop.org OSes (Linux, etc.)
             << "BC13C2FF-59E6-4262-A352-B275FD6F7172"
        // Atari IOS
             << "734E5AFE-F61A-11E6-BC64-92361F002671";
    }

    int index = list.indexOf(guid);

    if (index < 0)
        return InvalidGUID;

    return (GUIDType)(index + GUIDTypeBegin);
}

QString DPartInfo::guidTypeDescription(DPartInfo::GUIDType type)
{
    switch ((int)type) {
    case Unused_None:
        return "Unused entry";
    case MBR_PS_None:
        return "MBR partition scheme";
    case EFI_SP_None:
        return "EFI System partition";
    case BIOS_BP_None:
        return "BIOS boot partition";
    case iFFS_None:
        return "Intel Fast Flash (iFFS) partition (for Intel Rapid Start technology)";
    case Sony_BP_None:
        return "Sony boot partition";
    case Lenove_BP_None:
        return "Lenovo boot partition";
    // Windows
    case MSR_Win:
        return "Microsoft Reserved Partition (MSR)";
    case BasicData_Win:
        return "Basic data partition of Windows";
    case LDM_Win:
        return "Logical Disk Manager (LDM) metadata partition of Windows";
    case LDM_DP_Win:
        return "Logical Disk Manager data partition of Windows";
    case WRE_Win:
        return "Windows Recovery Environment";
    case IBM_GPFS_Win:
        return "IBM General Parallel File System (GPFS) partition of Windows";
    case SSP_Win:
        return "Storage Spaces partition of Windows";
    // HP-UX
    case DP_HPUX:
        return "Data partition of HP-UX";
    case SP_HPUX:
        return "Service Partition of HP-UX";
    // Linux
    case LFD_Linux:
        return "Linux filesystem data";
    case RAID_P_Linux:
        return "RAID partition of Linux";
    case RP_x86_Linux:
        return "Root partition (x86) of Linux";
    case RP_x86_64_Linux:
        return "Root partition (x86-64) of Linux";
    case RP_32bit_ARM_Linux:
        return "Root partition (32-bit ARM) of Linux";
    case RP_64bit_ARM_Linux:
        return "Root partition (64-bit ARM/AArch64) of Linux";
    case SP_Linux:
        return "Swap partition of Linux";
    case LVM_P_Linux:
        return "Logical Volume Manager (LVM) partition of Linux";
    case Home_P_Linux:
        return "/home partition of Linux";
    case Srv_P_Linux:
        return "/srv (server data) partition of Linux";
    case Plain_DC_P_Linux:
        return "Plain dm-crypt partition of Linux";
    case LUKS_P_Linux:
        return "LUKS partition of Linux";
    case Reserved_Linux:
        return "Reserved of Linux";
    // FreeBSD
    case BP_FreeBSD:
        return "Boot partition of FreeBSD";
    case DP_FreeBSD:
        return "Data partition of FreeBSD";
    case SP_FreeBSD:
        return "Swap partition of FreeBSD";
    case UFS_P_FreeBSD:
        return "Unix File System (UFS) partition of FreeBSD";
    case VVM_P_FreeBSD:
        return "Vinum volume manager partition of FreeBSD";
    case ZFS_P_FreeBSD:
        return "ZFS partition of FreeBSD";
    // macOS Darwin
    case HFS_PLUS_P_Mac:
        return "Hierarchical File System Plus (HFS+) partition of macOS";
    case UFS_Mac:
        return "Apple UFS";
    case ZFS_Mac:
        return "ZFS of macOS(Or /usr partition of Solaris illumos)";
    case RAID_P_Mac:
        return "Apple RAID partition";
    case RAID_P_Offline_Mac:
        return "Apple RAID partition, offline";
    case BP_Mac:
        return "Apple Boot partition (Recovery HD)";
    case Label_Mac:
        return "Apple Label";
    case TV_RP_Mac:
        return "Apple TV Recovery partition";
    case CS_P_Mac:
        return "Apple Core Storage (i.e. Lion FileVault) partition";
    case SoftRAID_Status_Mac:
        return "SoftRAID_Status of macOS";
    case SoftRAID_Scratch_Mac:
        return "SoftRAID_Scratch of macOS";
    case SoftRAID_Volume_Mac:
        return "SoftRAID_Volume of macOS";
    case SoftRAID_Cache_Mac:
        return "SoftRAID_Cache of macOS";
    // Solaris illumos
    case BP_Solaris:
        return "Boot partition of Solaris illumos";
    case RP_Solaris:
        return "Root partition of Solaris illumos";
    case SP_Solaris:
        return "Swap partition of Solaris illumos";
    case Backup_P_Solaris:
        return "Backup partition of Solaris illumos";
    case Var_P_Solaris:
        return "/var partition of Solaris illumos";
    case Home_P_Solaris:
        return "/home partition of Solaris illumos";
    case AS_Solaris:
        return "Alternate sector os Solaris illumos";
    case Reserved_Solaris:
        return "Reserved partition os Solaris illumos";
    // NetBSD
    case SP_NetBSD:
        return "Swap partition of NetBSD";
    case FFS_P_NetBSD:
        return "FFS partition of NetBSD";
    case LFS_P_NetBSD:
        return "LFS partition of NetBSD";
    case RAID_P_NetBSD:
        return "RAID partition of NetBSD";
    case CP_NetBSD:
        return "Concatenated partition of NetBSD";
    case EP_NetBSD:
        return "Encrypted partition of NetBSD";
    // ChromeOS
    case Kernel_ChromeOS:
        return "ChromeOS kernel";
    case Rootfs_ChromeOS:
        return "ChromeOS rootfs";
    case FU_ChromeOS:
        return "ChromeOS future use";
    // Haiku
    case BFS_Haiku:
        return "Haiku BFS";
    // MidnightBSD
    case BP_MidnightBSD:
        return "Boot partition of MidnightBSD";
    case DP_MidnightBSD:
        return "Data partition of MidnightBSD";
    case SP_MidnightBSD:
        return "Swap partition of MidnightBSD";
    case UFS_P_MidnightBSD:
        return "Unix File System (UFS) partition of MidnightBSD";
    case VVM_P_MidnightBSD:
        return "Vinum volume manager partition of MidnightBSD";
    case ZFS_P_MidnightBSD:
        return "ZFS partition of MidnightBSD";
    // Ceph
    case Journal_Ceph:
        return "Ceph Journal";
    case DC_EJ_Ceph:
        return "Ceph dm-crypt Encrypted Journal";
    case OSD_Ceph:
        return "Ceph OSD";
    case DC_OSD_Ceph:
        return "Ceph dm-crypt OSD";
    case DIC_Ceph:
        return "Ceph disk in creation";
    case DC_DIC_Ceph:
        return "Ceph dm-crypt disk in creation";
    // OpenBSD
    case DP_OpenBSD:
        return "Data partition of OpenBSD";
    // QNX
    case PAFS_QNX:
        return "Power-safe (QNX6) file system of QNX";
    // Plan9
    case Partition_Plan9:
        return "Plan 9 partition of Plan9";
    // VMware ESX
    case Vmkcore_VMware:
        return "vmkcore (coredump partition)";
    case VMFS_VMware:
        return "VMFS filesystem partition";
    case Reserved_VMware:
        return "VMware Reserved";
    // Android-IA
    case Bootloader_Android:
        return "Android Bootloader";
    case Bottloader2_Android:
        return "Android Bootloader2";
    case Boot_Android:
        return "Android Boot";
    case Recovery_Android:
        return "Android Recovery";
    case Misc_Android:
        return "Android Misc";
    case Metadata_Android:
        return "Android Metadata";
    case System_Android:
        return "Android System";
    case Cache_Android:
        return "Android Cache";
    case Data_Android:
        return "Android Data";
    case Persistent_Android:
        return "Android Persistent";
    case Factory_Android:
        return "Android Factory";
    case Fastboot_Android:
        return "Android Fastboot";
    case OEM_Android:
        return "Android OEM";
    // Open Network Install Environment (ONIE)
    case Boot_ONIE:
        return "Open Network Install Environment Boot";
    case Config_ONIE:
        return "Open Network Install Environment Config";
    // PowerPC
    case Boot_PowerPC:
        return "PowerPC PReP boot";
    // freedesktop.org OSes (Linux, etc.)
    case SBLC_OSes:
        return "Shared boot loader configuration of freedesktop.org OSes (Linux, etc.)";
    // Atari TOS
    case BD_P_Atari:
        return "Basic data partition (GEM, BGM, F32) of Atari TOS";
    }

    return "Invalid GUID type";
}

DPartInfo::DPartInfo(DPartInfoPrivate *dd)
    : d(dd)
{
    if (dd)
        dd->q = this;
}

void DPartInfo::fromJson(const QJsonObject &root, DPartInfoPrivate *dd)
{
    dd->filePath = root.value("filePath").toString();
    dd->parentDiskFilePath = root.value("parentDiskFilePath").toString();
    dd->name = root.value("name").toString();
    dd->kname = root.value("kname").toString();
    dd->blockSize = root.value("blockSize").toInt();
    dd->sizeStart = root.value("sizeStart").toString().toLongLong();
    dd->sizeEnd = root.value("sizeEnd").toString().toLongLong();
    dd->size = root.value("totalSize").toString().toLongLong();
    dd->usedSize = root.value("usedSize").toString().toLongLong();
    dd->freeSize = root.value("freeSize").toString().toLongLong();
    dd->fsTypeName = root.value("fsTypeName").toString();
    dd->fsType = (FSType)root.value("fsType").toInt();
    dd->mountPoint = root.value("mountPoint").toString();
    dd->label = root.value("label").toString();
    dd->partLabel = root.value("partLabel").toString();
    dd->partTypeName = root.value("typeName").toString();
    dd->partType = (Type)root.value("type").toInt();
    dd->guidType = (GUIDType)root.value("guidType").toInt();
    dd->readonly = root.value("readonly").toBool();
    dd->removeable = root.value("removeable").toBool();
    dd->transport = root.value("transport").toString();
    dd->index = root.value("index").toInt(-1);
    dd->isDeepinSystemRoot = root.value("isDeepinSystemRoot").toBool();
    dd->partUUID = root.value("partUUID").toString();
    dd->uuid = root.value("uuid").toString();
}

void DPartInfo::fromJson(const QByteArray &json, DPartInfoPrivate *dd)
{
    fromJson(QJsonDocument::fromJson(json).object(), dd);
}

QT_BEGIN_NAMESPACE
#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug deg, const DPartInfo &info)
{
    QDebugStateSaver saver(deg);
    Q_UNUSED(saver)

    deg.space() << "name:" << info.name()
                << "index:" << info.indexNumber()
                << "type:" << info.fileSystemTypeName()
                << "size:" << Helper::sizeDisplay(info.totalSize())
                << "used size:" << Helper::sizeDisplay(info.usedSize())
                << "free size:" << Helper::sizeDisplay(info.freeSize())
                << "mount point:" << info.mountPoint()
                << "label:" << info.label()
                << "part label:" << info.partLabel()
                << "GUID type:" << DPartInfo::guidTypeDescription(info.guidType());

    return deg;
}
#endif
QT_END_NAMESPACE
