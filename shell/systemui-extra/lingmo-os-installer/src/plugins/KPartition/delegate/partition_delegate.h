#ifndef PARTITION_DELEGATE_H
#define PARTITION_DELEGATE_H

#include <QObject>

#include "partman/device.h"
#include "partman/operationdisk.h"
#include "partman/partition.h"
#include "../PluginService/ksystemsetting_unit.h"
#include "validatestate.h"
using namespace KInstaller::Partman;
using namespace KServer;

namespace KInstaller {

enum class PartitionAction {
    CreateLogicalPartition,
    RemeveLogicalPartition,
};


class PartitionDelegate : public QObject
{
    Q_OBJECT
public:
    explicit PartitionDelegate(QObject* parent = nullptr);
    ~PartitionDelegate();

    //get virtual Devices
    inline const DeviceList virtualDevices() const
    {
        return m_virtualDevices;
    }

    //select partition
    virtual void selectPartition(const Partition::Ptr partition);
    //select device
    inline const DeviceList selectedDevices()
    {
        return m_selDevices;
    }
    // select partition
    inline const Partition::Ptr selectedPartition()
    {
        return m_selPartition;
    }
    //
    inline QStringList getOptDescriptios()
    {
        QStringList desc;
        for(OperationDisk& operation : m_operations) {
            desc.append(operation.OperationDiskDescription());
        }
        return desc;
    }

    inline OperationDiskList operations() const
    {
        return m_operations;
    }

    virtual void addSelectDevice(Device::Ptr device);

    virtual void onDeviceRefreshed(DeviceList devices);

    virtual bool setBootFlag();
    virtual bool canAddLogical(const Partition::Ptr partition);
    virtual bool canAddPrimary(const Partition::Ptr partition);

    inline const DeviceList realDevices() const
    {
        return m_realDevices;
    }

    virtual bool isMBRPreferredDe() const;
    virtual bool isPartitionTableMatchDe(const QString& devPath) const;
    virtual ValidateStates validate() const;

    virtual bool createPartition(Partition::Ptr partition,
                                 PartitionType partitionType,
                                 bool alignStart,
                                 FSType fsType,
                                 QString& mountPoint,
                                 qint64 totalSectors,
                                 QString label = QString(),
                                 bool blformat = true);
    virtual bool createPrimaryPartition(Partition::Ptr partition,
                                        PartitionType partitionType,
                                        bool alignStart,
                                        FSType fsType,
                                        QString& mountPoint,
                                        qint64 totalSectors,
                                       QString label = QString(),
                                        bool blformat = true);
    virtual bool createLogicalPartition(Partition::Ptr partition,
                                        bool alignStart,
                                        FSType fsType,
                                        QString& mountPoint,
                                        qint64 totalSectors,
                                       QString label = QString(),
                                        bool blformat = true);


    virtual void deletePartition(const Partition::Ptr partition);
    virtual void formatPartition(const Partition::Ptr partition,
                                 FSType fsType,
                                 QString& mountPoint, bool blformat);
    void createDeviceTable(Device::Ptr& dev);
    void freshVirtualDeviceList();
    virtual void resetOperations();
    virtual void setBootLoaderPath(const QString& path);
    QString getBootLoadPath(){return m_bootLoaderPath;}
    virtual void onCustomPartDone(const DeviceList& devices) = 0;

    inline const DiskPartitionSetting& settings() const
    {
        return m_setting;
    }

    void refreshShow();
    Partition::Ptr getRealPartition(const Partition::Ptr virtualPart) const;
    void resetOperationMountPoint(const QString mountPoint);
    void updateMountPoint(const Partition::Ptr partition, const QString mountPoint);

    bool reCalculateExtPartBoundry(PartitionAction action,
                                   Partition::Ptr curPartition,
                                   qint64& startSector,
                                   qint64& endSector);
    bool reCalculateExtPartBoundry(PartitionList& partitions,
                                   PartitionAction action,
                                   Partition::Ptr curPartition,
                                   qint64 &startSector,
                                   qint64 &endSector);
    Device::Ptr findDevice(const QString& devPath);

    void removeRepetition();


signals:
    void deviceRefreshed(const DeviceList& devices);

protected:
    DeviceList m_realDevices;
    DeviceList m_virtualDevices;
    QString m_bootLoaderPath;

    OperationDiskList m_operations;
    Partition::Ptr m_selPartition;
    int m_primaryPartitionLen;

    QStringList m_selDisks;
    DeviceList m_selDevices;

    DiskPartitionSetting m_setting;


};

}

#endif // PARTITION_DELEGATE_H
