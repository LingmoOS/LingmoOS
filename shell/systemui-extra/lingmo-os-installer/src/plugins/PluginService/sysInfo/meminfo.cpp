#include "meminfo.h"
#include <QHash>
#include <QDebug>
#include "../base_unit/file_unit.h"

namespace KServer {

MemInfo getMemInfo()
{
    QString content = "";
    content = (QString)KReadFile("/proc/meminfo");
    if (content.isEmpty()) {
        qWarning() << "Failed to read meminfo!";
        return MemInfo();
    }

    QHash<QString, qint64> hash;
    for(const QString& line : content.split('\n')) {
        const int index = line.indexOf(":");
        if(index > -1) {
            QString valueStr = QString("");
            valueStr = line.mid(index + 1);
            valueStr.remove("kB");
            valueStr = valueStr.trimmed();

            const qint64 value = valueStr.toLongLong() * 1024;
            hash.insert(line.left(index), value);
        }
    }
    MemInfo info;
    info.buffers = hash.value("Buffers");
    info.cached = hash.value("Cached");
    info.memAvailable = hash.value("MemAvailabel");
    info.memFree = hash.value("MemFree");
    info.memTotal = hash.value("MemTotal");
    info.swapFree = hash.value("SwapFree");
    info.swapTotal = hash.value("SwapTotal");

    return info;
}

}
