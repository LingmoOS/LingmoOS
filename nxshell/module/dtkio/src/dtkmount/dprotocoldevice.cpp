// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <DProtocolDevice>
#include "dprotocoldevice_p.h"
#include "utils.h"

#include <QTimer>
#include <QRegularExpression>
#include <QDebug>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QtConcurrent/QtConcurrent>

DMOUNT_USE_NAMESPACE

struct DataAskQuestion
{
    AskForChoice ask { nullptr };
};
struct DataAskPasswd
{
    AskForPasswd ask { nullptr };
    bool callOnceFlag { false };
    bool anonymous { false };
};
struct DataFinalCallback   // TODO(xust): error handle
{
    DataAskQuestion askQuestion;
    DataAskPasswd askPasswd;

    OperateCallback finalCallback1 { nullptr };
    OperateCallbackWithInfo finalCallback2 { nullptr };
    QEventLoop *blocker { nullptr };
    QPointer<DProtocolDevicePrivate> d { nullptr };

    QTimer timer;
    bool timeoutEnabled { false };
};

DProtocolDevice::DProtocolDevice(const QString &devicePath, QObject *parent)
    : QObject { parent }, d_ptr { new DProtocolDevicePrivate(devicePath, this) }
{
}

DProtocolDevice::~DProtocolDevice()
{
}

QString DProtocolDevice::path() const
{
    Q_D(const DProtocolDevice);
    return d->devicePath;
}

QString DProtocolDevice::mount(const QVariantMap &)
{
    Q_D(DProtocolDevice);
    // since GLib do not provide a sync method,
    // using QEventLoop to convert from async method.
    QEventLoop blocker;
    QString info;
    bool result { false };
    auto callback = [&](bool ok, const QString &msg) {
        result = ok;
        info = msg;
    };

    if (d->mount)
        return mountPoint();
    else if (d->volume)
        d->mountWithVolume(callback, &blocker);
    else
        d->mountNetworkDevice(callback, &blocker);

    blocker.exec();
    return info;
}

void DProtocolDevice::mountAsync(const QVariantMap &, OperateCallbackWithInfo callback)
{
    Q_D(DProtocolDevice);
    if (d->mount) {
        const auto &&mpt = mountPoint();
        if (callback)
            callback(true, mpt);
    } else if (d->volume) {
        d->mountWithVolume(callback);
    } else {
        d->mountNetworkDevice(callback);
    }
}

bool DProtocolDevice::unmount(const QVariantMap &)
{
    Q_D(DProtocolDevice);
    if (d->mount) {
        bool result { false };
        auto callback = [&](bool ok) { result = ok; };
        QEventLoop blocker;

        const QString &&mpt = mountPoint();
        static QRegularExpression regx(Utils::kRegxDaemonMountPath);
        if (mpt.contains(regx)) {
            if (DProtocolDevicePrivate::isDaemonMountEnable()) {
                d->unmountWithDaemon(mpt, callback, &blocker);
            } else {
                qWarning() << "mount: daemon is not enable cannot unmount by daemon";
                return false;
            }
        } else {
            d->unmountWithGvfs(callback, &blocker);
        }
        blocker.exec();
        return result;
    } else {
        return true;
    }
}

void DProtocolDevice::unmountAsync(const QVariantMap &, OperateCallback callback)
{
    Q_D(DProtocolDevice);
    if (d->mount) {
        const auto &&mpt = mountPoint();
        static QRegularExpression regx(Utils::kRegxDaemonMountPath);
        if (mpt.contains(regx)) {
            if (DProtocolDevicePrivate::isDaemonMountEnable())
                d->unmountWithDaemon(mpt, callback);
            else
                qWarning() << "mount: daemon is not running, cannot unmount device at" << mpt;
        } else {
            d->unmountWithGvfs(callback);
        }
    } else if (callback) {   // no mount object means it's not mounted.
        callback(true);
    }
}

QString DProtocolDevice::mountPoint() const
{
    Q_D(const DProtocolDevice);
    return d->mount ? Utils::mountPointOf(d->mount) : "";
}

QString DProtocolDevice::displayName() const
{
    Q_D(const DProtocolDevice);
    if (d->volume) {
        g_autofree char *name = g_volume_get_name(d->volume);
        return QString(name);
    } else if (d->mount) {
        g_autofree char *name = g_mount_get_name(d->mount);
        return QString(name);
    } else {
        qWarning() << "mount: not mounted to invoke" << __FUNCTION__;
        return "";
    }
}

QString DProtocolDevice::fileSystem() const
{
    Q_D(const DProtocolDevice);
    return d->queryAttrs(DProtocolDevicePrivate::FileSystem).toString();
}

quint64 DProtocolDevice::sizeTotal() const
{
    Q_D(const DProtocolDevice);
    return d->queryAttrs(DProtocolDevicePrivate::Total).toULongLong();
}

quint64 DProtocolDevice::sizeFree() const
{
    Q_D(const DProtocolDevice);
    return d->queryAttrs(DProtocolDevicePrivate::Free).toULongLong();
}

quint64 DProtocolDevice::sizeUsage() const
{
    Q_D(const DProtocolDevice);
    return d->queryAttrs(DProtocolDevicePrivate::Usage).toULongLong();
}

QStringList DProtocolDevice::iconNames() const
{
    Q_D(const DProtocolDevice);
    if (!d->icons.isEmpty())
        return d->icons;

    GIcon_autoptr icon { nullptr };
    if (d->volume)
        icon = g_volume_get_icon(d->volume);
    else if (d->mount)
        icon = g_mount_get_icon(d->mount);
    else
        return {};

    if (!icon)
        return {};

    g_autofree char *names = g_icon_to_string(icon);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (names)
        d->icons = QString(names).remove(". GThemedIcon").split(" ", Qt::SkipEmptyParts);
#else
    if (names)
        d->icons = QString(names).remove(". GThemedIcon").split(" ", QString::SkipEmptyParts);
#endif

    return d->icons;
}

void DProtocolDevice::setOperateTimeout(int secs)
{
    Q_ASSERT(secs >= 0);
    Q_D(DProtocolDevice);
    d->timeout = secs;
}

void DProtocolDevice::setAskForPasswd(AskForPasswd callback)
{
    Q_D(DProtocolDevice);
    d->askForPasswd = callback;
}

void DProtocolDevice::setAskForChoice(AskForChoice callback)
{
    Q_D(DProtocolDevice);
    d->askForChoice = callback;
}

DProtocolDevicePrivate::DProtocolDevicePrivate(const QString &devicePath, DProtocolDevice *qq)
    : QObject { qq }, devicePath { devicePath }, q_ptr { qq }
{
    if (!devicePath.endsWith("/"))
        this->devicePath = devicePath + "/";

    initVolume();
    initMount();
}

QVariant DProtocolDevicePrivate::queryAttrs(Attr attr) const
{
    // not mounted or apple afc file is not supported to obtain properties
    if (!mount || devicePath.startsWith("afc://"))
        return QVariant();

    switch (attr) {
    case FileSystem:
        if (!attrs.value(FileSystem, QString()).toString().isEmpty())
            return attrs.value(FileSystem);
        break;
    case Total:
        if (attrs.value(Total, 0).toLongLong() != 0)
            return attrs.value(Total);
        break;
    default:
        break;
    }

    GFile_autoptr devFile = g_file_new_for_uri(devicePath.toStdString().c_str());
    if (!devFile) {
        // TODO(xust): error handle.
        return QVariant();
    }

    static const QStringList attrs {
        G_FILE_ATTRIBUTE_FILESYSTEM_SIZE,
        G_FILE_ATTRIBUTE_FILESYSTEM_FREE,
        G_FILE_ATTRIBUTE_FILESYSTEM_USED,
        G_FILE_ATTRIBUTE_FILESYSTEM_TYPE
    };

    GError_autoptr err { nullptr };
    GCancellable_autoptr cancel = nullptr;
    QObject obj;
    if (timeout != 0) {
        cancel = g_cancellable_new();
        QTimer::singleShot(timeout * 1000, &obj, [&] { QtConcurrent::run([&] { g_cancellable_cancel(cancel); }); });
    }

    GFileInfo_autoptr fsInfo = g_file_query_filesystem_info(devFile, attrs.join(",").toStdString().c_str(),
                                                            cancel, &err);
    if (err) {
        // TODO(xust): error handle
        return QVariant();
    }

    quint64 total = g_file_info_get_attribute_uint64(fsInfo, G_FILE_ATTRIBUTE_FILESYSTEM_SIZE);
    quint64 free = g_file_info_get_attribute_uint64(fsInfo, G_FILE_ATTRIBUTE_FILESYSTEM_FREE);
    quint64 used = g_file_info_get_attribute_uint64(fsInfo, G_FILE_ATTRIBUTE_FILESYSTEM_USED);
    QString fs = g_file_info_get_attribute_as_string(fsInfo, G_FILE_ATTRIBUTE_FILESYSTEM_TYPE);

    this->attrs.insert(FileSystem, fs);
    this->attrs.insert(Total, total);
    this->attrs.insert(Free, free);
    this->attrs.insert(Usage, used);
    return this->attrs.value(attr, QVariant());
}

void DProtocolDevicePrivate::initVolume()
{
    GVolumeMonitor_autoptr monitor = g_volume_monitor_get();
    Q_ASSERT(monitor);

    GList_autoptr vols = g_volume_monitor_get_volumes(monitor);
    while (vols) {
        GVolume *vol = static_cast<GVolume *>(vols->data);
        GFile_autoptr activeRoot = g_volume_get_activation_root(vol);
        if (activeRoot) {
            g_autofree char *cUri = g_file_get_uri(activeRoot);
            if (QString(cUri) == this->devicePath) {
                this->volume = static_cast<GVolume *>(g_object_ref(vol));
                break;
            }
        }
        vols = vols->next;
    }
}

void DProtocolDevicePrivate::initMount()
{
    GVolumeMonitor_autoptr monitor = g_volume_monitor_get();
    Q_ASSERT(monitor);

    GList_autoptr mounts = g_volume_monitor_get_mounts(monitor);
    while (mounts) {
        GMount *mount = static_cast<GMount *>(mounts->data);
        GFile_autoptr root = g_mount_get_root(mount);
        if (root) {
            g_autofree char *cUri = g_file_get_uri(root);
            if (QString(cUri) == this->devicePath) {
                this->mount = static_cast<GMount *>(g_object_ref(mount));
                break;
            }
        }
        mounts = mounts->next;
    }
}

void DProtocolDevicePrivate::mountNetworkDevice(OperateCallbackWithInfo callback, QEventLoop *blocker)
{
    // only SAMBA is supported to mount by daemon FOR NOW.
    if (devicePath.startsWith("smb://") && isDaemonMountEnable())
        mountNetworkWithDaemon(callback, blocker);
    else
        mountNetworkWithGvfs(callback, blocker);
}

void DProtocolDevicePrivate::mountNetworkWithDaemon(OperateCallbackWithInfo /*callback*/, QEventLoop * /*blocker*/)
{
    // TODO(xust): impl me daemon
}

void DProtocolDevicePrivate::mountNetworkWithGvfs(OperateCallbackWithInfo callback, QEventLoop *blocker)
{
    if (devicePath.startsWith("ftp") && timeout != 0)
        devicePath += QString("?socket_timeout=%1").arg(timeout);

    GFile_autoptr file = g_file_new_for_uri(devicePath.toStdString().c_str());
    if (!file) {
        qWarning() << "mount: cannot new file for" << devicePath;
        if (callback)
            callback(false, "");
        return;
    }

    DataFinalCallback *datFinal = new DataFinalCallback;
    datFinal->askPasswd.ask = askForPasswd;
    datFinal->askQuestion.ask = askForChoice;
    datFinal->finalCallback2 = callback;
    datFinal->blocker = blocker;
    datFinal->d = this;
    datFinal->timer.setInterval(timeout * 1000);
    datFinal->timer.setSingleShot(true);

    GCancellable_autoptr cancel = nullptr;
    if (timeout != 0) {
        datFinal->timeoutEnabled = true;
        cancel = g_cancellable_new();
        connect(&datFinal->timer, &QTimer::timeout, this, [this, datFinal, cancel] {
            g_cancellable_cancel(cancel);
            if (datFinal->blocker) {
                QMutexLocker locker(&lockForLoop);
                datFinal->blocker->quit();
            }
        });
        datFinal->timer.start();
    }

    GMountOperation_autoptr op = g_mount_operation_new();
    g_signal_connect(op, "ask_question", G_CALLBACK(askQuestion), datFinal);
    g_signal_connect(op, "ask_password", G_CALLBACK(askPasswd), datFinal);

    g_file_mount_enclosing_volume(file, G_MOUNT_MOUNT_NONE, op, cancel, mountWithGvfsCb, datFinal);
}

void DProtocolDevicePrivate::mountWithGvfsCb(GObject *obj, GAsyncResult *res, gpointer data)
{
    DataFinalCallback *datFinal = static_cast<DataFinalCallback *>(data);
    Q_ASSERT(datFinal);

    auto file = reinterpret_cast<GFile *>(obj);
    GError_autoptr err { nullptr };   // TODO(xust): error handle
    bool ret = g_file_mount_enclosing_volume_finish(file, res, &err);

    g_autofree char *path = g_file_get_path(file);

    if (datFinal->finalCallback2)
        datFinal->finalCallback2(ret, QString(path));

    if (datFinal->d)
        datFinal->d->initMount();

    if (datFinal->blocker && datFinal->d) {
        QMutexLocker locker(&datFinal->d->lockForLoop);
        datFinal->blocker->quit();
    }

    delete datFinal;
}

void DProtocolDevicePrivate::askQuestion(GMountOperation *op, const char *msg, const char **choices, void *data)
{
    DataFinalCallback *dat = static_cast<DataFinalCallback *>(data);
    Q_ASSERT(dat);

    auto &askQuestion = dat->askQuestion;
    QStringList qChoices;
    while (*choices)
        qChoices << QString::asprintf("%s", *choices++);

    int choice = askQuestion.ask ? askQuestion.ask(msg, qChoices) : 0;

    if (dat->timeoutEnabled)
        dat->timer.start();

    if (choice < 0 || choice >= qChoices.count()) {
        g_mount_operation_reply(op, G_MOUNT_OPERATION_ABORTED);
        return;
    }

    g_mount_operation_set_choice(op, choice);
    g_mount_operation_reply(op, G_MOUNT_OPERATION_HANDLED);
}

void DProtocolDevicePrivate::askPasswd(GMountOperation *op, const char *msg, const char *defUser, const char *defDomain, GAskPasswordFlags flgs, void *data)
{
    DataFinalCallback *dat = static_cast<DataFinalCallback *>(data);
    Q_ASSERT(dat);

    auto &askPasswd = dat->askPasswd;
    if (askPasswd.callOnceFlag) {   // __FUNCTION__ has been invoked and still entered
        if (askPasswd.anonymous) {
            qDebug() << "mount: anonymous mount is not allowed";
            // TODO(xust): error handle. give information that server rejected anonymous mount.
        } else {
            // TODO(xust): error handle. tells that auth failed.
            qDebug() << "mount: auth failed";
        }
        // and then finish the mount process.
        g_mount_operation_reply(op, G_MOUNT_OPERATION_UNHANDLED);
        return;
    } else {   // the __FUNCTION__ is first invoked.
        askPasswd.callOnceFlag = true;
    }

    using namespace MountPasswdInfoParamKeys;
    // if ask-password callback is not specified, trying mount with default user infos.
    QVariantMap info { { kName, defUser },
                       { kPasswd, "" },
                       { kDomain, defDomain } };
    auto passInfo = askPasswd.ask ? askPasswd.ask(msg, defUser, defDomain) : info;

    if (dat->timeoutEnabled)
        dat->timer.start();

    if (passInfo.value(kCancel, false).toBool()) {
        g_mount_operation_reply(op, G_MOUNT_OPERATION_ABORTED);
        // TODO(xust): error handle
        qDebug() << "mount: user cancelled mount";
        return;
    }

    if (passInfo.value(kAnonymous, false).toBool()) {
        if (flgs & G_ASK_PASSWORD_ANONYMOUS_SUPPORTED) {
            askPasswd.anonymous = true;
            g_mount_operation_set_anonymous(op, true);
        } else {
            // TODO(xust): error handle. tells that anonymous is not allowed
            qDebug() << "mount: anonymous mount is not allowed in flags";
        }
    } else {
        if (flgs & G_ASK_PASSWORD_NEED_DOMAIN)
            g_mount_operation_set_domain(op, passInfo.value(kDomain, "").toString().toStdString().c_str());
        if (flgs & G_ASK_PASSWORD_NEED_USERNAME)
            g_mount_operation_set_username(op, passInfo.value(kName, "").toString().toStdString().c_str());
        if (flgs & G_ASK_PASSWORD_NEED_PASSWORD)
            g_mount_operation_set_password(op, passInfo.value(kPasswd, "").toString().toStdString().c_str());
        g_mount_operation_set_password_save(op, GPasswordSave(passInfo.value(kSaveMode, Never).toInt()));
    }
    g_mount_operation_reply(op, G_MOUNT_OPERATION_HANDLED);
}

void DProtocolDevicePrivate::mountWithVolume(OperateCallbackWithInfo callback, QEventLoop *blocker)
{
    Q_ASSERT(volume);

    DataFinalCallback *datFinal = new DataFinalCallback();
    datFinal->finalCallback2 = callback;
    datFinal->blocker = blocker;
    datFinal->d = this;
    datFinal->timer.setInterval(timeout * 1000);
    datFinal->timer.setSingleShot(true);

    GCancellable_autoptr cancel = nullptr;
    if (timeout != 0) {
        cancel = g_cancellable_new();
        datFinal->timeoutEnabled = true;
        connect(&datFinal->timer, &QTimer::timeout, this, [this, datFinal, cancel] {
            g_cancellable_cancel(cancel);
            if (datFinal->blocker) {
                QMutexLocker locker(&lockForLoop);
                datFinal->blocker->quit();
            }
        });
        datFinal->timer.start();
    }

    g_volume_mount(volume, G_MOUNT_MOUNT_NONE, nullptr, cancel, mountWithVolumeCb, datFinal);
}

void DProtocolDevicePrivate::mountWithVolumeCb(GObject *obj, GAsyncResult *res, gpointer data)
{
    DataFinalCallback *datFinal = static_cast<DataFinalCallback *>(data);
    Q_ASSERT(datFinal);

    QString mountPoint;
    GVolume *vol = reinterpret_cast<GVolume *>(obj);
    GError_autoptr err { nullptr };
    bool ret = g_volume_mount_finish(vol, res, &err);
    if (ret) {
        GMount *mount = g_volume_get_mount(vol);
        if (datFinal->d)
            datFinal->d->mount = mount;

        mountPoint = Utils::mountPointOf(mount);
    }

    if (datFinal->blocker && datFinal->d) {
        QMutexLocker locker(&datFinal->d->lockForLoop);
        datFinal->blocker->quit();
    }

    if (datFinal->finalCallback2)
        datFinal->finalCallback2(ret, mountPoint);

    delete datFinal;
}

void DProtocolDevicePrivate::unmountWithDaemon(const QString & /*mpt*/, OperateCallback /*callback*/, QEventLoop * /*blocker*/)
{
    // TODO(xust): impl me daemon
}

void DProtocolDevicePrivate::unmountWithGvfs(OperateCallback callback, QEventLoop *blocker)
{
    GCancellable_autoptr cancel { nullptr };

    DataFinalCallback *datFinal = new DataFinalCallback();
    datFinal->finalCallback1 = callback;
    datFinal->d = this;
    datFinal->blocker = blocker;
    datFinal->timer.setInterval(timeout * 1000);
    datFinal->timer.setSingleShot(true);

    if (timeout != 0) {
        datFinal->timeoutEnabled = true;
        cancel = g_cancellable_new();
        connect(&datFinal->timer, &QTimer::timeout, this, [this, datFinal, cancel] {
            g_cancellable_cancel(cancel);
            if (datFinal->blocker) {
                QMutexLocker locker(&lockForLoop);
                datFinal->blocker->quit();
            }
        });
        datFinal->timer.start();
    }

    g_mount_unmount_with_operation(mount, G_MOUNT_UNMOUNT_NONE, nullptr, cancel, unmountWithGvfsCb, datFinal);
}

void DProtocolDevicePrivate::unmountWithGvfsCb(GObject *obj, GAsyncResult *res, gpointer data)
{
    GMount *mount = reinterpret_cast<GMount *>(obj);
    DataFinalCallback *datFinal = static_cast<DataFinalCallback *>(data);
    Q_ASSERT(datFinal);

    GError_autoptr err { nullptr };
    bool ret = g_mount_unmount_with_operation_finish(mount, res, &err);   // TODO(xust): error handle

    if (datFinal->blocker && datFinal->d) {
        QMutexLocker locker(&datFinal->d->lockForLoop);
        datFinal->blocker->quit();
    }

    if (datFinal->finalCallback1)
        datFinal->finalCallback1(ret);

    if (datFinal->d) {
        g_object_unref(datFinal->d->mount);
        datFinal->d->mount = nullptr;
    }

    delete datFinal;
}

bool DProtocolDevicePrivate::isDaemonMountEnable()
{
    auto systemBusIFace = QDBusConnection::systemBus().interface();
    if (!systemBusIFace)
        return false;

    if (!systemBusIFace->isServiceRegistered(Utils::kDaemonService))
        return false;

    QDBusInterface daemonIface(Utils::kDaemonService, Utils::kDaemonPath, Utils::kDaemonIntro,
                               QDBusConnection::systemBus());
    QDBusReply<QString> reply = daemonIface.call(Utils::kDaemonIntroMethod);
    return reply.value().contains("<node name=\"MountControl\"/>");
}
