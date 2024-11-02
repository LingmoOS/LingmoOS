#include "proberos.h"
#include <QDir>
#include <QDebug>
#include "../PluginService/kcommand.h"
using namespace KServer;
namespace KInstaller {

namespace Partman {
QString readOSProberInfo()
{
    QString output = "";
    QString error = "";
    int exitcode = 0;
    QString program = "os-prober";
    QStringList args = {};
    QString path = "";
    path = QDir::currentPath();
    if(KServer::KCommand::getInstance()->RunScripCommand(program,
                            args,
                            path,
                            output,
                            error,
                            exitcode)) {
        return (QString)output;
    } else {
        return (QString)error;
    }

}

OSProberItemStr getOsItemInfoBy(QString path)
{
//    if(m_osProberList.count() > 0) {
//        for(OSProberItemStr osStr: m_osProberList) {
//            if(osStr.path.startsWith(path)) {
//                return  osStr;
//            }
//        }
//    }

}

OSType getOSTypeByName(QString typeName)
{
    QString lowerTypeName = typeName.toLower();
    if(lowerTypeName.isEmpty()) {
        qWarning() << "typeName is null";
    }
    if(lowerTypeName.contains("linux")) {
        return OSType::Linux;
    } else if(lowerTypeName.contains("windows")) {
        return OSType::Windows;
    } else if(lowerTypeName.contains("mac")) {
        return OSType::Mac;
    } else {
        return OSType::Unkown;
    }

}

bool parseOSProberInfo()
{
    QString OSInfo = readOSProberInfo();
    if(!OSInfo.isEmpty()) {
        QStringList osList = OSInfo.split("\n");
        for(int i = 0; i < osList.count(); i++) {
            QString str = osList.at(i);
            if(str.split(":").count() == 4) {
                OSProberItemStr osStr;
                osStr.path = str.split(":").at(0);
                osStr.osType = getOSTypeByName(str.split(":").at(2));
                osStr.OSdescription = str.split(":").at(1);
                osStr.bootName = str.split(":").at(3);
                //m_osProberList.append(osStr);
            } else {
                continue;
            }
        }
        return true;
    } else {
        return false;
    }
}

OSProberItemList getOsProberItems()
{
    OSProberItemList result = {};
    const QString osInfo = readOSProberInfo();
    if(!osInfo.isEmpty()) {
        for(const QString& line : osInfo.split('\n')) {
            if(line.isEmpty()) {
                continue;
            }

            const QStringList items = line.split(':');
            Q_ASSERT(items.length() == 4);

            if(items.length() != 4 ) {
                continue;
            }

            QString desc = items.at(1);
            const int leftindex = desc.indexOf('(');
            if(leftindex > -1) {
                desc = desc.left(leftindex).trimmed();
            }
            const QString bootname = items.at(2);

            OSType type;
            if(line.contains("linux", Qt::CaseInsensitive)) {
                type = OSType::Linux;
            } else if(line.contains("windows", Qt::CaseInsensitive)) {
                type = OSType::Windows;
            } else if(line.contains("mac", Qt::CaseInsensitive)) {
                type = OSType::Mac;
            } else {
                type = OSType::Unkown;
            }

            const QString devpath = items[0];
            const int index = devpath.indexOf('@');
            const QString path = (index == -1) ? devpath : devpath.left(index);

            const OSProberItemStr proberitem = {path, desc, bootname, type};

            bool exist = false;
            for(const OSProberItemStr& item : result) {
                if(item.path == path &&
                        item.osType == type &&
                        item.bootName == bootname &&
                        item.OSdescription == desc) {
                    exist = true;
                }

            }
            if(!exist) {
                result.append(proberitem);
            }
        }
    }
    return (OSProberItemList)result;
}


}

}
