#ifndef PROBEROS_H
#define PROBEROS_H

#include <QObject>
#include <QVector>
namespace KInstaller {

namespace Partman {
    enum class OSType {
        Linux,
        Windows,
        Mac,
        Unkown,
        Empty
    };

    struct OSProberItemStr{
        QString path;
        QString OSdescription;
        QString bootName;
        OSType osType;

    };
    OSProberItemStr getOsItemInfo(QString path);
    QString readOSProberInfo();
    bool parseOSProberInfo();
    typedef QVector <OSProberItemStr> OSProberItemList;
    OSProberItemList getOsProberItems();
}
}
#endif // PROBEROS_H
