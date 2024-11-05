#ifndef PARTITION_H
#define PARTITION_H

#include <QObject>
#include <parted/parted.h>
#include <QSharedPointer>
#include "filesystem.h"
#include "datastruct.h"
#include "proberos.h"
namespace KInstaller {

enum class PartitionFlag {
    Boot=1,
    Root=2,
    Swap=3,
    Hidden=4,
    RAID=5,
    LVM=6,
    LBA=7,
    HPServer,
    PALO,
    PREP,
    MSFTReserved,
    BIOSGrub,
    AppleTVRecovery,
    Diag,
    LegacyBoot,
    MSFTData,
    IRST,
    ESP=18,
};
typedef QList<PartitionFlag> PartitionFlags;
namespace Partman {

enum class PartitionStatus {
    Real,
    New,
    Format,
    Delete,
};

class Partition
{

public:
    explicit Partition();
    Partition(const Partition& partition);
    ~Partition();

    qint64 getSectorLenth();
    int getNum();
    qint64 getSector();
    qint64 getByteLength();
    QString getUsedSpace();
    QString getTotalSpace();


    void changeNumber(int partitinNum);

signals:

public slots:


public:
    typedef QSharedPointer<Partition> Ptr;
    bool operator==(const Partition& other) const;

    int m_num;
    FSType fs;
    QString m_fsname;
    qint64 m_sectorStart;
    qint64 m_sectorEnd;
    qint64 m_sectorSize;
    qint64 m_total;
    qint64 m_freespace;
    QString m_devPath;
    QString m_partitionPath;
    QString m_UUID;
    QString m_FSlabel;
    QString m_name;//part-label
    QString m_OSname;
    PartitionType m_type;
    PartitionStatus m_status;
    Partition::Ptr m_partFather;

    PartTableType tableType;
    bool blFormatPartition;
    PartitionFlags flags;
    QString m_mountPoint;
    bool m_isMounted;
    OSType osType;
};
typedef QList<Partition::Ptr> PartitionList;

int partitionIndex(PartitionList& partitions, Partition::Ptr partition);
int extendedPartitionIndex(const PartitionList& partitions);
PartitionList getPrimaryPartitions(const PartitionList& partitions);
PartitionList getLogicalPartitions(const PartitionList& partitions);

bool isPartitionsJoint(const Partition::Ptr part1, const Partition::Ptr part2);


}
}
#endif // PARTITION_H
