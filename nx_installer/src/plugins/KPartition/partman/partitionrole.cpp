#include "partitionrole.h"
#include <QMap>
namespace KInstaller {
static const QMap <PartitionRole, QString> roleMap {
    {PartitionRole::Extended, QString("extended")},
    {PartitionRole::Logical, QString("logical")},
    {PartitionRole::Lusk, QString("LUSK")},
    {PartitionRole::Lvm_Lv, QString("LVM logical volume")},
    {PartitionRole::Primary, QString("primary")},
    {PartitionRole::Unallocated, QString("unallocated")},
    {PartitionRole::None, QString("none")}
};


}
