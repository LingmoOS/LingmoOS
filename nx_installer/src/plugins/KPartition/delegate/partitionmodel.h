#ifndef PARTITIONMODEL_H
#define PARTITIONMODEL_H

#include <QObject>
#include <QThread>
#include "../partman/device.h"
#include "../partman/partition_server.h"
#include "../partman/operationdisk.h"
#include "../PluginService/backendThread/custominstallerprocess.h"
#include "../PluginService/backendThread/quickinsatllerprocess.h"


namespace KInstaller {
using namespace Partman;
class PartitionModel : public QObject
{
    Q_OBJECT
public:
    explicit PartitionModel(QObject *parent = nullptr);
    ~PartitionModel();

signals:
    void fullPartDone(int execcode);
    void deviceRefreshed(DeviceList devices);
    void customPartDone(int execcode);

    void signalInstallCustomPart(OperationDiskList operators);
    void signalInstallQuickPart();
public slots:

    void fullPart();
    void createPartitionTable(QString dev_path);
    void customPart(OperationDiskList operators);
    void scanDevices();

private:
    void initAllConnects();

    PartitionServer* partition_server;
    QThread* m_partThread;
    QThread* m_backQuickInstallThread;
    QThread* m_backCustomInstallThread;
    QuickInsatllerProcess* m_backFullProcess;
    CustomInstallerProcess* m_customProcess;
    OperationDiskList m_operators;

};
}
#endif // PARTITIONMODEL_H
