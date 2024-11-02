#ifndef DISKTYPE_NAME_H
#define DISKTYPE_NAME_H

#include <QString>

enum class DiskType {
    WD,
    Seagate,
    Hitachi,
    Samsung,
    Toshiba,
    Fujitsu,
    Maxtor,
    IBM,
    ExcelStor,
    LENOVO,
    Unknown,
    Other,
};

QString findNameByDiskType(DiskType type);
#endif // DISKTYPE_NAME_H
