#include "datastruct.h"
#include <QMap>

namespace KInstaller{
namespace Partman {
QString GetBiByateValue(QString str)
{
    QString tmpstr = str;
    qint64 tmp = str.toLongLong();
    int length = tmpstr.length();
/*    if(length < 3 && length >= 0) {
       return GetByateValue(tmp);
    } else if(length >= 3 && length < 7) {
       return GetKibiByateValue(tmp);
    } else*/ if(/*length >= 7 && */length < 11) {
       return GetMebiByateValue(tmp);
    } else if(length >= 11 && length < 15) {
       return GetGibiByateValue(tmp);
    }  else if(length >= 15 && length < 19) {
       return GetTebiByateValue(tmp);
    } else if(length >= 19 && length < 23) {
       return GetPebiByateValue(tmp);
    } else if(length >= 23 && length < 27) {
       return GetExbiByateValue(tmp);
    }

}

QString GetByateValue(qint64 tmp)
{
    QString reslult = "";
    reslult = QString("%1Byte").arg(tmp);
    return (QString)reslult;
}

QString GetKibiByateValue(qint64 tmp)
{
    double tmp1 = 1.0 * tmp / 1024;
    QString reslult = "";
    reslult = QString("%1KiB").arg(QString::number(tmp1, 'f', 1));
    return (QString)reslult;
}
QString GetMebiByateValue(qint64 tmp)
{
    double tmp1 = 1.0 *  tmp / 1024 / 1024;
    QString reslult = "";
    reslult = QString("%1MiB").arg(QString::number(tmp1, 'f', 1));
    return (QString)reslult;
}
QString GetGibiByateValue(qint64 tmp)
{
    double tmp1 = 1.0 *  tmp / 1024 / 1024 / 1024;
    QString reslult = "";
    reslult = QString("%1GiB").arg(QString::number(tmp1, 'f', 1));
    return (QString)reslult;
}
QString GetTebiByateValue(qint64 tmp)
{
    double tmp1 = 1.0 * tmp / 1024 / 1024 / 1024 / 1024;
    QString reslult = "";
    reslult = QString("%1TiB").arg(QString::number(tmp1, 'f', 1));
    return (QString)reslult;
}
QString GetPebiByateValue(qint64 tmp)
{
    double tmp1 = 1.0 * tmp / 1024 / 1024 / 1024 / 1024 / 1024;
    QString reslult = "";
    reslult = QString("%1PiB").arg(QString::number(tmp1, 'f', 1));
    return (QString)reslult;
}
QString GetExbiByateValue(qint64 tmp)
{
    double tmp1 = 1.0 * tmp / 1024 / 1024 / 1024 / 1024 / 1024 / 1024;
    QString reslult = "";
    reslult = QString("%1EiB").arg(QString::number(tmp1, 'f', 1));
    return (QString)reslult;
}


QString GetPartTableNameByType(PartTableType type)
{
    switch (type) {
    case PartTableType::GPT:
        return "gpt";
    case PartTableType::Empty:
        return "Empty";
    case PartTableType::MsDos:
        return "msdos";
    case PartTableType::NONE:
    case PartTableType::Others:
    case PartTableType::UnKown:

    default:
        return "unkown";
    }
}


}

}
