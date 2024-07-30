/*
 * SPDX-FileCopyrightText: 2018 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2018 Jan Grulich <jgrulich@redhat.com>
 */

#ifndef XDG_DESKTOP_PORTAL_KDE_REMOTEDESKTOP_H
#define XDG_DESKTOP_PORTAL_KDE_REMOTEDESKTOP_H

#include <QDBusAbstractAdaptor>
#include <QDBusObjectPath>
#include <QDBusUnixFileDescriptor>

class QDBusMessage;

class RemoteDesktopPortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.RemoteDesktop")
    Q_PROPERTY(uint version READ version CONSTANT)
    Q_PROPERTY(uint AvailableDeviceTypes READ AvailableDeviceTypes CONSTANT)
public:
    explicit RemoteDesktopPortal(QObject *parent);
    ~RemoteDesktopPortal() override;

    enum DeviceType {
        None = 0x0,
        Keyboard = 0x1,
        Pointer = 0x2,
        TouchScreen = 0x4,
        All = (Keyboard | Pointer | TouchScreen),
    };
    Q_DECLARE_FLAGS(DeviceTypes, DeviceType)

    uint version() const
    {
        return 2;
    }
    uint AvailableDeviceTypes() const
    {
        return All;
    };

public Q_SLOTS:
    uint CreateSession(const QDBusObjectPath &handle,
                       const QDBusObjectPath &session_handle,
                       const QString &app_id,
                       const QVariantMap &options,
                       QVariantMap &results);

    uint SelectDevices(const QDBusObjectPath &handle,
                       const QDBusObjectPath &session_handle,
                       const QString &app_id,
                       const QVariantMap &options,
                       QVariantMap &results);

    uint Start(const QDBusObjectPath &handle,
               const QDBusObjectPath &session_handle,
               const QString &app_id,
               const QString &parent_window,
               const QVariantMap &options,
               QVariantMap &results);

    void NotifyPointerMotion(const QDBusObjectPath &session_handle, const QVariantMap &options, double dx, double dy);

    void NotifyPointerMotionAbsolute(const QDBusObjectPath &session_handle, const QVariantMap &options, uint stream, double x, double y);

    void NotifyPointerButton(const QDBusObjectPath &session_handle, const QVariantMap &options, int button, uint state);

    void NotifyPointerAxis(const QDBusObjectPath &session_handle, const QVariantMap &options, double dx, double dy);

    void NotifyPointerAxisDiscrete(const QDBusObjectPath &session_handle, const QVariantMap &options, uint axis, int steps);

    void NotifyKeyboardKeycode(const QDBusObjectPath &session_handle, const QVariantMap &options, int keycode, uint state);

    void NotifyKeyboardKeysym(const QDBusObjectPath &session_handle, const QVariantMap &options, int keysym, uint state);

    void NotifyTouchDown(const QDBusObjectPath &session_handle, const QVariantMap &options, uint stream, uint slot, int x, int y);

    void NotifyTouchMotion(const QDBusObjectPath &session_handle, const QVariantMap &options, uint stream, uint slot, int x, int y);

    void NotifyTouchUp(const QDBusObjectPath &session_handle, const QVariantMap &options, uint slot);

    QDBusUnixFileDescriptor ConnectToEIS(const QDBusObjectPath &session_handle, const QString &app_id, const QVariantMap &options, const QDBusMessage &message);
};
Q_DECLARE_OPERATORS_FOR_FLAGS(RemoteDesktopPortal::DeviceTypes)

#endif // XDG_DESKTOP_PORTAL_KDE_REMOTEDESKTOP_H
