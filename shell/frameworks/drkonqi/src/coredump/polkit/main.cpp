// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusContext>
#include <QDBusMetaType>
#include <QDBusUnixFileDescriptor>
#include <QFileInfo>
#include <QProcess>
#include <QTemporaryDir>

#include <polkitqt1-agent-session.h>
#include <polkitqt1-authority.h>

#include <coredumpexcavator.h>

using namespace Qt::StringLiterals;

namespace
{
constexpr auto COREDUMP_PATH = "/var/lib/systemd/coredump/"_L1; // The path is hardcoded in systemd's coredump.c
constexpr auto ACTION_NAME = "org.kde.drkonqi.saveCoreToFile"_L1;
} // namespace

class Helper : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.drkonqi")

public Q_SLOTS:
    void saveCoreToFile(const QString &coreName, const QDBusUnixFileDescriptor &targetFileFd)
    {
        auto loopLock = std::make_shared<QEventLoopLocker>();

        if (!isAuthorized(coreName)) {
            qWarning() << "not authorized";
            sendErrorReply(QDBusError::AccessDenied);
            return;
        }

        if (coreName.contains('/'_L1)) { // don't allow anything that could look like a path
            sendErrorReply(QDBusError::AccessDenied);
            return;
        }

        setDelayedReply(true);

        auto connection = this->connection();
        auto msg = message();

        auto coreFileTarget = std::make_shared<QFile>();
        auto dupeFileDescriptor = fcntl(targetFileFd.fileDescriptor(), F_DUPFD_CLOEXEC, 0);
        if (dupeFileDescriptor == -1) {
            const auto error = errno;
            const QString errorString = u"Failed to duplicate file descriptor: (%1) %2"_s.arg(QString::number(error), QString::fromUtf8(strerror(error)));
            qWarning() << errorString;
            sendErrorReply(QDBusError::InternalError, errorString);
            return;
        }
        if (!coreFileTarget->open(dupeFileDescriptor, QFile::WriteOnly, QFile::AutoCloseHandle)) {
            QString errString = u"Failed to open coreFileTarget: %1"_s.arg(coreFileTarget->errorString());
            connection.send(msg.createErrorReply(QDBusError::InternalError, errString));
            return;
        }

        auto excavator = new CoredumpExcavator(this);
        connect(excavator, &CoredumpExcavator::excavated, this, [msg, loopLock, connection, excavator](int exitCode) mutable {
            excavator->deleteLater();

            if (exitCode != 0) {
                connection.send(msg.createErrorReply(QDBusError::InternalError, u"Excavation failed"_s));
                return;
            }

            connection.send(msg.createReply());
        });

        excavator->excavateFromTo(COREDUMP_PATH + coreName, coreFileTarget);
    }

private:
    bool isAuthorized(const QString &coreName)
    {
        auto authority = PolkitQt1::Authority::instance();
        auto result = authority->checkAuthorizationSyncWithDetails(ACTION_NAME,
                                                                   PolkitQt1::SystemBusNameSubject(message().service()),
                                                                   PolkitQt1::Authority::AllowUserInteraction,
                                                                   {
                                                                       {"coreName"_L1, coreName}, //
                                                                   });

        if (authority->hasError()) {
            qWarning() << authority->lastError() << authority->errorDetails();
            authority->clearError();
            return false;
        }

        switch (result) {
        case PolkitQt1::Authority::Yes:
            return true;
        case PolkitQt1::Authority::Unknown:
        case PolkitQt1::Authority::No:
        case PolkitQt1::Authority::Challenge:
            break;
        }
        return false;
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setQuitLockEnabled(true);

    Helper helper;

    if (!QDBusConnection::systemBus().registerObject(QStringLiteral("/"), &helper, QDBusConnection::ExportAllSlots)) {
        qWarning() << "Failed to register the daemon object" << QDBusConnection::systemBus().lastError().message();
        return 1;
    }
    if (!QDBusConnection::systemBus().registerService(QStringLiteral("org.kde.drkonqi"))) {
        qWarning() << "Failed to register the service" << QDBusConnection::systemBus().lastError().message();
        return 1;
    }

    return app.exec();
}

#include "main.moc"
