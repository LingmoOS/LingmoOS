/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2024 David Redondo <kde@david-redondo.de>
*/

#ifndef XDG_DESKTOP_PORTAL_KDE_INPUTCAPTURE_H
#define XDG_DESKTOP_PORTAL_KDE_INPUTCAPTURE_H

#include <QDBusAbstractAdaptor>
#include <QDBusContext>
#include <QDBusObjectPath>
#include <QDBusUnixFileDescriptor>

class InputCapturePortal : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.InputCapture")
    Q_PROPERTY(uint version READ version CONSTANT)
    Q_PROPERTY(uint SupportedCapabilities READ SupportedCapabilities CONSTANT)
public:
    explicit InputCapturePortal(QObject *parent);

    enum Capability : uint {
        None = 0x0,
        Keyboard = 0x1,
        Pointer = 0x2,
        TouchScreen = 0x4,
        All = (Keyboard | Pointer | TouchScreen),
    };
    Q_DECLARE_FLAGS(Capabilities, Capability)

    enum class State { Disabled, Deactivated, Activated };

    uint version() const
    {
        return 1;
    }
    uint SupportedCapabilities() const
    {
        return All;
    };

    struct zone {
        uint width;
        uint height;
        int x_offset;
        int y_offset;
    };

public Q_SLOTS:
    uint CreateSession(const QDBusObjectPath &handle,
                       const QDBusObjectPath &session_handle,
                       const QString &app_id,
                       const QString &parent_window,
                       const QVariantMap &options,
                       QVariantMap &results);
    uint
    GetZones(const QDBusObjectPath &handle, const QDBusObjectPath &session_handle, const QString &app_id, const QVariantMap &options, QVariantMap &results);
    uint SetPointerBarriers(const QDBusObjectPath &handle,
                            const QDBusObjectPath &session_handle,
                            const QString &app_id,
                            const QVariantMap &options,
                            const QList<QVariantMap> &barriers,
                            uint zone_set,
                            QVariantMap &results);

    uint Enable(const QDBusObjectPath &session_handle, const QString &app_id, const QVariantMap &options, QVariantMap &results);
    uint Disable(const QDBusObjectPath &session_handle, const QString &app_id, const QVariantMap &options, QVariantMap &results);

    uint Release(const QDBusObjectPath &session_handle, const QString &app_id, const QVariantMap &options, QVariantMap &results);
    QDBusUnixFileDescriptor ConnectToEIS(const QDBusObjectPath &session_handle, const QString &app_id, const QVariantMap &options, const QDBusMessage &message);

Q_SIGNALS:
    void Disabled(const QDBusObjectPath &session_handle, const QVariantMap &options);
    void Activated(const QDBusObjectPath &session_handle, const QVariantMap &options);
    void Deactivated(const QDBusObjectPath &session_handle, const QVariantMap &options);
    void ZonesChanged(const QDBusObjectPath &session_handle, const QVariantMap &options);

private:
    uint m_zoneId = 0;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(InputCapturePortal::Capabilities)

#endif
