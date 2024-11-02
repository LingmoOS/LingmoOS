#ifndef DATASTRUCT_H
#define DATASTRUCT_H

#include <QObject>
namespace KInstaller{
namespace Partman {
// 字节单位换算 https://en.wikipedia.org/wiki/Kibibyte
const qint64 kKibiByte = 1024;
const qint64 kMebiByte = kKibiByte * kKibiByte;
const qint64 kGibiByte = kMebiByte * kKibiByte;
const qint64 kTebiByte = kGibiByte * kKibiByte;
const qint64 kPebiByte = kTebiByte * kKibiByte;
const qint64 kExbiByte = kPebiByte * kKibiByte;

const int MsDosPartitionTableMaxPrims = 4;
const int GPTPartitionTableMaxPrims = 128;

QString GetBiByateValue(QString str);
QString GetByateValue(qint64 tmp);
QString GetKibiByateValue(qint64 tmp);
QString GetMebiByateValue(qint64 tmp);
QString GetGibiByateValue(qint64 tmp);
QString GetTebiByateValue(qint64 tmp);
QString GetPebiByateValue(qint64 tmp);
QString GetExbiByateValue(qint64 tmp);


enum class PartTableType {
    NONE,
    MsDos,
    GPT,
    UnKown,
    Empty,
    Others
};

QString GetPartTableNameByType(PartTableType type);

enum class PartitionType {
    Normal,
    Logical,
    Extended,
    Freespace,
    Metadata,
    Unallocated
};



enum class BYTETYPE {
    Byte,
    KiB,
    MiB,
    Gib,
    TiB,
    PiB,
    EiB,
    ZiB,
    YiB
};

class DataStruct
{



};
}
}
#endif // DATASTRUCT_H
