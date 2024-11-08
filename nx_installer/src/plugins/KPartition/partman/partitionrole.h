#ifndef PARTITIONROLE_H
#define PARTITIONROLE_H

#include <QObject>
namespace KInstaller {

enum class PartitionRole {
        None = 0,
        Primary = 1,
        Extended = 2,
        Logical = 4,
        Unallocated = 8,
        Lusk = 16,
        Lvm_Lv = 32,
        Any = 255
};


}
#endif // PARTITIONROLE_H
