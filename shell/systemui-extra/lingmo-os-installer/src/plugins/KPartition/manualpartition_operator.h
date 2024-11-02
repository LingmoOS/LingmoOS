#ifndef MANUALPARTITION_OPERATOR_H
#define MANUALPARTITION_OPERATOR_H

#include <QObject>
#include "partman/device.h"
#include "partman/partition.h"
using namespace KInstaller;
using namespace Partman;

class ManualPartition_operator : public QObject
{
    Q_OBJECT
public:
    explicit ManualPartition_operator(QObject *parent = nullptr);
    ManualPartition_operator(Device* dev, Partition* partition, QStringList mountpoints, QWidget *parent = nullptr);

signals:

public slots:
    bool creatorPartTable();
    bool creatorPartition();
    bool delPartition();
    bool formatPartition();
    bool restorePartitionTable();
    bool editPartition();
};

#endif // MANUALPARTITION_OPERATOR_H
