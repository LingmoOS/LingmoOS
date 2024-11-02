#include "sysdbusinterface.h"

#include <QFileInfo>
#include <QDir>
#include <file-utils.h>

#include "lingmo-log4qt.h"

sysdbusinterface::sysdbusinterface()
{

    QDBusConnection systemBus = QDBusConnection::systemBus();
    if (!systemBus.registerObject("/", this, QDBusConnection::ExportAllSlots))
    {
        KyWarning() << "QDbus register object failed reason:" << systemBus.lastError();
    }
}


sysdbusinterface::~sysdbusinterface()
{

}

QString sysdbusinterface::getProperFilePath(QString user, QString filename)
{
    KyInfo() << user << filename;
    QString dest;
    dest = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/" + filename;

    KyDebug() << dest;
    while (QFile::exists(dest))
    {
        QStringList newUrl = dest.split("/");
        newUrl.pop_back();
        newUrl.append(Peony::FileUtils::handleDuplicateName(Peony::FileUtils::urlDecode(dest)));
        dest = newUrl.join("/");
        KyDebug() << dest << newUrl;
    }

    return dest;
}
