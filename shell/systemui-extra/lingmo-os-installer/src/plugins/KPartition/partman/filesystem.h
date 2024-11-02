#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <QObject>
#include <QStringList>

namespace KInstaller {
namespace Partman {
enum class FSType {
    Empty,
    Ext2,
    Ext3,
    Ext4,
    Jfs,
    Fat12,
    Fat16,
    Fat32,
    NtFS,
    VFat,
    EFI,
    LinuxSwap,
    ReiserFS,
    Reiser4,
    XFS,
    BtrFS,
    ExFat,
    Lvm2_PV,
    Recovery,
    LingmoData,
    Extended

};
FSType findFSTypeByName(QString name);
QString findNameByFSType(FSType type);
QStringList getFSListNameString();

typedef QList<FSType> FsTypeList;

}
}
#endif // FILESYSTEM_H
