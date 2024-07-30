/*
 * SPDX-FileCopyrightText: 2018 Red Hat Inc
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 *
 * SPDX-FileCopyrightText: 2018 Jan Grulich <jgrulich@redhat.com>
 */

#ifndef XDG_DESKTOP_PORTAL_KDE_SESSION_H
#define XDG_DESKTOP_PORTAL_KDE_SESSION_H

#include <QAction>
#include <QDBusPendingReply>
#include <QDBusVirtualObject>
#include <QObject>
#include <QShortcut>

#include "globalshortcuts.h"
#include "inputcapture.h"
#include "remotedesktop.h"
#include "screencast.h"
#include "waylandintegration.h"

class KStatusNotifierItem;
class KGlobalAccelInterface;
class KGlobalAccelComponentInterface;
class RemoteDesktopPortal;

class Session : public QDBusVirtualObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Session)
public:
    explicit Session(QObject *parent = nullptr, const QString &appId = QString(), const QString &path = QString());
    ~Session() override;

    enum SessionType {
        ScreenCast = 0,
        RemoteDesktop = 1,
        GlobalShortcuts = 2,
        InputCapture = 3,
    };

    bool handleMessage(const QDBusMessage &message, const QDBusConnection &connection) override;
    QString introspect(const QString &path) const override;

    bool close();
    virtual SessionType type() const = 0;

    static Session *createSession(QObject *parent, SessionType type, const QString &appId, const QString &path);
    static Session *getSession(const QString &sessionHandle);

    QString handle() const
    {
        return m_path;
    }

Q_SIGNALS:
    void closed();

protected:
    const QString m_appId;
    const QString m_path;
};

class ScreenCastSession : public Session
{
    Q_OBJECT
public:
    explicit ScreenCastSession(QObject *parent, const QString &appId, const QString &path, const QString &iconName);
    ~ScreenCastSession() override;

    void setOptions(const QVariantMap &options);

    ScreenCastPortal::CursorModes cursorMode() const;
    bool multipleSources() const;
    ScreenCastPortal::SourceType types() const;

    SessionType type() const override
    {
        return SessionType::ScreenCast;
    }

    void setRestoreData(const QVariant &restoreData)
    {
        m_restoreData = restoreData;
    }

    QVariant restoreData() const
    {
        return m_restoreData;
    }

    void setPersistMode(ScreenCastPortal::PersistMode persistMode);

    ScreenCastPortal::PersistMode persistMode() const
    {
        return m_persistMode;
    }

    WaylandIntegration::Streams streams() const
    {
        return m_streams;
    }
    void setStreams(const WaylandIntegration::Streams &streams);
    virtual void refreshDescription()
    {
    }

protected:
    void setDescription(const QString &description);
    KStatusNotifierItem *const m_item;

private:
    bool m_multipleSources = false;
    ScreenCastPortal::CursorModes m_cursorMode = ScreenCastPortal::Hidden;
    ScreenCastPortal::SourceType m_types = ScreenCastPortal::Any;
    ScreenCastPortal::PersistMode m_persistMode = ScreenCastPortal::NoPersist;
    QVariant m_restoreData;

    void streamClosed();

    WaylandIntegration::Streams m_streams;
    friend class RemoteDesktopPortal;
};

class RemoteDesktopSession : public ScreenCastSession
{
    Q_OBJECT
public:
    explicit RemoteDesktopSession(QObject *parent, const QString &appId, const QString &path);
    ~RemoteDesktopSession() override;

    void setOptions(const QVariantMap &options);

    RemoteDesktopPortal::DeviceTypes deviceTypes() const;
    void setDeviceTypes(RemoteDesktopPortal::DeviceTypes deviceTypes);

    bool screenSharingEnabled() const;
    void setScreenSharingEnabled(bool enabled);

    void acquireStreamingInput();
    void refreshDescription() override;

    void setEisCookie(int cookie);
    int eisCookie() const;

    SessionType type() const override
    {
        return SessionType::RemoteDesktop;
    }

private:
    bool m_screenSharingEnabled;
    RemoteDesktopPortal::DeviceTypes m_deviceTypes;
    bool m_acquired = false;
    int m_cookie = 0;
};

class GlobalShortcutsSession : public Session
{
    Q_OBJECT
public:
    explicit GlobalShortcutsSession(QObject *parent, const QString &appId, const QString &path);
    ~GlobalShortcutsSession() override;

    SessionType type() const override
    {
        return SessionType::GlobalShortcuts;
    }

    void setActions(const Shortcuts &shortcuts);
    void loadActions();

    QVariant shortcutDescriptionsVariant() const;
    Shortcuts shortcutDescriptions() const;
    KGlobalAccelComponentInterface *component() const
    {
        return m_component;
    }
    QString componentName() const
    {
        return m_appId + m_token;
    }

Q_SIGNALS:
    void shortcutsChanged();
    void shortcutActivated(const QString &shortcutName, qlonglong timestamp);
    void shortcutDeactivated(const QString &shortcutName, qlonglong timestamp);

private:
    const QString m_token;
    std::unordered_map<QString, std::unique_ptr<QAction>> m_shortcuts;
    KGlobalAccelInterface *const m_globalAccelInterface;
    KGlobalAccelComponentInterface *const m_component;
};

class InputCaptureSession : public Session
{
    Q_OBJECT
public:
    explicit InputCaptureSession(QObject *parent, const QString &appId, const QString &path);
    ~InputCaptureSession() override;

    SessionType type() const override
    {
        return SessionType::InputCapture;
    }

    InputCapturePortal::State state;

    void connect(const QDBusObjectPath &path);
    QDBusObjectPath kwinInputCapture() const;

    QDBusPendingReply<void> enable();
    QDBusPendingReply<void> disable();
    QDBusPendingReply<void> release(const QPointF &cusorPosition, bool applyPosition);

    QDBusPendingReply<QDBusUnixFileDescriptor> connectToEIS();

    void addBarrier(const QPair<QPoint, QPoint> &barriers);
    void clearBarriers();

Q_SIGNALS:
    void disabled();
    void activated(uint activationId, const QPointF &cursorPosition);
    void deactivated(uint activationId);

private:
    QDBusObjectPath m_kwinInputCapture;
    QList<QPair<QPoint, QPoint>> m_barriers;
};

#endif // XDG_DESKTOP_PORTAL_KDE_SESSION_H
