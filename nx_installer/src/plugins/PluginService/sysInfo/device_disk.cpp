#include "device_disk.h"
#include <QDir>


namespace KServer {

QHash<QString, QString> parseDevDir(const QString& dirname)
{
    QHash<QString, QString> result;

    QDir dir(dirname);
    if(!dir.exists()) {
        return result;
    }
    const QFileInfoList items = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files);
    for(const QFileInfo& info : items) {
        result.insert(info.canonicalFilePath(), info.fileName());
    }
    return result;
}

QHash<QString, QString> parseLabelDir()
{
    return parseDevDir("/dev/disk/by-label");
}

QHash<QString, QString> parsePartLabelDir()
{
    return parseDevDir("/dev/disk/by-partlabel");
}

QHash<QString, QString> parseUUIDDir()
{
    return parseDevDir("/dev/disk/by-uuid");
}

}
