#include "mount.h"
#include <QDebug>
#include "base_unit/file_unit.h"

namespace KServer {

QList<MountItemStr> parseMountItems()
{
    MountItemList result;
    const QString content(KReadFile("/proc/mounts"));
    const QStringList lines = content.split("\n");

    for(const QString& line : lines) {
        const QStringList parts = line.split(" ", QString::SkipEmptyParts);
        if(parts.length() == 6) {
            const MountItemStr item = {parts[0], parts[1], parts[2], parts[3]};
            result.append(item);
        }
    }
    return (MountItemList)result;
}
}
