/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <kwinglobals.h>

#include <QObject>
#include <QPointer>
#include <QSet>

class QThread;
class QProcess;
class QWindow;

namespace KWaylandServer
{
class AppMenuManagerInterface;
class ClientConnection;
class CompositorInterface;
class Display;
class DataDeviceInterface;
class IdleInterface;
class InputMethodV1Interface;
class SeatInterface;
class DataDeviceManagerInterface;
class ServerSideDecorationManagerInterface;
class ServerSideDecorationPaletteManagerInterface;
class SurfaceInterface;
class OutputInterface;
class PlasmaShellInterface;
class PlasmaWindowActivationFeedbackInterface;
class PlasmaVirtualDesktopManagementInterface;
class PlasmaWindowManagementInterface;
class OutputDeviceV2Interface;
class OutputDeviceInterface;
class OutputManagementV2Interface;
class OutputManagementInterface;
class XdgForeignV2Interface;
class XdgOutputManagerV1Interface;
class LinuxDmaBufV1ClientBufferIntegration;
class LinuxDmaBufV1ClientBuffer;
class TabletManagerV2Interface;
class KeyboardShortcutsInhibitManagerV1Interface;
class XdgDecorationManagerV1Interface;
class XWaylandKeyboardGrabManagerV1Interface;
class XWaylandKeyboardGrabV1Interface;
class ContentTypeManagerV1Interface;
class DrmLeaseManagerV1;
class TearingControlManagerV1Interface;
class XwaylandShellV1Interface;
class OutputOrderV1Interface;
class ClientManagementInterface;
class OCEANShellInterface;
class OCEANSeatInterface;
class OCEANKvmInterface;
class OCEANRestrictInterface;
class OCEANSecurityInterface;
class StrutInterface;
}

namespace KWin
{

class Window;
class Output;
class XdgActivationV1Integration;
class XdgPopupWindow;
class XdgSurfaceWindow;
class XdgToplevelWindow;
class XdgPopupV6Window;
class XdgSurfaceV6Window;
class XdgToplevelV6Window;

class KWIN_EXPORT WaylandServer : public QObject
{
    Q_OBJECT

public:
    enum class InitializationFlag {
        NoOptions = 0x0,
        LockScreen = 0x1,
        NoLockScreenIntegration = 0x2,
        NoGlobalShortcuts = 0x4,
        NoVtSwitch = 0x8
    };

    Q_DECLARE_FLAGS(InitializationFlags, InitializationFlag)

    ~WaylandServer() override;
    bool init(const QString &socketName, InitializationFlags flags = InitializationFlag::NoOptions);
    bool init(InitializationFlags flags = InitializationFlag::NoOptions);

    bool start();

    KWaylandServer::Display *display() const
    {
        return m_display;
    }
    KWaylandServer::CompositorInterface *compositor() const
    {
        return m_compositor;
    }
    KWaylandServer::SeatInterface *seat() const
    {
        return m_seat;
    }
    KWaylandServer::TabletManagerV2Interface *tabletManagerV2() const
    {
        return m_tabletManagerV2;
    }
    KWaylandServer::DataDeviceManagerInterface *dataDeviceManager() const
    {
        return m_dataDeviceManager;
    }
    KWaylandServer::PlasmaWindowActivationFeedbackInterface *plasmaActivationFeedback() const
    {
        return m_plasmaActivationFeedback;
    }
    KWaylandServer::PlasmaVirtualDesktopManagementInterface *virtualDesktopManagement() const
    {
        return m_virtualDesktopManagement;
    }
    KWaylandServer::PlasmaWindowManagementInterface *windowManagement() const
    {
        return m_windowManagement;
    }
    KWaylandServer::ServerSideDecorationManagerInterface *decorationManager() const
    {
        return m_decorationManager;
    }
    KWaylandServer::XdgOutputManagerV1Interface *xdgOutputManagerV1() const
    {
        return m_xdgOutputManagerV1;
    }
    KWaylandServer::KeyboardShortcutsInhibitManagerV1Interface *keyboardShortcutsInhibitManager() const
    {
        return m_keyboardShortcutsInhibitManager;
    }
    KWaylandServer::XwaylandShellV1Interface *xwaylandShell() const
    {
        return m_xwaylandShell;
    }
    KWaylandServer::ClientManagementInterface *clientManagement() const {
        return m_clientManagement;
    }
    KWaylandServer::OCEANShellInterface *oceanShell() const {
        return m_oceanShell;
    }
    KWaylandServer::OCEANSecurityInterface *oceanSecurity() const {
        return m_oceanSecurity;
    }
    KWaylandServer::StrutInterface *strut() const {
        return m_strut;
    }
    KWaylandServer::OCEANSeatInterface *oceanSeat() const {
        return m_oceanSeat;
    }
    KWaylandServer::OCEANKvmInterface *oceanKvm() const {
        return m_oceanKvm;
    }
    KWaylandServer::OCEANRestrictInterface *oceanRestrict() const {
        return m_oceanRestrict;
    }
    KWaylandServer::XWaylandKeyboardGrabV1Interface *XWaylandKeyboardGrabClientV1() const {
        return m_grabClient;
    }

    bool isKeyboardShortcutsInhibited() const;

    KWaylandServer::LinuxDmaBufV1ClientBufferIntegration *linuxDmabuf();

    KWaylandServer::InputMethodV1Interface *inputMethod() const
    {
        return m_inputMethod;
    }
    KWaylandServer::IdleInterface *idle() const
    {
        return m_idle;
    }
    QList<Window *> windows() const
    {
        return m_windows;
    }
    void removeWindow(Window *c);
    Window *findWindow(quint32 id) const;
    Window *findWindow(const KWaylandServer::SurfaceInterface *surface) const;
    XdgToplevelWindow *findXdgToplevelWindow(KWaylandServer::SurfaceInterface *surface) const;
    XdgSurfaceWindow *findXdgSurfaceWindow(KWaylandServer::SurfaceInterface *surface) const;
    XdgToplevelV6Window *findXdgToplevelV6Window(KWaylandServer::SurfaceInterface *surface) const;
    XdgSurfaceV6Window *findXdgSurfaceV6Window(KWaylandServer::SurfaceInterface *surface) const;
    Window *findClient(const QByteArray &resource) const;

    /**
     * @returns a transient parent of a surface imported with the foreign protocol, if any
     */
    KWaylandServer::SurfaceInterface *findForeignTransientForSurface(KWaylandServer::SurfaceInterface *surface);

    /**
     * @returns file descriptor for Xwayland to connect to.
     */
    int createXWaylandConnection();
    void destroyXWaylandConnection();

    /**
     * @returns file descriptor to the input method server's socket.
     */
    int createInputMethodConnection();
    void destroyInputMethodConnection();

    /**
     * @returns true if screen is locked.
     */
    bool isScreenLocked() const;
    /**
     * @returns whether integration with KScreenLocker is available.
     */
    bool hasScreenLockerIntegration() const;

    /**
     * @returns whether any kind of global shortcuts are supported.
     */
    bool hasGlobalShortcutSupport() const;

    /**
     * @returns whether vt switch global shortcuts are supported.
     **/
    bool hasVtSwitchSupport() const;

     /**
     * @return whether there are clients that prohibits screenshots
     */
    bool hasProhibitWindows() const;

    void initWorkspace();

    KWaylandServer::ClientConnection *xWaylandConnection() const;
    KWaylandServer::ClientConnection *inputMethodConnection() const;
    KWaylandServer::ClientConnection *screenLockerClientConnection() const
    {
        return m_screenLockerClientConnection;
    }

    /**
     * Struct containing information for a created Wayland connection through a
     * socketpair.
     */
    struct SocketPairConnection
    {
        /**
         * ServerSide Connection
         */
        KWaylandServer::ClientConnection *connection = nullptr;
        /**
         * client-side file descriptor for the socket
         */
        int fd = -1;
    };
    /**
     * Creates a Wayland connection using a socket pair.
     */
    SocketPairConnection createConnection();

    QSet<KWaylandServer::LinuxDmaBufV1ClientBuffer *> linuxDmabufBuffers() const
    {
        return m_linuxDmabufBuffers;
    }
    void addLinuxDmabufBuffer(KWaylandServer::LinuxDmaBufV1ClientBuffer *buffer)
    {
        m_linuxDmabufBuffers << buffer;
    }
    void removeLinuxDmabufBuffer(KWaylandServer::LinuxDmaBufV1ClientBuffer *buffer)
    {
        m_linuxDmabufBuffers.remove(buffer);
    }

    KWaylandServer::OutputInterface *findWaylandOutput(Output * output) const;

    /**
     * Returns the first socket name that can be used to connect to this server.
     * For a full list, use display()->socketNames()
     */
    QString socketName() const;

    XdgActivationV1Integration *xdgActivationIntegration() const
    {
        return m_xdgActivationIntegration;
    }

    quint32 createWindowId(KWaylandServer::SurfaceInterface *surface);

Q_SIGNALS:
    void windowAoceand(KWin::Window *);
    void windowRemoved(KWin::Window *);
    void initialized();
    void foreignTransientChanged(KWaylandServer::SurfaceInterface *child);
    void lockStateChanged();

private:
    int createScreenLockerConnection();
    void windowShown(Window *t);
    void initScreenLocker();
    void registerXdgGenericWindow(Window *window);
    void registerXdgToplevelWindow(XdgToplevelWindow *window);
    void registerXdgPopupWindow(XdgPopupWindow *window);
    void registerXdgGenericV6Window(Window *window);
    void registerXdgToplevelV6Window(XdgToplevelV6Window *window);
    void registerXdgPopupV6Window(XdgPopupV6Window *window);
    void registerWindow(Window *window);
    void handleOutputAoceand(Output *output);
    void handleOutputRemoved(Output *output);
    void handleOutputEnabled(Output *output);
    void handleOutputDisabled(Output *output);
    quint16 createClientId(KWaylandServer::ClientConnection *c);

    class LockScreenPresentationWatcher : public QObject
    {
    public:
        LockScreenPresentationWatcher(WaylandServer *server);

    private:
        QSet<Output *> m_signaledOutputs;
    };

    KWaylandServer::Display *m_display = nullptr;
    KWaylandServer::CompositorInterface *m_compositor = nullptr;
    KWaylandServer::SeatInterface *m_seat = nullptr;
    KWaylandServer::TabletManagerV2Interface *m_tabletManagerV2 = nullptr;
    KWaylandServer::DataDeviceManagerInterface *m_dataDeviceManager = nullptr;
    KWaylandServer::PlasmaShellInterface *m_plasmaShell = nullptr;
    KWaylandServer::PlasmaWindowActivationFeedbackInterface *m_plasmaActivationFeedback = nullptr;
    KWaylandServer::PlasmaWindowManagementInterface *m_windowManagement = nullptr;
    KWaylandServer::PlasmaVirtualDesktopManagementInterface *m_virtualDesktopManagement = nullptr;
    KWaylandServer::ServerSideDecorationManagerInterface *m_decorationManager = nullptr;
    KWaylandServer::OutputManagementV2Interface *m_outputManagement = nullptr;
    KWaylandServer::OutputManagementInterface *m_outputV1Management = nullptr;
    KWaylandServer::AppMenuManagerInterface *m_appMenuManager = nullptr;
    KWaylandServer::ServerSideDecorationPaletteManagerInterface *m_paletteManager = nullptr;
    KWaylandServer::IdleInterface *m_idle = nullptr;
    KWaylandServer::XdgOutputManagerV1Interface *m_xdgOutputManagerV1 = nullptr;
    KWaylandServer::XdgDecorationManagerV1Interface *m_xdgDecorationManagerV1 = nullptr;
    KWaylandServer::LinuxDmaBufV1ClientBufferIntegration *m_linuxDmabuf = nullptr;
    KWaylandServer::KeyboardShortcutsInhibitManagerV1Interface *m_keyboardShortcutsInhibitManager = nullptr;
    QSet<KWaylandServer::LinuxDmaBufV1ClientBuffer *> m_linuxDmabufBuffers;
    QPointer<KWaylandServer::ClientConnection> m_xwaylandConnection;
    KWaylandServer::InputMethodV1Interface *m_inputMethod = nullptr;
    QPointer<KWaylandServer::ClientConnection> m_inputMethodServerConnection;
    KWaylandServer::ClientConnection *m_screenLockerClientConnection = nullptr;
    KWaylandServer::XdgForeignV2Interface *m_XdgForeign = nullptr;
    XdgActivationV1Integration *m_xdgActivationIntegration = nullptr;
    KWaylandServer::XWaylandKeyboardGrabManagerV1Interface *m_xWaylandKeyboardGrabManager = nullptr;
    KWaylandServer::XWaylandKeyboardGrabV1Interface *m_grabClient = nullptr;
    KWaylandServer::ContentTypeManagerV1Interface *m_contentTypeManager = nullptr;
    KWaylandServer::TearingControlManagerV1Interface *m_tearingControlInterface = nullptr;
    KWaylandServer::XwaylandShellV1Interface *m_xwaylandShell = nullptr;
    KWaylandServer::ClientManagementInterface *m_clientManagement = nullptr;
    KWaylandServer::OCEANShellInterface *m_oceanShell = nullptr;
    KWaylandServer::OCEANSeatInterface *m_oceanSeat = nullptr;
    KWaylandServer::OCEANKvmInterface *m_oceanKvm = nullptr;
    KWaylandServer::OCEANRestrictInterface *m_oceanRestrict = nullptr;
    KWaylandServer::OCEANSecurityInterface *m_oceanSecurity = nullptr;
    KWaylandServer::StrutInterface *m_strut = nullptr;
    QList<Window *> m_windows;
    InitializationFlags m_initFlags;
    QHash<KWaylandServer::ClientConnection*, quint16> m_clientIds;
    QHash<Output *, KWaylandServer::OutputInterface *> m_waylandOutputs;
    QHash<Output *, KWaylandServer::OutputDeviceV2Interface *> m_waylandOutputDevices;
    QHash<Output *, KWaylandServer::OutputDeviceInterface *> m_waylandOutputV1Devices;
    KWaylandServer::DrmLeaseManagerV1 *m_leaseManager = nullptr;
    KWaylandServer::OutputOrderV1Interface *m_outputOrder = nullptr;
    KWIN_SINGLETON(WaylandServer)
};

inline WaylandServer *waylandServer()
{
    return WaylandServer::self();
}

} // namespace KWin
