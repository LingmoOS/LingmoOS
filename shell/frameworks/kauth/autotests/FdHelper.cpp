#include "FdHelper.h"

#include <helpersupport.h>

#include <QCoreApplication>
#include <QDBusUnixFileDescriptor>
#include <QDebug>
#include <QFile>
#include <QTimer>

#include <unistd.h>

ActionReply FdHelper::standardaction(QVariantMap args)
{
    QDBusUnixFileDescriptor tempFileFd = args[QStringLiteral("tempFileDescriptor")].value<QDBusUnixFileDescriptor>();
    QFile tempFile;
    args.remove(QStringLiteral("tempFileDescriptor"));

    if (!tempFileFd.isValid()) {
        return ActionReply::HelperErrorReply();
    }

    if (!tempFile.open(tempFileFd.fileDescriptor(), QIODevice::ReadOnly)) {
        return ActionReply::HelperErrorReply();
    }

    QByteArray blob = tempFile.readAll();

    QVariantMap arguments;
    QDataStream s(&blob, QIODevice::ReadOnly);
    s >> arguments;

    if (args == arguments) {
        return ActionReply::SuccessReply();
    }
    return ActionReply::HelperErrorReply();
}

KAUTH_HELPER_MAIN("org.kde.kf6auth.autotest", FdHelper)

#include "moc_FdHelper.cpp"
