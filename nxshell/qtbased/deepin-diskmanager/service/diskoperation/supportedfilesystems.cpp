/* modified by Uniontech Software Technology Co., Ltd. */
/* Copyright (C) 2016 Mike Fleetwood
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "supportedfilesystems.h"
#include "filesystems/ext2.h"
#include "filesystems/linuxswap.h"
#include "filesystems/fat16.h"
#include "filesystems/ntfs.h"
#include "filesystems/btrfs.h"
#include "filesystems/exfat.h"
#include "filesystems/xfs.h"
#include "utils.h"

#include <QDebug>

namespace DiskManager {

SupportedFileSystems::SupportedFileSystems()
{
    m_fsObjects[FS_UNKNOWN] = NULL;
    m_fsObjects[FS_OTHER] = NULL;
    m_fsObjects[FS_BTRFS] = new Btrfs;
    m_fsObjects[FS_EXFAT] = new ExFat();
    m_fsObjects[FS_EXT2] = new EXT2(FS_EXT2);
    m_fsObjects[FS_EXT3] = new EXT2(FS_EXT3);
    m_fsObjects[FS_EXT4] = new EXT2(FS_EXT4);
    //    m_fs_objects[FS_F2FS]            = new f2fs();
    //    m_fs_objects[FS_FAT16]           = new fat16(FS_FAT16);
    m_fsObjects[FS_FAT32]           = new FAT16(FS_FAT32);
    //    m_fs_objects[FS_HFS]             = new hfs();
    //    m_fs_objects[FS_HFSPLUS]         = new hfsplus();
    //    m_fs_objects[FS_JFS]             = new jfs();
    m_fsObjects[FS_LINUX_SWAP] = new LinuxSwap;
    //    m_fs_objects[FS_LVM2_PV]         = new lvm2_pv();
    //    m_fs_objects[FS_LUKS]            = new luks();
    //    m_fs_objects[FS_MINIX]           = new minix();
    //    m_fs_objects[FS_NILFS2]          = new nilfs2();
    m_fsObjects[FS_NTFS]            = new NTFS();
    //    m_fs_objects[FS_REISER4]         = new reiser4();
    //    m_fs_objects[FS_REISERFS]        = new reiserfs();
    //    m_fs_objects[FS_UDF]             = new udf();
    m_fsObjects[FS_XFS]             = new XFS();
    m_fsObjects[FS_APFS] = NULL;
    m_fsObjects[FS_ATARAID] = NULL;
    m_fsObjects[FS_BITLOCKER] = NULL;
    m_fsObjects[FS_GRUB2_CORE_IMG] = NULL;
    m_fsObjects[FS_ISO9660] = NULL;
    m_fsObjects[FS_LINUX_SWRAID] = NULL;
    m_fsObjects[FS_LINUX_SWSUSPEND] = NULL;
    m_fsObjects[FS_REFS] = NULL;
    m_fsObjects[FS_UFS] = NULL;
    m_fsObjects[FS_ZFS] = NULL;
}

SupportedFileSystems::~SupportedFileSystems()
{
    FSObjectsMap::iterator iter;
    for (iter = m_fsObjects.begin(); iter != m_fsObjects.end(); iter++) {
        auto pvalue = iter.value();

        if (pvalue != NULL) {
            delete pvalue;
        }

        iter.value() = NULL;
    }
}

void SupportedFileSystems::findSupportedFilesystems()
{
    FSObjectsMap::iterator iter;
    m_fsSupport.clear();

    for (iter = m_fsObjects.begin(); iter != m_fsObjects.end(); iter++) {
        if (iter.value()) {
            FileSystem *psys = iter.value();
            m_effectivefs.append(Utils::fileSystemTypeToString(iter.key()));
            m_fsSupport.push_back(psys->getFilesystemSupport());
        } else {
            FS fsBasicsupp(iter.key());
            fsBasicsupp.move = FS::GPARTED;
            fsBasicsupp.copy = FS::GPARTED;
            m_fsSupport.push_back(fsBasicsupp);
        }
    }
}

FileSystem *SupportedFileSystems::getFsObject(FSType fstype) const
{
//    qDebug() << Utils::fileSystemTypeToString(fstype);
    FSObjectsMap::const_iterator iter = m_fsObjects.find(fstype);
    if (iter == m_fsObjects.end()) {
        return nullptr;
    } else {
        return iter.value();
    }
}

const FS &SupportedFileSystems::getFsSupport(FSType fstype) const
{
    for (int i = 0; i < m_fsSupport.size(); i++) {
        if (m_fsSupport[i].fstype == fstype) {
            return m_fsSupport[i];
        }
    }
    static FS fsNotsupp(FS_UNSUPPORTED);
    return fsNotsupp;
}

//const QVector<FS> &SupportedFileSystems::getAllFsSupport() const
//{
//    return m_fsSupport;
//}

//bool SupportedFileSystems::supportedFilesystem(FSType fstype) const
//{
//    return getFsObject(fstype) != NULL;
//}

const QStringList &SupportedFileSystems::getAllFsName()
{
    return m_effectivefs;
}

} // namespace DiskManager
