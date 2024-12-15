// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DPROTOCOLDEVICE_P_H
#define DPROTOCOLDEVICE_P_H

#include <DProtocolDevice>

#include <gio/gio.h>

#include <QObject>
#include <QPointer>
#include <QEventLoop>
#include <QMutex>

DMOUNT_BEGIN_NAMESPACE

class DProtocolDevicePrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(DProtocolDevice)

    enum Attr {
        Total,
        Usage,
        Free,
        FileSystem
    };

public:
    explicit DProtocolDevicePrivate(const QString &devicePath, DProtocolDevice *qq);
    ~DProtocolDevicePrivate() override
    {
        if (mount)
            g_object_unref(mount);
        mount = nullptr;

        if (volume)
            g_object_unref(volume);
        volume = nullptr;
    }

    QVariant queryAttrs(Attr attr) const;

    void initVolume();
    void initMount();

    void mountNetworkDevice(OperateCallbackWithInfo callback, QEventLoop *blocker = nullptr);
    void mountNetworkWithDaemon(OperateCallbackWithInfo callback, QEventLoop *blocker = nullptr);

    void mountNetworkWithGvfs(OperateCallbackWithInfo callback, QEventLoop *blocker = nullptr);
    static void mountWithGvfsCb(GObject *obj, GAsyncResult *res, gpointer data);
    static void askQuestion(GMountOperation *op, const char *msg, const char **choices, void *data);
    static void askPasswd(GMountOperation *op, const char *msg, const char *defUser,
                          const char *defDomain, GAskPasswordFlags flgs, void *data);

    void mountWithVolume(OperateCallbackWithInfo callback, QEventLoop *blocker = nullptr);
    static void mountWithVolumeCb(GObject *obj, GAsyncResult *res, gpointer data);

    void unmountWithDaemon(const QString &mpt, OperateCallback callback, QEventLoop *blocker = nullptr);

    void unmountWithGvfs(OperateCallback callback, QEventLoop *blocker = nullptr);
    static void unmountWithGvfsCb(GObject *obj, GAsyncResult *res, gpointer data);

    // these functions is designed for daemon mount.
    static bool isDaemonMountEnable();

private:
    QString devicePath;
    int timeout { 3 };
    AskForPasswd askForPasswd { nullptr };
    AskForChoice askForChoice { nullptr };

    mutable QMap<Attr, QVariant> attrs;
    mutable QStringList icons;

    QMutex lockForLoop;

    GMount *mount { nullptr };
    GVolume *volume { nullptr };

    DProtocolDevice *q_ptr { nullptr };
};

DMOUNT_END_NAMESPACE

#endif   // DPROTOCOLDEVICE_P_H
