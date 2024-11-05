#ifndef OPERATIONDisk_H
#define OPERATIONDisk_H

#include <QObject>
#include "device.h"
#include "partition.h"
namespace KInstaller{
namespace Partman {

enum class  OperatorTYPE
{
    Create,
    Delete,
    Format,
    MountPoint,
    NewPartitionTable,
    Resize,
    Invalid
    
};

class OperationDisk
{
public:
    explicit OperationDisk(Device::Ptr dev);
    OperationDisk(OperatorTYPE type,
                 Partition::Ptr origPartition,
                 Partition::Ptr newPartition);
    ~OperationDisk();

    void applyToShow(Device::Ptr dev);
    bool applyToDisk();
    void umount(Partition::Ptr part);
    void umount(Device::Ptr dev);

    QString OperationDiskDescription();
private:
    void applyCreateShow(PartitionList& partitions);
    void applyDeleteShow(PartitionList& partitions);
    void applyNewTableShow(Device::Ptr dev);
    void applyResizeShow(PartitionList& partitions);
    void substitutePartition(PartitionList& partitions);

    

signals:

public slots:
    
public:
    Device::Ptr m_dev;
    Partition::Ptr m_origPartition;
    Partition::Ptr m_newPartition;
    OperatorTYPE m_type;
    
};
typedef QList<OperationDisk> OperationDiskList;

void mergeAllOperation(OperationDiskList& operations, OperationDisk& operate);
void mergeAllUnallocatesPartitions(PartitionList& partitions);
}
}
#endif // OPERATIONDisk_H
