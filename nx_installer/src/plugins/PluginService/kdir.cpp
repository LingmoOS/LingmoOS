#include "kdir.h"

namespace KServer {

//读取主项目Lingmo-installer_new路径
QString GetLingmoInstallPath()
{
#if 0
    QDir dir(QDir::currentPath());
    dir.cdUp();
    dir.cdUp();
    dir.cdUp();

    QString path = dir.path();
    path.append("/lingmo-os-installer");
#else
    QString path = "/usr/share/lingmo-os-installer/";
#endif
    return (QString)path;
}

}
