#ifndef PARTITION_SERVER_H
#define PARTITION_SERVER_H

#include <QObject>
#include "device.h"
#include "datastruct.h"
#include "operationdisk.h"

namespace KInstaller {

namespace Partman {
class PartitionServer : public QObject
{
    Q_OBJECT
public:
    explicit PartitionServer(QObject *parent = nullptr);
    ~PartitionServer();
    int execScript();

signals:
    void signalRefreshDevices(bool umount, bool enable_osprober);
    void signalDevicesRefreshed(DeviceList& devs);

    void signalCreatePartitionTable(QString& dev_path, PartTableType table);

    void signalQuickPart();
    void signalQuikPartDone();

    void signalCustomPart(OperationDiskList& operations);
    void signalCustomPartDone(bool exitcode, DeviceList& devs);

private:
    void initAllConnect();


public slots:

    void doCreatePartitionTable(QString& devPath, PartTableType table);
    void doRefreshDevices(bool umount, bool osprober_enable);
    void doQuickPart();
    void doCustomPart(OperationDiskList &operations);

public:
    bool m_blosprober;
    DeviceList m_deviceListl;
};
bool UnmountDevices();
DeviceList scanAllDevices(bool osprober_enable);
}
}
#endif // PARTITION_SERVER_H
