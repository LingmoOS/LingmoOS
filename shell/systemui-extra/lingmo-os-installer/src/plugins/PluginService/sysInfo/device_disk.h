#ifndef DEVICE_DISK_H
#define DEVICE_DISK_H

#include <QObject>
#include <QString>
#include <QHash>
namespace KServer {

typedef QHash<QString, QString>LabelItems;
typedef QHash<QString, QString> PartLableItems;
typedef QHash<QString, QString> UUIDItems;

LabelItems parseLabelDir();
PartLableItems parsePartLabelDir();
UUIDItems parseUUIDDir();

}
#endif // DEVICE_DISK_H
