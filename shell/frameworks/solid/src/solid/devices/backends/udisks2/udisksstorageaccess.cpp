/*
    SPDX-FileCopyrightText: 2009 Pino Toscano <pino@kde.org>
    SPDX-FileCopyrightText: 2009-2012 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "udisksstorageaccess.h"
#include "udisks2.h"
#include "udisks_debug.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMetaType>
#include <QDir>
#include <QGuiApplication>
#include <QWindow>

#include <config-solid.h>
#if HAVE_LIBMOUNT
#include <libmount/libmount.h>
#endif

struct AvailableAnswer {
    bool checkResult;
    QString binaryName;
};
Q_DECLARE_METATYPE(AvailableAnswer)

QDBusArgument &operator<<(QDBusArgument &argument, const AvailableAnswer &answer)
{
    argument.beginStructure();
    argument << answer.checkResult << answer.binaryName;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, AvailableAnswer &answer)
{
    argument.beginStructure();
    argument >> answer.checkResult >> answer.binaryName;
    argument.endStructure();
    return argument;
}

using namespace Solid::Backends::UDisks2;

StorageAccess::StorageAccess(Device *device)
    : DeviceInterface(device)
    , m_setupInProgress(false)
    , m_teardownInProgress(false)
    , m_repairInProgress(false)
    , m_passphraseRequested(false)
{
    qDBusRegisterMetaType<AvailableAnswer>();

    connect(device, SIGNAL(changed()), this, SLOT(checkAccessibility()));
    updateCache();

    // Delay connecting to DBus signals to avoid the related time penalty
    // in hot paths such as predicate matching
    QTimer::singleShot(0, this, SLOT(connectDBusSignals()));
}

StorageAccess::~StorageAccess()
{
}

void StorageAccess::connectDBusSignals()
{
    m_device->registerAction("setup", this, SLOT(slotSetupRequested()), SLOT(slotSetupDone(int, QString)));

    m_device->registerAction("teardown", this, SLOT(slotTeardownRequested()), SLOT(slotTeardownDone(int, QString)));

    m_device->registerAction("repair", this, SLOT(slotRepairRequested()), SLOT(slotRepairDone(int, QString)));
}

bool StorageAccess::isLuksDevice() const
{
    return m_device->isEncryptedContainer(); // encrypted device
}

bool StorageAccess::isAccessible() const
{
    if (isLuksDevice()) { // check if the cleartext slave is mounted
        const QString path = clearTextPath();
        // qDebug() << Q_FUNC_INFO << "CLEARTEXT device path: " << path;
        if (path.isEmpty() || path == "/") {
            return false;
        }
        Device holderDevice(path);
        return holderDevice.isMounted();
    }

    return m_device->isMounted();
}

bool StorageAccess::isEncrypted() const
{
    // FIXME We should also check if physical device is encrypted
    // FIXME Gocryptfs is not supported
    return isLuksDevice() || m_device->isEncryptedCleartext();
}

bool StorageAccess::canCheck() const
{
    const auto idType = m_device->prop("IdType").toString();
    auto c = QDBusConnection::systemBus();
    auto msg = QDBusMessage::createMethodCall(UD2_DBUS_SERVICE, UD2_DBUS_PATH_MANAGER, "org.freedesktop.UDisks2.Manager", "CanCheck");
    msg << idType;
    QDBusReply<AvailableAnswer> r = c.call(msg);
    if (!r.isValid()) {
        qCDebug(UDISKS2) << Q_FUNC_INFO << dbusPath() << idType << "DBus error, code" << r.error().type();
        return false;
    }

    const bool ret = r.value().checkResult;
    qCDebug(UDISKS2) << Q_FUNC_INFO << dbusPath() << idType << ret << r.value().binaryName;
    return ret;
}

bool StorageAccess::check()
{
    if (m_setupInProgress || m_teardownInProgress) {
        return false;
    }

    const auto path = dbusPath();
    auto c = QDBusConnection::systemBus();
    auto msg = QDBusMessage::createMethodCall(UD2_DBUS_SERVICE, path, UD2_DBUS_INTERFACE_FILESYSTEM, "Check");
    QVariantMap options;
    msg << options;
    QDBusReply<bool> r = c.call(msg);
    bool ret = r.isValid() && r.value();
    qCDebug(UDISKS2) << Q_FUNC_INFO << path << ret;
    return ret;
}

bool StorageAccess::canRepair() const
{
    const auto idType = m_device->prop("IdType").toString();
    auto c = QDBusConnection::systemBus();
    auto msg = QDBusMessage::createMethodCall(UD2_DBUS_SERVICE, UD2_DBUS_PATH_MANAGER, "org.freedesktop.UDisks2.Manager", "CanRepair");
    msg << idType;
    QDBusReply<AvailableAnswer> r = c.call(msg);
    if (!r.isValid()) {
        qCDebug(UDISKS2) << Q_FUNC_INFO << dbusPath() << idType << "DBus error, code" << r.error().type();
        return false;
    }

    const bool ret = r.value().checkResult;
    qCDebug(UDISKS2) << Q_FUNC_INFO << dbusPath() << idType << ret << r.value().binaryName;
    return ret;
}

bool StorageAccess::repair()
{
    if (m_teardownInProgress || m_setupInProgress || m_repairInProgress) {
        return false;
    }
    m_repairInProgress = true;
    m_device->broadcastActionRequested("repair");

    const auto path = dbusPath();
    auto c = QDBusConnection::systemBus();
    auto msg = QDBusMessage::createMethodCall(UD2_DBUS_SERVICE, path, UD2_DBUS_INTERFACE_FILESYSTEM, "Repair");
    QVariantMap options;
    msg << options;

    qCDebug(UDISKS2) << Q_FUNC_INFO << path;
    return c.callWithCallback(msg, this, SLOT(slotDBusReply(QDBusMessage)), SLOT(slotDBusError(QDBusError)));
}

static QString baseMountPoint(const QByteArray &dev)
{
    QString mountPoint;

#if HAVE_LIBMOUNT
    // UDisks "MountPoints" property contains multiple paths, this happens with
    // devices with bind mounts; try finding the "base" mount point
    if (struct libmnt_table *table = mnt_new_table()) {
        // This parses "/etc/mtab" if present or "/proc/self/mountinfo" by default
        if (mnt_table_parse_mtab(table, "/proc/self/mountinfo") == 0) {
            // BACKWARD because the fs's we're interested in, /dev/sdXY, are typically at the end
            struct libmnt_iter *itr = mnt_new_iter(MNT_ITER_BACKWARD);
            struct libmnt_fs *fs;

            const QByteArray devicePath = dev.endsWith('\x00') ? dev.chopped(1) : dev;

            while (mnt_table_next_fs(table, itr, &fs) == 0) {
                if (mnt_fs_get_srcpath(fs) == devicePath //
                    && (qstrcmp(mnt_fs_get_root(fs), "/") == 0) // Base mount point will have "/" as root fs
                ) {
                    mountPoint = QFile::decodeName(mnt_fs_get_target(fs));
                    break;
                }
            }

            mnt_free_iter(itr);
        }

        mnt_free_table(table);
    }
#else
    Q_UNUSED(dev);
#endif

    return mountPoint;
}

QString StorageAccess::filePath() const
{
    if (isLuksDevice()) { // encrypted (and unlocked) device
        const QString path = clearTextPath();
        if (path.isEmpty() || path == "/") {
            return QString();
        }
        Device holderDevice(path);
        const auto mntPoints = qdbus_cast<QByteArrayList>(holderDevice.prop("MountPoints"));
        if (!mntPoints.isEmpty()) {
            QByteArray first = mntPoints.first();
            if (first.endsWith('\x00')) {
                first.chop(1);
            }
            return QFile::decodeName(first); // FIXME Solid doesn't support multiple mount points
        } else {
            return QString();
        }
    }

    const auto mntPoints = qdbus_cast<QByteArrayList>(m_device->prop("MountPoints"));
    if (mntPoints.isEmpty()) {
        return {};
    }

    QByteArray first = mntPoints.first();
    if (first.endsWith('\x00')) {
        first.chop(1);
    }
    const QString potentialMountPoint = QFile::decodeName(first);

    if (mntPoints.size() == 1) {
        return potentialMountPoint;
    }

    // Device has bind mounts?
    const QString basePoint = baseMountPoint(m_device->prop("Device").toByteArray());

    return !basePoint.isEmpty() ? basePoint : potentialMountPoint;
}

bool StorageAccess::isIgnored() const
{
    if (m_device->prop("HintIgnore").toBool()) {
        return true;
    }

    const QStringList mountOptions = m_device->prop("UserspaceMountOptions").toStringList();
    if (mountOptions.contains(QLatin1String("x-gdu.hide"))) {
        return true;
    }

    const QString path = filePath();

    const bool inUserPath = (path.startsWith(QLatin1String("/media/")) //
                             || path.startsWith(QLatin1String("/run/media/")) //
                             || path.startsWith(QDir::homePath()));
    return !inUserPath;
}

bool StorageAccess::setup()
{
    if (m_teardownInProgress || m_setupInProgress || m_repairInProgress) {
        return false;
    }
    m_setupInProgress = true;
    m_device->broadcastActionRequested("setup");

    if (m_device->isEncryptedContainer() && clearTextPath().isEmpty()) {
        return requestPassphrase();
    } else {
        return mount();
    }
}

bool StorageAccess::teardown()
{
    if (m_teardownInProgress || m_setupInProgress || m_repairInProgress) {
        return false;
    }
    m_teardownInProgress = true;
    m_device->broadcastActionRequested("teardown");

    return unmount();
}

void StorageAccess::updateCache()
{
    m_isAccessible = isAccessible();
}

void StorageAccess::checkAccessibility()
{
    const bool old_isAccessible = m_isAccessible;
    updateCache();

    if (old_isAccessible != m_isAccessible) {
        Q_EMIT accessibilityChanged(m_isAccessible, m_device->udi());
    }
}

void StorageAccess::slotDBusReply(const QDBusMessage & /*reply*/)
{
    if (m_setupInProgress) {
        if (isLuksDevice() && !isAccessible()) { // unlocked device, now mount it
            mount();
        } else { // Don't broadcast setupDone unless the setup is really done. (Fix kde#271156)
            m_setupInProgress = false;
            m_device->invalidateCache();
            m_device->broadcastActionDone("setup");

            checkAccessibility();
        }
    } else if (m_teardownInProgress) { // FIXME
        const QString ctPath = clearTextPath();
        qCDebug(UDISKS2) << "Successfully unmounted " << m_device->udi();
        if (isLuksDevice() && !ctPath.isEmpty() && ctPath != "/") { // unlocked device, lock it
            callCryptoTeardown();
        } else if (!ctPath.isEmpty() && ctPath != "/") {
            callCryptoTeardown(true); // Lock encrypted parent
        } else {
            // try to "eject" (aka safely remove) from the (parent) drive, e.g. SD card from a reader
            QString drivePath = m_device->drivePath();
            if (!drivePath.isEmpty() || drivePath != "/") {
                Device drive(drivePath);
                QDBusConnection c = QDBusConnection::systemBus();

                if (drive.prop("MediaRemovable").toBool() //
                    && drive.prop("MediaAvailable").toBool() //
                    && !m_device->isOpticalDisc()) { // optical drives have their Eject method
                    QDBusMessage msg = QDBusMessage::createMethodCall(UD2_DBUS_SERVICE, drivePath, UD2_DBUS_INTERFACE_DRIVE, "Eject");
                    msg << QVariantMap(); // options, unused now
                    c.call(msg, QDBus::NoBlock);
                } else if (drive.prop("CanPowerOff").toBool() //
                           && !m_device->isOpticalDisc()) { // avoid disconnecting optical drives from the bus
                    qCDebug(UDISKS2) << "Drive can power off:" << drivePath;
                    QDBusMessage msg = QDBusMessage::createMethodCall(UD2_DBUS_SERVICE, drivePath, UD2_DBUS_INTERFACE_DRIVE, "PowerOff");
                    msg << QVariantMap(); // options, unused now
                    c.call(msg, QDBus::NoBlock);
                }
            }

            m_teardownInProgress = false;
            m_device->invalidateCache();
            m_device->broadcastActionDone("teardown");

            checkAccessibility();
        }
    } else if (m_repairInProgress) {
        qCDebug(UDISKS2) << "Successfully repaired " << m_device->udi();
        m_repairInProgress = false;
        m_device->broadcastActionDone("repair");
    }
}

void StorageAccess::slotDBusError(const QDBusError &error)
{
    // qDebug() << Q_FUNC_INFO << "DBUS ERROR:" << error.name() << error.message();

    if (m_setupInProgress) {
        m_setupInProgress = false;
        m_device->broadcastActionDone("setup", //
                                      m_device->errorToSolidError(error.name()),
                                      m_device->errorToString(error.name()) + ": " + error.message());

        checkAccessibility();
    } else if (m_teardownInProgress) {
        m_teardownInProgress = false;
        m_device->broadcastActionDone("teardown", //
                                      m_device->errorToSolidError(error.name()),
                                      m_device->errorToString(error.name()) + ": " + error.message());
        checkAccessibility();
    } else if (m_repairInProgress) {
        m_repairInProgress = false;
        m_device->broadcastActionDone("repair", m_device->errorToSolidError(error.name()), m_device->errorToString(error.name()) + ": " + error.message());
    }
}

void StorageAccess::slotSetupRequested()
{
    m_setupInProgress = true;
    // qDebug() << "SETUP REQUESTED:" << m_device->udi();
    Q_EMIT setupRequested(m_device->udi());
}

void StorageAccess::slotSetupDone(int error, const QString &errorString)
{
    m_setupInProgress = false;
    // qDebug() << "SETUP DONE:" << m_device->udi();
    checkAccessibility();
    Q_EMIT setupDone(static_cast<Solid::ErrorType>(error), errorString, m_device->udi());
}

void StorageAccess::slotTeardownRequested()
{
    m_teardownInProgress = true;
    Q_EMIT teardownRequested(m_device->udi());
}

void StorageAccess::slotTeardownDone(int error, const QString &errorString)
{
    m_teardownInProgress = false;
    checkAccessibility();
    Q_EMIT teardownDone(static_cast<Solid::ErrorType>(error), errorString, m_device->udi());
}

void StorageAccess::slotRepairRequested()
{
    m_repairInProgress = true;
    Q_EMIT repairRequested(m_device->udi());
}

void StorageAccess::slotRepairDone(int error, const QString &errorString)
{
    m_repairInProgress = false;
    Q_EMIT repairDone(static_cast<Solid::ErrorType>(error), errorString, m_device->udi());
}

bool StorageAccess::mount()
{
    const auto path = dbusPath();

    QDBusConnection c = QDBusConnection::systemBus();
    QDBusMessage msg = QDBusMessage::createMethodCall(UD2_DBUS_SERVICE, path, UD2_DBUS_INTERFACE_FILESYSTEM, "Mount");
    QVariantMap options;

    if (m_device->prop("IdType").toString() == "vfat") {
        options.insert("options", "flush");
    }

    msg << options;

    return c.callWithCallback(msg, this, SLOT(slotDBusReply(QDBusMessage)), SLOT(slotDBusError(QDBusError)));
}

bool StorageAccess::unmount()
{
    const auto path = dbusPath();

    QDBusConnection c = QDBusConnection::systemBus();
    QDBusMessage msg = QDBusMessage::createMethodCall(UD2_DBUS_SERVICE, path, UD2_DBUS_INTERFACE_FILESYSTEM, "Unmount");

    msg << QVariantMap(); // options, unused now

    qCDebug(UDISKS2) << "Initiating unmount of " << path;
    return c.callWithCallback(msg, this, SLOT(slotDBusReply(QDBusMessage)), SLOT(slotDBusError(QDBusError)), s_unmountTimeout);
}

QString StorageAccess::generateReturnObjectPath()
{
    static QAtomicInt number = 1;

    return "/org/kde/solid/UDisks2StorageAccess_" + QString::number(number++);
}

QString StorageAccess::clearTextPath() const
{
    const QString path = m_device->prop("CleartextDevice").value<QDBusObjectPath>().path();
    if (path != QLatin1String("/")) {
        return path;
    }
    return QString();
}

QString StorageAccess::dbusPath() const
{
    QString path = m_device->udi();
    if (isLuksDevice()) { // mount options for the cleartext volume
        const QString ctPath = clearTextPath();
        if (!ctPath.isEmpty()) {
            path = ctPath;
        }
    }
    return path;
}

bool StorageAccess::requestPassphrase()
{
    QString udi = m_device->udi();
    QString returnService = QDBusConnection::sessionBus().baseService();
    m_lastReturnObject = generateReturnObjectPath();

    QDBusConnection::sessionBus().registerObject(m_lastReturnObject, this, QDBusConnection::ExportScriptableSlots);

    // TODO: this only works on X11, Wayland doesn't have global window ids.
    // Passing ids to other processes doesn't make any sense
    auto activeWindow = QGuiApplication::focusWindow();
    uint wId = 0;
    if (activeWindow != nullptr) {
        wId = (uint)activeWindow->winId();
    }

    QString appId = QCoreApplication::applicationName();

    const auto plasmaVersionMajor = qEnvironmentVariable("KDE_SESSION_VERSION", "6");

    // TODO KF6: remove hard dep on Plasma here which provides the SolidUiServer kded plugin
    QDBusInterface soliduiserver(QStringLiteral("org.kde.kded") + plasmaVersionMajor, "/modules/soliduiserver", "org.kde.SolidUiServer");
    QDBusReply<void> reply = soliduiserver.call("showPassphraseDialog", udi, returnService, m_lastReturnObject, wId, appId);
    m_passphraseRequested = reply.isValid();
    if (!m_passphraseRequested) {
        qCWarning(UDISKS2) << "Failed to call the SolidUiServer, D-Bus said:" << reply.error();
    }

    return m_passphraseRequested;
}

void StorageAccess::passphraseReply(const QString &passphrase)
{
    if (m_passphraseRequested) {
        QDBusConnection::sessionBus().unregisterObject(m_lastReturnObject);
        m_passphraseRequested = false;
        if (!passphrase.isEmpty()) {
            callCryptoSetup(passphrase);
        } else {
            m_setupInProgress = false;
            m_device->broadcastActionDone("setup", Solid::UserCanceled);
        }
    }
}

void StorageAccess::callCryptoSetup(const QString &passphrase)
{
    QDBusConnection c = QDBusConnection::systemBus();
    QDBusMessage msg = QDBusMessage::createMethodCall(UD2_DBUS_SERVICE, m_device->udi(), UD2_DBUS_INTERFACE_ENCRYPTED, "Unlock");

    msg << passphrase;
    msg << QVariantMap(); // options, unused now

    c.callWithCallback(msg, this, SLOT(slotDBusReply(QDBusMessage)), SLOT(slotDBusError(QDBusError)));
}

bool StorageAccess::callCryptoTeardown(bool actOnParent)
{
    QDBusConnection c = QDBusConnection::systemBus();
    QDBusMessage msg = QDBusMessage::createMethodCall(UD2_DBUS_SERVICE,
                                                      actOnParent ? (m_device->prop("CryptoBackingDevice").value<QDBusObjectPath>().path()) : m_device->udi(),
                                                      UD2_DBUS_INTERFACE_ENCRYPTED,
                                                      "Lock");
    msg << QVariantMap(); // options, unused now

    return c.callWithCallback(msg, this, SLOT(slotDBusReply(QDBusMessage)), SLOT(slotDBusError(QDBusError)));
}

#include "moc_udisksstorageaccess.cpp"
