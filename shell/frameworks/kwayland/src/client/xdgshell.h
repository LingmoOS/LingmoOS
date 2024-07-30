/*
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef KWAYLAND_CLIENT_XDG_SHELL_H
#define KWAYLAND_CLIENT_XDG_SHELL_H

#include "KWayland/Client/kwaylandclient_export.h"
#include <QObject>
#include <QRect>
#include <QSize>

// This is a mix of structs for both xdgshell unstable v5 AND xdg wm base stable
struct xdg_wm_base;
struct xdg_shell;
struct xdg_surface;
struct xdg_popup;
struct xdg_toplevel;

struct zxdg_shell_v6;
struct zxdg_toplevel_v6;
struct zxdg_surface_v6;
struct zxdg_popup_v6;
struct zxdg_position_v6;

namespace KWayland
{
namespace Client
{
class EventQueue;
class Output;
class Surface;
class Seat;
class XdgShellPopup;
class XdgShellSurface;

/**
 * Builder class describing how a popup should be positioned
 * when created
 *
 * @since 5.39
 */
class KWAYLANDCLIENT_EXPORT XdgPositioner
{
public:
    /*
     * Flags describing how a popup should be reposition if constrained
     */
    enum class Constraint {
        /*
         * Slide the popup on the X axis until there is room
         */
        SlideX = 1 << 0,
        /*
         * Slide the popup on the Y axis until there is room
         */
        SlideY = 1 << 1,
        /*
         * Invert the anchor and gravity on the X axis
         */
        FlipX = 1 << 2,
        /*
         * Invert the anchor and gravity on the Y axis
         */
        FlipY = 1 << 3,
        /*
         * Resize the popup in the X axis
         */
        ResizeX = 1 << 4,
        /*
         * Resize the popup in the Y axis
         */
        ResizeY = 1 << 5,
    };

    Q_DECLARE_FLAGS(Constraints, Constraint)

    XdgPositioner(const QSize &initialSize = QSize(), const QRect &anchor = QRect());
    XdgPositioner(const XdgPositioner &other);
    ~XdgPositioner();

    /**
     * Which edge of the anchor should the popup be positioned around
     */
    // KF6 TODO use a better data type (enum of 8 options) rather than flags which allow invalid values
    Qt::Edges anchorEdge() const;
    void setAnchorEdge(Qt::Edges edge);

    /**
     * Specifies in what direction the popup should be positioned around the anchor
     * i.e if the gravity is "bottom", then then the top of top of the popup will be at the anchor edge
     * if the gravity is top, then the bottom of the popup will be at the anchor edge
     *
     */
    // KF6 TODO use a better data type (enum of 8 options) rather than flags which allow invalid values
    Qt::Edges gravity() const;
    void setGravity(Qt::Edges edge);

    /**
     * The area this popup should be positioned around
     */
    QRect anchorRect() const;
    void setAnchorRect(const QRect &anchor);

    /**
     * The size of the surface that is to be positioned.
     */
    QSize initialSize() const;
    void setInitialSize(const QSize &size);

    /**
     * Specifies how the compositor should position the popup if it does not fit in the requested position
     */
    Constraints constraints() const;
    void setConstraints(Constraints constraints);

    /**
     * An additional offset that should be applied from the anchor.
     */
    QPoint anchorOffset() const;
    void setAnchorOffset(const QPoint &offset);

private:
    class Private;
    QScopedPointer<Private> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(XdgPositioner::Constraints)

/**
 * @short Wrapper for the xdg_shell interface.
 *
 * This class provides a convenient wrapper for the xdg_shell interface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the XdgShell interface:
 * @code
 * XdgShell *c = registry->createXdgShell(name, version);
 * @endcode
 *
 * This creates the XdgShell and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * XdgShell *c = new XdgShell;
 * c->setup(registry->bindXdgShell(name, version));
 * @endcode
 *
 * The XdgShell can be used as a drop-in replacement for any xdg_shell
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 * @since 5.25
 **/
class KWAYLANDCLIENT_EXPORT XdgShell : public QObject
{
    Q_OBJECT
public:
    ~XdgShell() override;

    /**
     * Setup this XdgShell to manage the @p xdgshellv5.
     * When using Registry::createXdgShell there is no need to call this
     * method.
     **/
    void setup(xdg_shell *xdgshellv5);

    /**
     * Setup this XdgShell to manage the @p xdgshellv6.
     * When using Registry::createXdgShell there is no need to call this
     * method.
     **/
    void setup(zxdg_shell_v6 *xdgshellv6);

    /**
     * Setup this XdgShell to manage the @p xdg_wm_base.
     * When using Registry::createXdgShell there is no need to call this
     * method.
     **/
    void setup(xdg_wm_base *xdg_wm_base);
    /**
     * @returns @c true if managing a xdg_shell.
     **/
    bool isValid() const;
    /**
     * Releases the xdg_shell interface.
     * After the interface has been released the XdgShell instance is no
     * longer valid and can be setup with another xdg_shell interface.
     **/
    void release();
    /**
     * Destroys the data held by this XdgShell.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new xdg_shell interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * XdgShell gets destroyed.
     *
     * @see release
     **/
    void destroy();

    /**
     * Sets the @p queue to use for creating objects with this XdgShell.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating objects with this XdgShell.
     **/
    EventQueue *eventQueue();

    /**
     * Creates a new XdgShellSurface for the given @p surface.
     **/
    XdgShellSurface *createSurface(Surface *surface, QObject *parent = nullptr);

    /**
     * Creates a new XdgShellPopup for the given @p surface on top of @p parentSurface.
     * This method is only valid for Xdgv5
     **/
    XdgShellPopup *createPopup(Surface *surface, Surface *parentSurface, Seat *seat, quint32 serial, const QPoint &parentPos, QObject *parent = nullptr);

    /**
     * Creates a new XdgShellPopup for the given @p surface on top of @p parentSurface with the given @p positioner.
     * This method is only valid for Xdgv6 onwards.
     * @since 5.39
     **/
    XdgShellPopup *createPopup(Surface *surface, XdgShellSurface *parentSurface, const XdgPositioner &positioner, QObject *parent = nullptr);

    /**
     * Creates a new XdgShellPopup for the given @p surface on top of @p parentSurface with the given @p positioner.
     * @since 5.39
     **/
    XdgShellPopup *createPopup(Surface *surface, XdgShellPopup *parentSurface, const XdgPositioner &positioner, QObject *parent = nullptr);

    operator xdg_wm_base *();
    operator xdg_wm_base *() const;
    operator xdg_shell *();
    operator xdg_shell *() const;
    operator zxdg_shell_v6 *();
    operator zxdg_shell_v6 *() const;

Q_SIGNALS:
    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the XdgShell got created by
     * Registry::createXdgShell
     **/
    void removed();

protected:
    /**
     * Creates a new XdgShell.
     * Note: after constructing the XdgShell it is not yet valid and one needs
     * to call setup. In order to get a ready to use XdgShell prefer using
     * Registry::createXdgShell.
     **/
    class Private;
    explicit XdgShell(Private *p, QObject *parent = nullptr);

private:
    QScopedPointer<Private> d;
};

/**
 *
 * @since 5.25
 **/
class KWAYLANDCLIENT_EXPORT XdgShellSurface : public QObject
{
    Q_OBJECT
public:
    ~XdgShellSurface() override;
    /**
     * States the Surface can be in
     **/
    enum class State {
        /**
         * The Surface is maximized.
         **/
        Maximized = 1 << 0,
        /**
         * The Surface is fullscreen.
         **/
        Fullscreen = 1 << 1,
        /**
         * The Surface is currently being resized by the Compositor.
         **/
        Resizing = 1 << 2,
        /**
         * The Surface is considered active. Does not imply keyboard focus.
         **/
        Activated = 1 << 3
    };
    Q_DECLARE_FLAGS(States, State)

    /**
     * Setup this XdgShellSurface to manage the @p xdgsurfacev5.
     * When using XdgShell::createXdgShellSurface there is no need to call this
     * method.
     **/
    void setup(xdg_surface *xdgsurfacev5);

    /**
     * Setup this XdgShellSurface to manage the @p toplevel on the relevant @p xdgsurfacev6
     * When using XdgShell::createXdgShellSurface there is no need to call this
     * method.
     **/
    void setup(zxdg_surface_v6 *xdgsurfacev6, zxdg_toplevel_v6 *toplevel);

    /**
     * Setup this XdgShellSurface to manage the @p toplevel on the relevant @p xdgsurface
     * When using XdgShell::createXdgShellSurface there is no need to call this
     * method.
     **/
    void setup(xdg_surface *xdgsurface, xdg_toplevel *toplevel);

    /**
     * @returns @c true if managing a xdg_surface.
     **/
    bool isValid() const;
    /**
     * Releases the xdg_surface interface.
     * After the interface has been released the XdgShellSurface instance is no
     * longer valid and can be setup with another xdg_surface interface.
     **/
    void release();
    /**
     * Destroys the data held by this XdgShellSurface.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new xdg_surface interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, xdgsurfacev5, &XdgShellSurface::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();
    /**
     * Sets the @p queue to use for bound proxies.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for bound proxies.
     **/
    EventQueue *eventQueue();

    /**
     * The currently configured size.
     * @see sizeChanged
     * @see setSize
     **/
    QSize size() const;

    /**
     * Sets the size for the XdgShellSurface to @p size.
     * This is mostly an internal information. The actual size of the XdgShellSurface is
     * determined by the size of the Buffer attached to the XdgShellSurface's Surface.
     *
     * @param size The new size to be used for the XdgShellSurface
     * @see size
     * @see sizeChanged
     **/
    void setSize(const QSize &size);

    /**
     * Set this XdgShellSurface as transient for @p parent.
     **/
    void setTransientFor(XdgShellSurface *parent);

    /**
     * Sets the window title of this XdgShellSurface to @p title.
     **/
    void setTitle(const QString &title);

    /**
     * Set an application identifier for the surface.
     **/
    void setAppId(const QByteArray &appId);

    /**
     * Requests to show the window menu at @p pos in surface coordinates.
     **/
    void requestShowWindowMenu(Seat *seat, quint32 serial, const QPoint &pos);

    /**
     * Requests a move on the given @p seat after the pointer button press with the given @p serial.
     *
     * @param seat The seat on which to move the window
     * @param serial The serial of the pointer button press which should trigger the move
     **/
    void requestMove(Seat *seat, quint32 serial);

    /**
     * Requests a resize on the given @p seat after the pointer button press with the given @p serial.
     *
     * @param seat The seat on which to resize the window
     * @param serial The serial of the pointer button press which should trigger the resize
     * @param edges A hint for the compositor to set e.g. an appropriate cursor image
     **/
    void requestResize(Seat *seat, quint32 serial, Qt::Edges edges);

    /**
     * When a configure event is received, if a client commits the
     * Surface in response to the configure event, then the client
     * must make an ackConfigure request sometime before the commit
     * request, passing along the @p serial of the configure event.
     * @see configureRequested
     **/
    void ackConfigure(quint32 serial);

    /**
     * Request to set this XdgShellSurface to be maximized if @p set is @c true.
     * If @p set is @c false it requests to unset the maximized state - if set.
     *
     * @param set Whether the XdgShellSurface should be maximized
     **/
    void setMaximized(bool set);

    /**
     * Request to set this XdgShellSurface as fullscreen on @p output.
     * If @p set is @c true the Surface should be set to fullscreen, otherwise restore
     * from fullscreen state.
     *
     * @param set Whether the Surface should be fullscreen or not
     * @param output Optional output as hint to the compositor where the Surface should be put
     **/
    void setFullscreen(bool set, Output *output = nullptr);

    /**
     * Request to the compositor to minimize this XdgShellSurface.
     **/
    void requestMinimize();

    /**
     * Set this surface to have a given maximum size
     * @since 5.39
     */
    void setMaxSize(const QSize &size);

    /**
     * Set this surface to have a given minimum size
     * @since 5.39
     */
    void setMinSize(const QSize &size);

    /**
     * Sets the position of the window contents within the buffer
     * @since 5.59
     */
    void setWindowGeometry(const QRect &windowGeometry);

    operator xdg_surface *();
    operator xdg_surface *() const;
    operator xdg_toplevel *();
    operator xdg_toplevel *() const;
    operator zxdg_surface_v6 *();
    operator zxdg_surface_v6 *() const;
    operator zxdg_toplevel_v6 *();
    operator zxdg_toplevel_v6 *() const;

Q_SIGNALS:
    /**
     * The compositor requested to close this window.
     **/
    void closeRequested();
    /**
     * The compositor sent a configure with the new @p size and the @p states.
     * Before the next commit of the surface the @p serial needs to be passed to ackConfigure.
     **/
    void configureRequested(const QSize &size, KWayland::Client::XdgShellSurface::States states, quint32 serial);

    /**
     * Emitted whenever the size of the XdgShellSurface changes by e.g. receiving a configure request.
     *
     * @see configureRequested
     * @see size
     * @see setSize
     **/
    void sizeChanged(const QSize &);

protected:
    class Private;
    explicit XdgShellSurface(Private *p, QObject *parent = nullptr);

private:
    QScopedPointer<Private> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(XdgShellSurface::States)

/**
 * A XdgShellPopup is a short-lived, temporary surface that can be
 * used to implement menus. It takes an explicit grab on the surface
 * that will be dismissed when the user dismisses the popup. This can
 * be done by the user clicking outside the surface, using the keyboard,
 * or even locking the screen through closing the lid or a timeout.
 * @since 5.25
 **/
class KWAYLANDCLIENT_EXPORT XdgShellPopup : public QObject
{
    Q_OBJECT
public:
    ~XdgShellPopup() override;

    /**
     * Setup this XdgShellPopup to manage the @p xdgpopupv5.
     * When using XdgShell::createXdgShellPopup there is no need to call this
     * method.
     *
     * @deprecated Since 5.49. This was for XDGShellV5, this is now deprecated
     **/
    // KWAYLANDCLIENT_DEPRECATED_VERSION(5, 49, "Use XDGShell version >5") // TODO KF6 remove internal usage
    void setup(xdg_popup *xdgpopupv5);

    /**
     * Setup this XdgShellPopup to manage the @p xdgpopupv6 on associated @p xdgsurfacev6
     * When using XdgShell::createXdgShellPopup there is no need to call this
     * method.
     * @since 5.39
     **/
    void setup(zxdg_surface_v6 *xdgsurfacev6, zxdg_popup_v6 *xdgpopup6);

    /**
     * Setup this XdgShellPopup to manage the @p xdgpopupv on associated @p xdgsurface
     * When using XdgShell::createXdgShellPopup there is no need to call this
     * method.
     * @since 5.XDGSTABLE
     **/
    void setup(xdg_surface *xdgsurface, xdg_popup *xdgpopup);

    /**
     * @returns @c true if managing an xdg_popup.
     **/
    bool isValid() const;
    /**
     * Releases the xdg_popup interface.
     * After the interface has been released the XdgShellPopup instance is no
     * longer valid and can be setup with another xdg_popup interface.
     **/
    void release();
    /**
     * Destroys the data held by this XdgShellPopup.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new xdg_popup interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, xdgpopupv5, &XdgShellPopup::destroy);
     * @endcode
     *
     * @see release
     **/
    void destroy();
    /**
     * Sets the @p queue to use for bound proxies.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for bound proxies.
     **/
    EventQueue *eventQueue();

    /**
     * Requests a grab on this popup
     * @since 5.39
     */
    void requestGrab(Seat *seat, quint32 serial);

    /**
     * When a configure event is received, if a client commits the
     * Surface in response to the configure event, then the client
     * must make an ackConfigure request sometime before the commit
     * request, passing along the @p serial of the configure event.
     * @see configureRequested
     * @since 5.56
     **/
    void ackConfigure(quint32 serial);

    /**
     * Sets the position of the window contents within the buffer
     * @since 5.59
     */
    void setWindowGeometry(const QRect &windowGeometry);

    operator xdg_surface *();
    operator xdg_surface *() const;
    operator xdg_popup *();
    operator xdg_popup *() const;
    operator zxdg_surface_v6 *();
    operator zxdg_surface_v6 *() const;
    operator zxdg_popup_v6 *();
    operator zxdg_popup_v6 *() const;

Q_SIGNALS:
    /**
     * This signal is emitted when a XdgShellPopup is dismissed by the
     * compositor. The user should delete this instance at this point.
     **/
    void popupDone();

    /**
     * Emitted when the server has configured the popup with the final location of @p relativePosition
     * This is emitted for V6 surfaces only
     * @since 5.39
     **/
    void configureRequested(const QRect &relativePosition, quint32 serial);

protected:
    class Private;
    explicit XdgShellPopup(Private *p, QObject *parent = nullptr);

private:
    QScopedPointer<Private> d;
};

/**
 * Wrapper class for xdg_wm_base interface.
 *
 * @since 5.101
 */
class KWAYLANDCLIENT_EXPORT XdgShellStable : public XdgShell
{
    Q_OBJECT
public:
    explicit XdgShellStable(QObject *parent = nullptr);
    ~XdgShellStable() override;

private:
    class Private;
};

/**
 * Wrapper class for xdg_toplevel interface.
 *
 * @since 5.101
 */
class KWAYLANDCLIENT_EXPORT XdgTopLevelStable : public XdgShellSurface
{
    Q_OBJECT
public:
    explicit XdgTopLevelStable(QObject *parent = nullptr);
    ~XdgTopLevelStable() override;

private:
    friend class XdgShellStable;
    class Private;
};

/**
 * Wrapper class for xdg_popup interface.
 *
 * @since 5.101
 */
class KWAYLANDCLIENT_EXPORT XdgShellPopupStable : public XdgShellPopup
{
public:
    explicit XdgShellPopupStable(QObject *parent = nullptr);
    ~XdgShellPopupStable() override;

private:
    friend class XdgShellStable;
    class Private;
};

}
}

Q_DECLARE_METATYPE(KWayland::Client::XdgPositioner)
Q_DECLARE_METATYPE(KWayland::Client::XdgShellSurface::State)
Q_DECLARE_METATYPE(KWayland::Client::XdgShellSurface::States)
Q_DECLARE_METATYPE(KWayland::Client::XdgPositioner::Constraint)
Q_DECLARE_METATYPE(KWayland::Client::XdgPositioner::Constraints)

#endif
