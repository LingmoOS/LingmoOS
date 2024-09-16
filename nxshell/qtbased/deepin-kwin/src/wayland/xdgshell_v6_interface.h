/*
    SPDX-FileCopyrightText: 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include "deepin-kwin_export.h"

#include <QObject>
#include <QSharedDataPointer>
#include <memory>

struct wl_resource;

namespace KWaylandServer
{
class Display;
class OutputInterface;
class SeatInterface;
class SurfaceInterface;
class XdgShellV6InterfacePrivate;
class XdgSurfaceV6InterfacePrivate;
class XdgToplevelV6InterfacePrivate;
class XdgPopupV6InterfacePrivate;
class XdgPositionerV6Data;
class XdgToplevelV6Interface;
class XdgPopupV6Interface;
class XdgSurfaceV6Interface;

/**
 * The XdgShellV6Interface class represents an extension for destrop-style user interfaces.
 *
 * The XdgShellV6Interface class provides a way for a client to extend a regular Wayland surface
 * with functionality required to construct user interface elements, e.g. toplevel windows or
 * menus.
 *
 * XdgShellV6Interface corresponds to the WaylandInterface \c xdg_shell_v6.
 */
class KWIN_EXPORT XdgShellV6Interface : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructs an XdgShellV6Interface object with the given wayland display \a display.
     */
    XdgShellV6Interface(Display *display, QObject *parent = nullptr);
    /**
     * Destructs the XdgShellV6Interface object.
     */
    ~XdgShellV6Interface() override;

    /**
     * Returns the wayland display of the XdgShellV6Interface.
     */
    Display *display() const;

    /**
     * Sends a ping event to the client with the given xdg-surface \a surface. If the client
     * replies to the event within a reasonable amount of time, pongReceived signal will be
     * emitted.
     */
    quint32 ping(XdgSurfaceV6Interface *surface);

Q_SIGNALS:
    /**
     * This signal is emitted when a new XdgToplevelInterface object is created.
     */
    void toplevelCreated(XdgToplevelV6Interface *toplevel);

    /**
     * This signal is emitted when a new XdgPopupV6Interface object is created.
     */
    void popupCreated(XdgPopupV6Interface *popup);

    /**
     * This signal is emitted when the client has responded to a ping event with serial \a serial.
     */
    void pongReceived(quint32 serial);

    /**
     * @todo Drop this signal.
     *
     * This signal is emitted when the client has not responded to a ping event with serial
     * \a serial within a reasonable amount of time and the compositor gave up on it.
     */
    void pingTimeout(quint32 serial);

    /**
     * This signal is emitted when the client has not responded to a ping event with serial
     * \a serial within a reasonable amount of time.
     */
    void pingDelayed(quint32 serial);

private:
    std::unique_ptr<XdgShellV6InterfacePrivate> d;
    friend class XdgShellV6InterfacePrivate;
};

/**
 * The XdgSurfaceV6Interface class provides a base set of functionality required to construct
 * user interface elements.
 *
 * XdgSurfaceV6Interface corresponds to the Wayland interface \c xdg_surface.
 */
class KWIN_EXPORT XdgSurfaceV6Interface : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructs an XdgSurfaceV6Interface for the given \a shell and \a surface.
     */
    XdgSurfaceV6Interface(XdgShellV6Interface *shell, SurfaceInterface *surface, ::wl_resource *resource);
    /**
     * Destructs the XdgSurfaceV6Interface object.
     */
    ~XdgSurfaceV6Interface() override;

    /**
     * Returns the XdgToplevelInterface associated with this XdgSurfaceV6Interface.
     *
     * This method will return \c null if no xdg_toplevel object is associated with this surface.
     */
    XdgToplevelV6Interface *toplevel() const;

    /**
     * Returns the XdgPopupV6Interface associated with this XdgSurfaceV6Interface.
     *
     * This method will return \c null if no xdg_popup object is associated with this surface.
     */
    XdgPopupV6Interface *popup() const;

    /**
     * Returns the XdgShellV6Interface associated with this XdgSurfaceV6Interface.
     */
    XdgShellV6Interface *shell() const;

    /**
     * Returns the SurfaceInterface assigned to this XdgSurfaceV6Interface.
     */
    SurfaceInterface *surface() const;

    /**
     * Returns \c true if the surface has been configured; otherwise returns \c false.
     */
    bool isConfigured() const;

    /**
     * Returns the window geometry of the XdgSurfaceV6Interface.
     *
     * This method will return an invalid QRect if the window geometry is not set by the client.
     */
    QRect windowGeometry() const;

    /**
     * Returns the XdgSurfaceV6Interface for the specified wayland resource object \a resource.
     */
    static XdgSurfaceV6Interface *get(::wl_resource *resource);

Q_SIGNALS:
    /**
     * This signal is emitted when the xdg-surface is about to be destroyed.
     */
    void aboutToBeDestroyed();

    /**
     * This signal is emitted when a configure event with serial \a serial has been acknowledged.
     */
    void configureAcknowledged(quint32 serial);

    /**
     * This signal is emitted when the window geometry has been changed.
     */
    void windowGeometryChanged(const QRect &rect);

    /**
     * This signal is emitted when the surface has been unmapped and its state has been reset.
     */
    void resetOccurred();

private:
    std::unique_ptr<XdgSurfaceV6InterfacePrivate> d;
    friend class XdgSurfaceV6InterfacePrivate;
};

/**
 * The XdgToplevelInterface class represents a surface with window-like functionality such
 * as maximize, fullscreen, resizing, minimizing, etc.
 *
 * XdgToplevelInterface corresponds to the Wayland interface \c xdg_toplevel.
 */
class KWIN_EXPORT XdgToplevelV6Interface : public QObject
{
    Q_OBJECT

public:
    enum State {
        MaximizedHorizontal = 0x1,
        MaximizedVertical = 0x2,
        FullScreen = 0x4,
        Resizing = 0x8,
        Activated = 0x10,
        TiledLeft = 0x20,
        TiledTop = 0x40,
        TiledRight = 0x80,
        TiledBottom = 0x100,
        Maximized = MaximizedHorizontal | MaximizedVertical,
    };
    Q_DECLARE_FLAGS(States, State)

    enum class ResizeAnchor {
        None = 0,
        Top = 1,
        Bottom = 2,
        Left = 4,
        TopLeft = 5,
        BottomLeft = 6,
        Right = 8,
        TopRight = 9,
        BottomRight = 10,
    };
    Q_ENUM(ResizeAnchor)

    /**
     * Constructs an XdgToplevelInterface for the given xdg-surface \a surface.
     */
    XdgToplevelV6Interface(XdgSurfaceV6Interface *surface, ::wl_resource *resource);
    /**
     * Destructs the XdgToplevelInterface object.
     */
    ~XdgToplevelV6Interface() override;

    /**
     * Returns the XdgShellV6Interface for this XdgToplevelInterface.
     *
     * This is equivalent to xdgSurface()->shell().
     */
    XdgShellV6Interface *shell() const;

    /**
     * Returns the XdgSurfaceV6Interface associated with the XdgToplevelInterface.
     */
    XdgSurfaceV6Interface *xdgSurface() const;

    /**
     * Returns the SurfaceInterface associated with the XdgToplevelInterface.
     */
    SurfaceInterface *surface() const;

    /**
     * Returns the parent XdgToplevelInterface above which this toplevel is stacked.
     */
    XdgToplevelV6Interface *parentXdgToplevel() const;

    /**
     * Returns \c true if the toplevel has been configured; otherwise returns \c false.
     */
    bool isConfigured() const;

    /**
     * Returns the window title of the toplevel surface.
     */
    QString windowTitle() const;

    /**
     * Returns the window class of the toplevel surface.
     */
    QString windowClass() const;

    /**
     * Returns the minimum window geometry size of the toplevel surface.
     */
    QSize minimumSize() const;

    /**
     * Returns the maximum window geometry size of the toplevel surface.
     */
    QSize maximumSize() const;

    /**
     * Sends a configure event to the client. \a size specifies the new window geometry size. A size
     * of zero means the client should decide its own window dimensions.
     */
    quint32 sendConfigure(const QSize &size, const States &states);

    /**
     * Sends a close event to the client. The client may choose to ignore this request.
     */
    void sendClose();

    /**
     * Sends an event to the client specifying the maximum bounds for the surface size. Must be
     * called before sendConfigure().
     */
    void sendConfigureBounds(const QSize &size);

    /**
     * Returns the XdgToplevelInterface for the specified wayland resource object \a resource.
     */
    static XdgToplevelV6Interface *get(::wl_resource *resource);

Q_SIGNALS:
    /**
     * This signal is emitted when the xdg-toplevel is about to be destroyed.
     */
    void aboutToBeDestroyed();

    /**
     * This signal is emitted when the xdg-toplevel has commited the initial state and wants to
     * be configured. After initializing the toplevel, you must send a configure event.
     */
    void initializeRequested();

    /**
     * This signal is emitted when the toplevel has been unmapped and its state has been reset.
     */
    void resetOccurred();

    /**
     * This signal is emitted when the toplevel's title has been changed.
     */
    void windowTitleChanged(const QString &windowTitle);

    /**
     * This signal is emitted when the toplevel's application id has been changed.
     */
    void windowClassChanged(const QString &windowClass);

    /**
     * This signal is emitted when the toplevel has requested the window menu to be shown at
     * \a pos. The \a seat and the \a serial indicate the user action that triggerred the request.
     */
    void windowMenuRequested(KWaylandServer::SeatInterface *seat, const QPoint &pos, quint32 serial);

    /**
     * This signal is emitted when the toplevel's minimum size has been changed.
     */
    void minimumSizeChanged(const QSize &size);

    /**
     * This signal is emitted when the toplevel's maximum size has been changed.
     */
    void maximumSizeChanged(const QSize &size);

    /**
     * This signal is emitted when the toplevel wants to be interactively moved. The \a seat and
     * the \a serial indicate the user action in response to which this request has been issued.
     */
    void moveRequested(KWaylandServer::SeatInterface *seat, quint32 serial);

    /**
     * This signal is emitted when the toplevel wants to be interactively resized by dragging
     * the specified \a anchor. The \a seat and the \a serial indicate the user action
     * in response to which this request has been issued.
     */
    void resizeRequested(KWaylandServer::SeatInterface *seat, KWaylandServer::XdgToplevelV6Interface::ResizeAnchor anchor, quint32 serial);

    /**
     * This signal is emitted when the toplevel surface wants to become maximized.
     */
    void maximizeRequested();

    /**
     * This signal is emitted when the toplevel surface wants to become unmaximized.
     */
    void unmaximizeRequested();

    /**
     * This signal is emitted when the toplevel wants to be shown in the full screen mode.
     */
    void fullscreenRequested(KWaylandServer::OutputInterface *output);

    /**
     * This signal is emitted when the toplevel surface wants to leave the full screen mode.
     */
    void unfullscreenRequested();

    /**
     * This signal is emitted when the toplevel wants to be iconified.
     */
    void minimizeRequested();

    /**
     * This signal is emitted when the parent toplevel has changed.
     */
    void parentXdgToplevelChanged();

private:
    std::unique_ptr<XdgToplevelV6InterfacePrivate> d;
    friend class XdgToplevelV6InterfacePrivate;
};

/**
 * The XdgPositionerV6 class provides a collection of rules for the placement of a popup surface.
 *
 * XdgPositionerV6 corresponds to the Wayland interface \c xdg_positioner_v6.
 */
class KWIN_EXPORT XdgPositionerV6
{
public:
    /**
     * Constructs an incomplete XdgPositionerV6 object.
     */
    XdgPositionerV6();
    /**
     * Constructs a copy of the XdgPositionerV6 object.
     */
    XdgPositionerV6(const XdgPositionerV6 &other);
    /**
     * Destructs the XdgPositionerV6 object.
     */
    ~XdgPositionerV6();

    /**
     * Assigns the value of \a other to this XdgPositionerV6 object.
     */
    XdgPositionerV6 &operator=(const XdgPositionerV6 &other);

    /**
     * Returns \c true if the positioner object is complete; otherwise returns \c false.
     *
     * An xdg positioner considered complete if it has a valid size and a valid anchor rect.
     */
    bool isComplete() const;

    /**
     * Returns the set of orientations along which the compositor may slide the popup to ensure
     * that it is entirely inside the compositor's defined "work area."
     */
    Qt::Orientations slideConstraintAdjustments() const;

    /**
     * Returns the set of orientations along which the compositor may flip the popup to ensure
     * that it is entirely inside the compositor's defined "work area."
     */
    Qt::Orientations flipConstraintAdjustments() const;

    /**
     * Returns the set of orientations along which the compositor can resize the popup to ensure
     * that it is entirely inside the compositor's defined "work area."
     */
    Qt::Orientations resizeConstraintAdjustments() const;

    /**
     * Returns the set of edges on the anchor rectangle that the surface should be positioned
     * around.
     */
    Qt::Edges anchorEdges() const;

    /**
     * Returns the direction in which the surface should be positioned, relative to the anchor
     * point of the parent surface.
     */
    Qt::Edges gravityEdges() const;

    /**
     * Returns the window geometry size of the surface that is to be positioned.
     */
    QSize size() const;

    /**
     * Returns the anchor rectangle relative to the upper left corner of the window geometry of
     * the parent surface that the popup should be positioned around.
     */
    QRect anchorRect() const;

    /**
     * Returns the surface position offset relative to the position of the anchor on the anchor
     * rectangle and the anchor on the surface.
     */
    QPoint offset() const;

    /**
     * Returns whether the surface should respond to movements in its parent window.
     */
    bool isReactive() const;

    /**
     * Returns the parent size to use when positioning the popup.
     */
    QSize parentSize() const;

    /**
     * Returns the serial of the configure event for the parent window.
     */
    quint32 parentConfigure() const;

    /**
     * Returns the current state of the xdg positioner object identified by \a resource.
     */
    static XdgPositionerV6 get(::wl_resource *resource);

private:
    XdgPositionerV6(const QSharedDataPointer<XdgPositionerV6Data> &data);
    QSharedDataPointer<XdgPositionerV6Data> d;
};

/**
 * The XdgPopupV6Interface class represents a surface that can be used to implement context menus,
 * popovers and other similar short-lived user interface elements.
 *
 * XdgPopupV6Interface corresponds to the Wayland interface \c xdg_popup.
 */
class KWIN_EXPORT XdgPopupV6Interface : public QObject
{
    Q_OBJECT

public:
    XdgPopupV6Interface(XdgSurfaceV6Interface *surface, SurfaceInterface *parentSurface, const XdgPositionerV6 &positioner, ::wl_resource *resource);
    /**
     * Destructs the XdgPopupV6Interface object.
     */
    ~XdgPopupV6Interface() override;

    XdgShellV6Interface *shell() const;

    /**
     * Returns the parent surface for this popup surface. If the initial state hasn't been
     * committed yet, this function may return \c null.
     */
    SurfaceInterface *parentSurface() const;

    /**
     * Returns the XdgSurfaceV6Interface associated with the XdgPopupV6Interface.
     */
    XdgSurfaceV6Interface *xdgSurface() const;

    /**
     * Returns the SurfaceInterface associated with the XdgPopupV6Interface.
     */
    SurfaceInterface *surface() const;

    /**
     * Returns the XdgPositionerV6 assigned to this XdgPopupV6Interface.
     */
    XdgPositionerV6 positioner() const;

    /**
     * Returns \c true if the popup has been configured; otherwise returns \c false.
     */
    bool isConfigured() const;

    /**
     * Sends a configure event to the client and returns the serial number of the event.
     */
    quint32 sendConfigure(const QRect &rect);

    /**
     * Sends a popup done event to the client.
     */
    void sendPopupDone();

    /**
     * Sends a popup repositioned event to the client.
     */
    void sendRepositioned(quint32 token);

    /**
     * Returns the XdgPopupV6Interface for the specified wayland resource object \a resource.
     */
    static XdgPopupV6Interface *get(::wl_resource *resource);

Q_SIGNALS:
    /**
     * This signal is emitted when the xdg-popup is about to be destroyed.
     */
    void aboutToBeDestroyed();

    /**
     * This signal is emitted when the xdg-popup has commited the initial state and wants to
     * be configured. After initializing the popup, you must send a configure event.
     */
    void initializeRequested();
    void grabRequested(SeatInterface *seat, quint32 serial);
    void repositionRequested(quint32 token);

private:
    std::unique_ptr<XdgPopupV6InterfacePrivate> d;
    friend class XdgPopupV6InterfacePrivate;
};

} // namespace KWaylandServer

Q_DECLARE_OPERATORS_FOR_FLAGS(KWaylandServer::XdgToplevelV6Interface::States)
Q_DECLARE_METATYPE(KWaylandServer::XdgToplevelV6Interface::State)
Q_DECLARE_METATYPE(KWaylandServer::XdgToplevelV6Interface::States)
