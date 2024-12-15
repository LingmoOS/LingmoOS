// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef COMMONDEF_H
#define COMMONDEF_H
#include <QObject>
#include <QVector>
#include <QDBusArgument>

//new by liuwh 2022/1/17
#define DBUSStructEnd(className) Q_DECLARE_METATYPE(className)\
    QDBusArgument &operator<<(QDBusArgument &argument, const className &data);\
    const QDBusArgument &operator>>(const QDBusArgument &argument, className &data);

#define GPTBACKUP 33
#define UEFI_SECTOR 2048 //第一个分区从2048开始 为将来UEFI升级预留空间

//todo 此处待优化  改为枚举
#define FAST_TYPE 1     // 快速
#define SECURE_TYPE 2   // 安全
#define DOD_TYPE 3      // 高级
#define GUTMANN_TYPE 4  // 古德曼算法
//todo 此处待优化  修改为dev_type
#define DISK_TYPE   0   // 磁盘
#define PART_TYPE   1   // 分区

const unsigned int LUKS1_MaxKey = 8;  //luks 1 最大密钥槽数
const unsigned int LUKS2_MaxKey = 32; //luks 2 最大密钥槽数

typedef long long Sector;
typedef long long Byte_Value;

//Size units defined in bytes
const Byte_Value KIBIBYTE = 1024;
const Byte_Value MEBIBYTE = (KIBIBYTE * KIBIBYTE);
const Byte_Value GIBIBYTE = (MEBIBYTE * KIBIBYTE);
const Byte_Value TEBIBYTE = (GIBIBYTE * KIBIBYTE);
const Byte_Value PEBIBYTE = (TEBIBYTE * KIBIBYTE);
const Byte_Value EXBIBYTE = (PEBIBYTE * KIBIBYTE);

enum FSType {
    // Special partition types and functions
    FS_UNSUPPORTED = 0, // Type with no supported actions
    FS_UNALLOCATED = 1, // Unallocated space on a partitioned drive
    FS_UNKNOWN = 2, // Unrecognised content in a drive or partition
    FS_UNFORMATTED = 3, // Create a partition without a file system
    FS_CLEARED = 4, // Clear existing file system signatures
    FS_OTHER = 5, // Just for showing in the File System Support dialog
    FS_EXTENDED = 6,

    // Fully supported file system types
    FS_BTRFS = 7,
    FS_EXFAT = 8, /* Also known as fat64 */
    FS_EXT2 = 9,
    FS_EXT3 = 10,
    FS_EXT4 = 11,
    FS_F2FS = 12,
    FS_FAT16 = 13,
    FS_FAT32 = 14,
    FS_HFS = 15,
    FS_HFSPLUS = 16,
    FS_JFS = 17,
    FS_LINUX_SWAP = 18,
    FS_LUKS = 19,
    FS_LVM2_PV = 20,
    FS_MINIX = 21,
    FS_NILFS2 = 22,
    FS_NTFS = 23,
    FS_REISER4 = 24,
    FS_REISERFS = 25,
    FS_UDF = 26,
    FS_XFS = 27,

    // Other recognised file system types
    FS_APFS = 28,
    FS_ATARAID = 29,
    FS_BITLOCKER = 30,
    FS_GRUB2_CORE_IMG = 31,
    FS_ISO9660 = 32,
    FS_LINUX_SWRAID = 33,
    FS_LINUX_SWSUSPEND = 34,
    FS_REFS = 35,
    FS_UFS = 36,
    FS_ZFS = 37,

    // Partition space usage colours
    FS_USED = 38,
    FS_UNUSED = 39
};



enum SIZE_UNIT {
    UNIT_SECTOR = 0,
    UNIT_BYTE = 1,

    UNIT_KIB = 2,
    UNIT_MIB = 3,
    UNIT_GIB = 4,
    UNIT_TIB = 5
};



enum PartitionType {
    TYPE_PRIMARY = 0, // Primary partition on a partitioned drive
    TYPE_LOGICAL = 1, // Logical partition on a partitioned drive
    TYPE_EXTENDED = 2, // Extended partition on a partitioned drive
    TYPE_UNALLOCATED = 3, // Unallocated space on a partitioned drive
    TYPE_UNPARTITIONED = 4 // Unpartitioned whole drive
};



enum PartitionStatus {
    STAT_REAL = 0,
    STAT_NEW = 1,
    STAT_COPY = 2,
    STAT_FORMATTED = 3
};



enum PartitionAlignment {
    ALIGN_CYLINDER = 0, //Align to nearest cylinder
    ALIGN_MEBIBYTE = 1, //Align to nearest mebibyte
    ALIGN_STRICT = 2 //Strict alignment - no rounding
                   //  Indicator if start and end sectors must remain unchanged
};



enum CUSTOM_TEXT {
    CTEXT_NONE,
    CTEXT_ACTIVATE_FILESYSTEM, // Activate text ('Mount', 'Swapon', VG 'Activate', ...)
    CTEXT_DEACTIVATE_FILESYSTEM, // Deactivate text ('Unmount', 'Swapoff', VG 'Deactivate', ...)
    CTEXT_CHANGE_UUID_WARNING, // Warning to print when changing UUIDs
    CTEXT_RESIZE_DISALLOWED_WARNING // File system resizing currently disallowed reason
};



//resize opertation
enum Action {
    NONE = 0,
    MOVE_RIGHT = 1,
    MOVE_LEFT = 2,
    GROW = 3,
    SHRINK = 4,
    MOVE_RIGHT_GROW = 5,
    MOVE_RIGHT_SHRINK = 6,
    MOVE_LEFT_GROW = 7,
    MOVE_LEFT_SHRINK = 8
};



// Minimum and maximum file system size limits
struct FS_Limits {
    Byte_Value min_size; // 0 => no limit, +ve => limit defined.  (As implemented by)
    Byte_Value max_size; // -----------------"-----------------   (code using these.)

    FS_Limits()
        : min_size(0)
        , max_size(0)
    {
    }
    FS_Limits(Byte_Value min, Byte_Value max)
        : min_size(min)
        , max_size(max)
    {
    }
};



enum ExecFlags {
    EXEC_NONE = 1 << 0,
    EXEC_CHECK_STATUS = 1 << 1, // Set the status of the command in the operation
    // details based on the exit status being zero or
    // non-zero.  Must either use this flag when calling
    // ::execute_command() or call ::set_status()
    // afterwards.
    EXEC_CANCEL_SAFE = 1 << 2,
    EXEC_PROGRESS_STDOUT = 1 << 3, // Run progress tracking callback after reading new
    // data on stdout from command.
    EXEC_PROGRESS_STDERR = 1 << 4, // Same but for stderr.
    EXEC_PROGRESS_TIMED = 1 << 5 // Run progress tracking callback periodically.
};



// Struct to store file system support information
struct FS {
    enum Support {
        NONE = 0, //not support
        GPARTED = 1, //using internal func
        LIBPARTED = 2, //using libparted func
        EXTERNAL = 3 //using external func
    };

    FSType fstype;
    Support busy; // How to determine if partition/file system is busy
    Support read; // Can and how to read sector usage while inactive
    Support read_label;
    Support write_label;
    Support read_uuid;
    Support write_uuid;
    Support create;
    Support create_with_label; // Can and how to format file system with label
    Support grow;
    Support shrink;
    Support move; // start point and endpoint
    Support check; // Some check tool available?
    Support copy;
    Support remove;
    Support online_read; // Can and how to read sector usage while active
    Support online_grow;
    Support online_shrink;

    FS(FSType fstype_ = FS_UNSUPPORTED)
        : fstype(fstype_)
    {
        busy = read = read_label = write_label = read_uuid = write_uuid = create = NONE;
        create_with_label = grow = shrink = move = check = copy = remove = online_read = NONE;
        online_grow = online_shrink = NONE;
    }
};



struct MountEntry {
    bool readonly; // Is the file system mounted read-only?
    QVector<QString> mountpoints; // File system mounted on ...
    MountEntry()
        : readonly(false) {}
};



//new by liuwh 2022/3/4
/**
 * @enum DISK_ERROR
 * @brief 磁盘 错误类型
 */
enum DISK_ERROR {
    DISK_ERR_DBUS_ARGUMENT = 0,         //DBus参数错误
    DISK_ERR_DISK_INFO = 1,             //磁盘信息错误
    DISK_ERR_PART_INFO = 2,             //分区信息错误
    DISK_ERR_DELETE_PART_FAILED = 3,    //删除分区失败
    DISK_ERR_UPDATE_KERNEL_FAILED = 4,  //提交内核失败
    DISK_ERR_CREATE_FS_FAILED = 5,      //创建文件系统失败
    DISK_ERR_MOUNT_FAILED = 6,          //挂载失败
    DISK_ERR_CREATE_MOUNTDIR_FAILED = 7,//创建挂载文件夹失败
    DISK_ERR_CHOWN_FAILED = 8,          //修改属主失败
    DISK_ERR_CREATE_PART_FAILED=9,      //分区创建失败
    DISK_ERR_CREATE_PARTTAB_FAILED=10,  //分区表创建失败


    DISK_ERR_NORMAL = 100               //无错误 正常
};



//new by liuwh 2022/1/17
/**
 * @enum DevType
 * @brief 设备类型
 */
enum DevType {
    DEV_UNKNOW_DEVICES,         //未知设备
    DEV_DISK,                   //磁盘
    DEV_UNALLOCATED_PARTITION,  //未分配的分区
    DEV_PARTITION,              //分区
    DEV_LOOP,                   //loop设备
    DEV_META_DEVICES            //元数据设备 raid 加密磁盘映射等虚拟设备
};
#endif // COMMONDEF_H
