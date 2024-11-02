#include "disktype_name.h"
#include <QObject>

QString findNameByDiskType(DiskType type)
{
    QString str = "";
    switch (type) {
    case DiskType::WD:
        return str = QObject::tr("wd");
    case DiskType::Seagate:
        return str = QObject::tr("seagate");
    case DiskType::Hitachi:
        return str = QObject::tr("hitachi");
    case DiskType::Samsung:
        return str = QObject::tr("samsung");
    case DiskType::Toshiba:
        return str = QObject::tr("toshiba");
    case DiskType::Fujitsu:
        return str = QObject::tr("fujitsu");
    case DiskType::Maxtor:
        return str = QObject::tr("maxtor");
    case DiskType::IBM:
        return str = QObject::tr("IBM");
    case DiskType::ExcelStor:
        return str = QObject::tr("excelStor");
    case DiskType::LENOVO:
        return str = QObject::tr("lenovo");
    case DiskType::Other:
        return str = QObject::tr("other");
    default:
        return str = QObject::tr("Unknown");
        break;
    }
}
