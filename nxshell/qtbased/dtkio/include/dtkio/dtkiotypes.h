// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTKIOTYPES_H
#define DTKIOTYPES_H

#include <stdint.h>

#include <string>

#include <QFlags>
#include <QVariant>

#include "dtkio_global.h"

DIO_BEGIN_NAMESPACE

enum class OpenFlag : quint16 {
    NotOpen = 0x0000,
    ReadOnly = 0x0001,
    WriteOnly = 0x0002,
    ReadWrite = ReadOnly | WriteOnly,
    Append = 0x0004,
    Truncate = 0x0008,
    Text = 0x0010,
    Unbuffered = 0x0020,
    NewOnly = 0x0040,
    ExistingOnly = 0x0080,

    CustomStart = 0x0100
};
Q_DECLARE_FLAGS(OpenFlags, OpenFlag)

enum class CopyFlag : quint8 {
    None = 0,   //!<@~english No flags set.
    Overwrite = 1,   //!<@~english Overwrite any existing files.
    Backup = 2,   //!<@~english Make a backup of any existing files.
    NoFollowSymlinks = 3,   //!<@~english Don’t follow symlinks.
    AllMetadata = 4,   //!<@~english Copy all file metadata instead of just default set used for copy.
    NoFallbackForMove = 5,   //!<@~english Don’t use copy and delete fallback if native move not supported.
    TargetDefaultPerms = 6,   //!<@~english Leaves target file with default perms, instead of setting the source file perms.

    UserFlag = 0x10
};

enum class AttributeID : quint16 {
    StandardType = 0,   //!<@~english uint32
    StandardIsHidden = 1,   //!<@~english boolean
    StandardIsBackup = 2,   //!<@~english boolean
    StandardIsSymlink = 3,   //!<@~english boolean
    StandardIsVirtual = 4,   //!<@~english boolean
    StandardIsVolatile = 5,   //!<@~english boolean
    StandardName = 6,   //!<@~english byte string
    StandardDisplayName = 7,   //!<@~english string
    StandardEditName = 8,   // string
    StandardCopyName = 9,   //!<@~english string
    StandardIcon = 10,   //!<@~english QList<QString>
    StandardSymbolicIcon = 11,   //!<@~english QList<QString>
    StandardContentType = 12,   //!<@~english string
    StandardFastContentType = 13,   //!<@~english string
    StandardSize = 14,   //!<@~english uint64
    StandardAllocatedSize = 15,   //!<@~english uint64
    StandardSymlinkTarget = 16,   //!<@~english byte string
    StandardTargetUri = 17,   //!<@~english string
    StandardSortOrder = 18,   //!<@~english int32
    StandardDescription = 19,   //!<@~english string

    EtagValue = 40,   //!<@~english string

    IdFile = 60,   //!<@~english string
    IdFilesystem = 61,   //!<@~english string

    AccessCanRead = 100,   //!<@~english boolean
    AccessCanWrite = 101,   //!<@~english boolean
    AccessCanExecute = 102,   //!<@~english boolean
    AccessCanDelete = 103,   //!<@~english boolean
    AccessCanTrash = 104,   //!<@~english boolean
    AccessCanRename = 105,   //!<@~english boolean

    MountableCanMount = 130,   //!<@~english boolean
    MountableCanUnmount = 131,   //!<@~english boolean
    MountableCanEject = 132,   //!<@~english boolean
    MountableUnixDevice = 133,   //!<@~english uint32
    MountableUnixDeviceFile = 134,   //!<@~english string
    MountableHalUdi = 135,   //!<@~english string
    MountableCanPoll = 136,   //!<@~english boolean
    MountableIsMediaCheckAutomatic = 137,   //!<@~english boolean
    MountableCanStart = 138,   //!<@~english boolean
    MountableCanStartDegraded = 139,   //!<@~english boolean
    MountableCanStop = 140,   //!<@~english boolean
    MountableStartStopType = 141,   //!<@~english uint32

    TimeModified = 200,   //!<@~english uint64
    TimeModifiedUsec = 201,   //!<@~english uint32
    TimeAccess = 202,   //!<@~english uint64
    TimeAccessUsec = 203,   //!<@~english uint32
    TimeChanged = 204,   //!<@~english uint64
    TimeChangedUsec = 205,   //!<@~english uint32
    TimeCreated = 206,   //!<@~english uint64
    TimeCreatedUsec = 207,   //!<@~english uint32

    UnixDevice = 330,   //!<@~english uint32
    UnixInode = 331,   //!<@~english uint64
    UnixMode = 332,   //!<@~english uint32
    UnixNlink = 333,   //!<@~english uint32
    UnixUID = 334,   //!<@~english uint32
    UnixGID = 335,   //!<@~english uint32
    UnixRdev = 336,   //!<@~english uint32
    UnixBlockSize = 337,   //!<@~english uint32
    UnixBlocks = 338,   //!<@~english uint64
    UnixIsMountPoint = 339,   //!<@~english boolean

    DosIsArchive = 360,   //!<@~english boolean
    DosIsSystem = 361,   //!<@~english boolean

    OwnerUser = 300,   //!<@~english string
    OwnerUserReal = 301,   //!<@~english string
    OwnerGroup = 302,   //!<@~english string

    ThumbnailPath = 390,   //!<@~english byte string
    ThumbnailFailed = 391,   //!<@~english boolean
    ThumbnailIsValid = 392,   //!<@~english boolean

    PreviewIcon = 420,   //!<@~english object

    FileSystemSize = 440,   //!<@~english uint64
    FileSystemFree = 441,   //!<@~english uint64
    FileSystemUsed = 442,   //!<@~english uint64
    FileSystemType = 443,   //!<@~english string
    FileSystemReadOnly = 444,   //!<@~english boolean
    FileSystemUsePreview = 445,   //!<@~english uint32
    FileSystemRemote = 446,   //!<@~english boolean

    GvfsBackend = 470,   //!<@~english string

    SelinuxContext = 490,   //!<@~english string

    TrashItemCount = 510,   //!<@~english uint32
    TrashDeletionDate = 511,   //!<@~english string
    TrashOrigPath = 512,   //!<@~english byte string

    RecentModified = 540,   //!<@~english int64

    CustomStart = 600,

    StandardIsFile = 610,
    StandardIsDir = 611,
    StandardIsRoot = 612,
    StandardSuffix = 613,
    StandardCompleteSuffix = 614,
    StandardFilePath = 615,
    StandardParentPath = 616,
    StandardBaseName = 617,
    StandardFileName = 618,
    StandardCompleteBaseName = 619,

    AttributeIDMax = 999,
};

enum class SeekType : quint8 {
    Begin = 0x00,
    Current = 0x01,
    End = 0x02
};

enum class Permission : quint16 {
    NoPermission = 0x0000,

    ExeOther = 0x0001,
    WriteOther = 0x0002,
    ReadOther = 0x0004,

    ExeGroup = 0x0010,
    WriteGroup = 0x0020,
    ReadGroup = 0x0040,

    ExeUser = 0x0100,
    WriteUser = 0x0200,
    ReadUser = 0x0400,

    ExeOwner = 0x1000,
    WriteOwner = 0x2000,
    ReadOwner = 0x4000,
};
Q_DECLARE_FLAGS(Permissions, Permission)

enum class AttributeType : quint8 {
    TypeInvalid = 0,   //!<@~english Indicates an invalid or uninitialized type
    TypeString = 1,   //!<@~english A null terminated UTF8 string
    TypeByteString = 2,   //!<@~english A zero terminated string of non-zero bytes
    TypeBool = 3,   //!<@~english A boolean value
    TypeUInt32 = 4,   //!<@~english An unsigned 4-byte/32-bit integer
    TypeInt32 = 5,   //!<@~english A signed 4-byte/32-bit integer
    TypeUInt64 = 6,   //!<@~english An unsigned 8-byte/64-bit integer
    TypeInt64 = 7,   //!<@~english A signed 8-byte/64-bit integer
    TypeObject = 8,   //!<@~english A Object
    TypeStringV = 9   //!<@~english A NULL terminated char **
};

enum class FileQueryInfoFlags : quint8 {
    TypeNone,
    TypeNoFollowSymlinks
};

enum class DirFilter : qint16 {
    NoFilter = 0x0000,   //!<@~english no filter
    Dirs = 0x0001,   //!<@~english List directories that match the filters.
    Files = 0x0002,   //!<@~english List files.
    Drives = 0x0004,   //!<@~english List disk drives (ignored under Unix).
    AllEntries = Dirs | Files | Drives,   //!<@~english List directories, files, drives and symlinks (this does not list broken symlinks unless you specify System).
    NoSymLinks = 0x0008,   //!<@~english Do not list symbolic links (ignored by operating systems that don't support symbolic links).

    Readable = 0x0010,   //!<@~english List files for which the application has read access. The Readable value needs to be combined with Dirs or Files.
    Writable = 0x0020,   //!<@~english List files for which the application has write access. The Writable value needs to be combined with Dirs or Files.
    Executable = 0x0040,   //!<@~english//!<@~english List files for which the application has execute access. The Executable value needs to be combined with Dirs or Files.
    Modified = 0x0080,   //!<@~english Only list files that have been modified (ignored on Unix).

    Hidden = 0x0100,   //!<@~english List hidden files (on Unix, files starting with a ".").
    System = 0x0200,   //!<@~english List system files (on Unix, FIFOs, sockets and device files are included; on Windows, .lnk files are included)
    AllDirs = 0x0400,   //!<@~english List all directories; i.e. don't apply the filters to directory names.
    CaseSensitive = 0x0800,   //!<@~english The filter should be case sensitive.

    NoDot = 0x2000,   //!<@~english Do not list the special entry ".".
    NoDotDot = 0x4000,   //!<@~english Do not list the special entry "..".
    NoDotAndDotDot = NoDot | NoDotDot,   //!<@~english Do not list the special entries "." and "..".
};
Q_DECLARE_FLAGS(DirFilters, DirFilter)

enum class IteratorFlag : quint8 {
    NoIteratorFlags = 0x00,   //!<@~english The default value, representing no flags. The iterator will return entries for the assigned path.
    FollowSymlinks = 0x01,   //!<@~english When combined with Subdirectories, this flag enables iterating through all subdirectories of the assigned path, following all symbolic links. Symbolic link loops (e.g., "link" => "." or "link" => "..") are automatically detected and ignored.
    Subdirectories = 0x02,   //!<@~english List entries inside all subdirectories as well.
};
Q_DECLARE_FLAGS(IteratorFlags, IteratorFlag)

enum class WatchType : quint8 {
    Auto = 0x00,
    Dir = 0x01,
    File = 0x02,
};

struct AttributeDetails
{
    AttributeID id;
    std::string key;
    QVariant defaultValue;
    AttributeType type;
};

DIO_END_NAMESPACE

#endif   // DTKIOTYPES_H
