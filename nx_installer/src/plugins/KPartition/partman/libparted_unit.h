#ifndef LIBPARTED_UNIT_H
#define LIBPARTED_UNIT_H

#include <QObject>
#include <parted/parted.h>
#include "datastruct.h"
#include "partition.h"
namespace KInstaller {

namespace Partman {

    bool commit(PedDisk* pdisk);
    bool syncUdev(int timeout);
    void destroyDevice(PedDevice* pdev);
    void destroyDisk(PedDisk* pdisk);
    bool getDevice(const QString devpath, PedDevice* &pdev);
    bool getDisk(PedDevice* &pdev, PedDisk* &pdisk);
    PedPartitionType getPedPartitionType(Partition::Ptr partition);
    QString getPedFsTypeName(Partition::Ptr partition);
    QString getPartitionPath(PedPartition* pPartition);
//    QString getDevicePath(PedPartition* pPartition);
    PedFileSystemType* getPedFSType(Partition::Ptr partition);
    bool flushDevice(PedDevice* pdev);


    bool createPartitionTable(const QString &devpath, PartTableType tabletype);
    bool createPartition(Partition::Ptr partition);
    bool DeletePartition(Partition::Ptr partition);

    bool resetPartitionSize(Partition::Ptr partition);
    bool resetPartitionFSType(Partition::Ptr partition);
    bool setPartitionFlag(Partition::Ptr partition, PedPartitionFlag flag, bool blset);
    bool setPartitionFlags(Partition::Ptr partition);

    bool resizePartition(Partition::Ptr partition);

    bool checkCanCreate(PedDevice* pdev);

    bool upatePartitionNumber(Partition::Ptr partition);



}
}
#endif // LIBPARTED_UNIT_H
