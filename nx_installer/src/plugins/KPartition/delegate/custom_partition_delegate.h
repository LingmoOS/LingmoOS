#ifndef CUSTOM_PARTITION_DELEGATE_H
#define CUSTOM_PARTITION_DELEGATE_H

#include <QObject>
#include "partition_delegate.h"
#include "partman/device.h"
namespace KInstaller {

class PartitionDelegate;
class CustomPartitiondelegate : public PartitionDelegate
{
    Q_OBJECT
public:
    explicit CustomPartitiondelegate(QObject *parent = nullptr);
    ~CustomPartitiondelegate();
    QStringList getFsTypeList();

    FsTypeList getBootFsTypeList()const;

    QStringList getMountPoints();

    QList<Device::Ptr> getAllUsedDevice();

    QString comboxOperatorPartitions();
    void onCustomPartDone(const DeviceList& devices);
    bool unFormatPartition(Partition::Ptr part);

    bool isPartitionTableMatchDe(const QString &devPath);

    ValidateStates validate() const override;

    Device::Ptr FlushAddPartition(Device::Ptr dev, Partition::Ptr orgPartition, Partition::Ptr newPartition);
    void writeMountOperate();

signals:

public slots:

public:
    QString iniPartitions;

};
}
#endif // CUSTOM_PARTITION_DELEGATE_H
