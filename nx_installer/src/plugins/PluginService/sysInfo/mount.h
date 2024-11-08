#ifndef MOUNT_H
#define MOUNT_H

#include <QObject>
#include <QString>
#include <QList>
namespace KServer {

struct MountItemStr
{
    QString path;
    QString mount;
    QString fs;
    QString options;
};

typedef QList<MountItemStr> MountItemList;

MountItemList parseMountItems();
}
#endif // MOUNT_H
