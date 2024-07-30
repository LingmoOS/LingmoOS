/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_SHELL_H
#define WAYLAND_SHELL_H

#include <QObject>
#include <QPoint>
#include <QSize>
#include <QWindow>

#include "KWayland/Client/kwaylandclient_export.h"

struct wl_surface;
struct wl_shell;
struct wl_shell_surface;

namespace KWayland
{
namespace Client
{
class EventQueue;
class ShellSurface;
class Output;
class Seat;
class Surface;

/**
 * @short Wrapper for the wl_shell interface.
 *
 * This class provides a convenient wrapper for the wl_shell interface.
 * It's main purpose is to create a ShellSurface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the Shell interface:
 * @code
 * Shell *s = registry->createShell(name, version);
 * @endcode
 *
 * This creates the Shell and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * Shell *s = new Shell;
 * s->setup(registry->bindShell(name, version));
 * @endcode
 *
 * The Shell can be used as a drop-in replacement for any wl_shell
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 * @see ShellSurface
 **/
class KWAYLANDCLIENT_EXPORT Shell : public QObject
{
    Q_OBJECT
public:
    explicit Shell(QObject *parent = nullptr);
    ~Shell() override;

    /**
     * @returns @c true if managing a wl_shell.
     **/
    bool isValid() const;
    /**
     * Releases the wl_shell interface.
     * After the interface has been released the Shell instance is no
     * longer valid and can be setup with another wl_shell interface.
     *
     * Right before the interface is released the signal interfaceAboutToBeReleased is emitted.
     * @see interfaceAboutToBeReleased
     **/
    void release();
    /**
     * Destroys the data held by this Shell.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. Once the connection becomes invalid, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new wl_shell interface
     * once there is a new connection available.
     *
     * It is suggested to connect this method to ConnectionThread::connectionDied:
     * @code
     * connect(connection, &ConnectionThread::connectionDied, shell, &Shell::destroy);
     * @endcode
     *
     * Right before the data is destroyed, the signal interfaceAboutToBeDestroyed is emitted.
     *
     * @see release
     * @see interfaceAboutToBeDestroyed
     **/
    void destroy();
    /**
     * Setup this Shell to manage the @p shell.
     * When using Registry::createShell there is no need to call this
     * method.
     **/
    void setup(wl_shell *shell);

    /**
     * Sets the @p queue to use for creating a Surface.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating a Surface.
     **/
    EventQueue *eventQueue();

    /**
     * Creates a ShellSurface for the given @p surface and sets it up.
     *
     * @param surface The native surface to create the ShellSurface for
     * @param parent The parent to use for the ShellSurface
     * @returns created ShellSurface
     **/
    ShellSurface *createSurface(wl_surface *surface, QObject *parent = nullptr);
    /**
     * Creates a ShellSurface for the given @p surface and sets it up.
     *
     * @param surface The Surface to create the ShellSurface for
     * @param parent The parent to use for the ShellSurface
     * @returns created ShellSurface
     **/
    ShellSurface *createSurface(Surface *surface, QObject *parent = nullptr);

    operator wl_shell *();
    operator wl_shell *() const;

Q_SIGNALS:
    /**
     * This signal is emitted right before the interface is released.
     **/
    void interfaceAboutToBeReleased();
    /**
     * This signal is emitted right before the data is destroyed.
     **/
    void interfaceAboutToBeDestroyed();

    /**
     * The corresponding global for this interface on the Registry got removed.
     *
     * This signal gets only emitted if the Compositor got created by
     * Registry::createShell
     *
     * @since 5.5
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

/**
 * @short Wrapper for the wl_shell_surface interface.
 *
 * This class is a convenient wrapper for the wl_shell_surface interface.
 *
 * To create an instance use Shell::createSurface.
 *
 * @see Shell
 * @see Surface
 **/
class KWAYLANDCLIENT_EXPORT ShellSurface : public QObject
{
    Q_OBJECT
    /**
     * The size of the ShellSurface.
     **/
    Q_PROPERTY(QSize size READ size WRITE setSize NOTIFY sizeChanged)
public:
    explicit ShellSurface(QObject *parent);
    ~ShellSurface() override;

    /**
     * Releases the wl_shell_surface interface.
     * After the interface has been released the ShellSurface instance is no
     * longer valid and can be setup with another wl_shell_surface interface.
     *
     * This method is automatically invoked when the Shell which created this
     * ShellSurface gets released.
     **/
    void release();
    /**
     * Destroys the data held by this ShellSurface.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new wl_shell_surface interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Shell which created this
     * ShellSurface gets destroyed.
     *
     * @see release
     **/
    void destroy();
    /**
     * Setup this ShellSurface to manage the @p surface.
     * There is normally no need to call this method as it's invoked by
     * Shell::createSurface.
     **/
    void setup(wl_shell_surface *surface);
    QSize size() const;
    void setSize(const QSize &size);

    /**
     * Sets the ShellSurface fullscreen on @p output.
     **/
    void setFullscreen(Output *output = nullptr);
    void setMaximized(Output *output = nullptr);
    void setToplevel();
    /**
     * Flags which can be passed to a transient surface.
     * @see setTransient
     * @since 5.5
     **/
    enum class TransientFlag {
        Default = 0x0, ///< Default: transient surface accepts keyboard focus
        NoFocus = 0x1, ///< Transient surface does not accept keyboard focus
    };
    Q_DECLARE_FLAGS(TransientFlags, TransientFlag)
    /**
     * Sets this Surface as a transient for @p parent.
     *
     * @param parent The parent Surface of this surface
     * @param offset The offset of this Surface in the parent coordinate system
     * @param flags The flags for the transient
     * @since 5.5
     **/
    void setTransient(Surface *parent, const QPoint &offset = QPoint(), TransientFlags flags = TransientFlag::Default);

    /**
     * Sets this Surface as a popup transient for @p parent.
     *
     * A popup is a transient with an added pointer grab on the @p grabbedSeat.
     *
     * The popup grab can be created if the client has an implicit grab (e.g. button press)
     * on the @p grabbedSeat. It needs to pass the @p grabSerial indicating the implicit grab
     * to the request for setting the surface. The implicit grab is turned into a popup grab
     * which will persist after the implicit grab ends. The popup grab ends when the ShellSurface
     * gets destroyed or when the compositor breaks the grab through the {@link popupDone} signal.
     *
     * @param parent The parent Surface of this ShellSurface
     * @param grabbedSeat The Seat on which an implicit grab exists
     * @param grabSerial The serial of the implicit grab
     * @param offset The offset of this Surface in the parent coordinate system
     * @param flags The flags for the transient
     * @since 5.33
     **/
    void
    setTransientPopup(Surface *parent, Seat *grabbedSeat, quint32 grabSerial, const QPoint &offset = QPoint(), TransientFlags flags = TransientFlag::Default);

    bool isValid() const;

    /**
     * Requests a move on the given @p seat after the pointer button press with the given @p serial.
     *
     * @param seat The seat on which to move the window
     * @param serial The serial of the pointer button press which should trigger the move
     * @since 5.5
     **/
    void requestMove(Seat *seat, quint32 serial);

    /**
     * Requests a resize on the given @p seat after the pointer button press with the given @p serial.
     *
     * @param seat The seat on which to resize the window
     * @param serial The serial of the pointer button press which should trigger the resize
     * @param edges A hint for the compositor to set e.g. an appropriate cursor image
     * @since 5.5
     **/
    void requestResize(Seat *seat, quint32 serial, Qt::Edges edges);

    /**
     * Sets a short title for the surface.
     *
     * This string may be used to identify the surface in a task bar, window list, or other user
     * interface elements provided by the compositor.
     *
     * @since 5.55
     **/
    void setTitle(const QString &title);

    /**
     * Sets a window class for the surface.
     *
     * The surface class identifies the general class of applications to which the surface belongs.
     * A common convention is to use the file name (or the full path if it is a non-standard location)
     * of the application's .desktop file as the class.
     *
     * @since 5.55
     **/
    void setWindowClass(const QByteArray &windowClass);

    /**
     * Creates a ShellSurface for the given @p window.
     * This is an integration feature for QtWayland. On non-wayland platforms this method returns
     * @c nullptr as well as for not created QWindows.
     *
     * The returned ShellSurface will be fully setup, but won't be released. It gets automatically
     * destroyed together with the @p window.
     * @since 5.28
     **/
    static ShellSurface *fromWindow(QWindow *window);

    /**
     * Creates a ShellSurface for the given @p winId.
     * This is an integration feature for QtWayland. On non-wayland platforms this method returns
     * @c nullptr as well as for not created QWindows.
     *
     * The returned ShellSurface will be fully setup, but won't be released. It gets automatically
     * destroyed together with the QWindow corresponding
     * the @p wid.
     * @since 5.28
     **/
    static ShellSurface *fromQtWinId(WId wid);

    /**
     * @returns The Surface referencing the @p native wl_surface or @c null if there is no such Surface.
     * @since 5.28
     **/
    static ShellSurface *get(wl_shell_surface *native);

    operator wl_shell_surface *();
    operator wl_shell_surface *() const;

Q_SIGNALS:
    /**
     * Signal is emitted when the ShellSurface received a ping request.
     * The ShellSurface automatically responds to the ping.
     **/
    void pinged();
    void sizeChanged(const QSize &);

    /**
     * The popupDone signal is sent out when a popup grab is broken, that is,
     * when the user clicks a surface that doesn't belong to the client owning
     * the popup surface.
     * @see setTransientPopup
     * @since 5.33
     **/
    void popupDone();

private:
    class Private;
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(KWayland::Client::ShellSurface::TransientFlag)
Q_DECLARE_METATYPE(KWayland::Client::ShellSurface::TransientFlags)

#endif
