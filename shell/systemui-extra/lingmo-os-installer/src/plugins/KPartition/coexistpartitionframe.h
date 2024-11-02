#ifndef COEXISTPARTITIONFRAME_H
#define COEXISTPARTITIONFRAME_H

#include <QWidget>
#include "partman/partition_server.h"
namespace KInstaller{
using namespace Partman;
class CoexistPartitionFrame : public QWidget
{
    Q_OBJECT
public:
    explicit CoexistPartitionFrame(DeviceList devs, QWidget *parent = nullptr);

    DeviceList devlist;
signals:

public slots:
};
}
#endif // COEXISTPARTITIONFRAME_H
