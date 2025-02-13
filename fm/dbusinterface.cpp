#include "dbusinterface.h"

#include <QProcess>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnectionInterface>

DBusInterface::DBusInterface()
{
    QDBusConnection::sessionBus().registerObject("/org/freedesktop/FileManager1", this,
                                                 QDBusConnection::ExportScriptableContents | QDBusConnection::ExportAdaptors);
    QDBusConnectionInterface *sessionInterface = QDBusConnection::sessionBus().interface();

    if (sessionInterface) {
        sessionInterface->registerService(QStringLiteral("org.freedesktop.FileManager1"), QDBusConnectionInterface::QueueService);
    }
}

void DBusInterface::ShowFolders(const QStringList &uriList, const QString &startUpId)
{
    Q_UNUSED(startUpId);

    QProcess::startDetached("lingmo-filemanager", uriList);
}

void DBusInterface::ShowItems(const QStringList &uriList, const QString &startUpId)
{
    Q_UNUSED(startUpId);

    QProcess::startDetached("lingmo-filemanager", uriList);
}

void DBusInterface::ShowItemProperties(const QStringList &uriList, const QString &startUpId)
{
    Q_UNUSED(uriList);
    Q_UNUSED(startUpId);

    // TODO
}
