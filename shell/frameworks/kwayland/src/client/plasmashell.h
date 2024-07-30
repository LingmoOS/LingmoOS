/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_PLASMASHELL_H
#define WAYLAND_PLASMASHELL_H

#include <QObject>
#include <QSize>

#include "KWayland/Client/kwaylandclient_export.h"

struct wl_surface;
struct org_kde_plasma_shell;
struct org_kde_plasma_surface;

namespace KWayland
{
namespace Client
{
class EventQueue;
class Surface;
class PlasmaShellSurface;

/**
 * @short Wrapper for the org_kde_plasma_shell interface.
 *
 * This class provides a convenient wrapper for the org_kde_plasma_shell interface.
 * It's main purpose is to create a PlasmaShellSurface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the Shell interface:
 * @code
 * PlasmaShell *s = registry->createPlasmaShell(name, version);
 * @endcode
 *
 * This creates the PlasmaShell and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * PlasmaShell *s = new PlasmaShell;
 * s->setup(registry->bindPlasmaShell(name, version));
 * @endcode
 *
 * The PlasmaShell can be used as a drop-in replacement for any org_kde_plasma_shell
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 * @see PlasmaShellSurface
 **/
class KWAYLANDCLIENT_EXPORT PlasmaShell : public QObject
{
    Q_OBJECT
public:
    explicit PlasmaShell(QObject *parent = nullptr);
    ~PlasmaShell() override;

    /**
     * @returns @c true if managing a org_kde_plasma_shell.
     **/
    bool isValid() const;
    /**
     * Releases the org_kde_plasma_shell interface.
     * After the interface has been released the PlasmaShell instance is no
     * longer valid and can be setup with another org_kde_plasma_shell interface.
     *
     * Right before the interface is released the signal interfaceAboutToBeReleased is emitted.
     * @see interfaceAboutToBeReleased
     **/
    void release();
    /**
     * Destroys the data held by this PlasmaShell.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. Once the connection becomes invalid, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_plasma_shell interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * PlasmaShell gets destroyed.
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
    void setup(org_kde_plasma_shell *shell);

    /**
     * Sets the @p queue to use for creating a Surface.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating a Surface.
     **/
    EventQueue *eventQueue();

    /**
     * Creates a PlasmaShellSurface for the given @p surface and sets it up.
     *
     * If a PlasmaShellSurface for the given @p surface has already been created
     * a pointer to the existing one is returned instead of creating a new surface.
     *
     * @param surface The native surface to create the PlasmaShellSurface for
     * @param parent The parent to use for the PlasmaShellSurface
     * @returns created PlasmaShellSurface
     **/
    PlasmaShellSurface *createSurface(wl_surface *surface, QObject *parent = nullptr);
    /**
     * Creates a PlasmaShellSurface for the given @p surface and sets it up.
     *
     * If a PlasmaShellSurface for the given @p surface has already been created
     * a pointer to the existing one is returned instead of creating a new surface.
     *
     * @param surface The Surface to create the PlasmaShellSurface for
     * @param parent The parent to use for the PlasmaShellSurface
     * @returns created PlasmaShellSurface
     **/
    PlasmaShellSurface *createSurface(Surface *surface, QObject *parent = nullptr);

    operator org_kde_plasma_shell *();
    operator org_kde_plasma_shell *() const;

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
     * Registry::createPlasmaShell
     *
     * @since 5.5
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

/**
 * @short Wrapper for the org_kde_plasma_surface interface.
 *
 * This class is a convenient wrapper for the org_kde_plasma_surface interface.
 *
 * To create an instance use PlasmaShell::createSurface.
 *
 * A PlasmaShellSurface is a privileged Surface which can add further hints to the
 * Wayland server about it's position and the usage role. The Wayland server is allowed
 * to ignore all requests.
 *
 * Even if a PlasmaShellSurface is created for a Surface a normal ShellSurface (or similar)
 * needs to be created to have the Surface mapped as a window by the Wayland server.
 *
 * @see PlasmaShell
 * @see Surface
 **/
class KWAYLANDCLIENT_EXPORT PlasmaShellSurface : public QObject
{
    Q_OBJECT
public:
    explicit PlasmaShellSurface(QObject *parent);
    ~PlasmaShellSurface() override;

    /**
     * Releases the org_kde_plasma_surface interface.
     * After the interface has been released the PlasmaShellSurface instance is no
     * longer valid and can be setup with another org_kde_plasma_surface interface.
     *
     * This method is automatically invoked when the PlasmaShell which created this
     * PlasmaShellSurface gets released.
     **/
    void release();
    /**
     * Destroys the data held by this PlasmaShellSurface.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_plasma_surface interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the PlasmaShell which created this
     * PlasmaShellSurface gets destroyed.
     *
     * @see release
     **/
    void destroy();
    /**
     * Setup this PlasmaShellSurface to manage the @p surface.
     * There is normally no need to call this method as it's invoked by
     * PlasmaShell::createSurface.
     **/
    void setup(org_kde_plasma_surface *surface);

    /**
     * @returns the PlasmaShellSurface * associated with surface,
     * if any, nullptr if not found.
     * @since 5.6
     */
    static PlasmaShellSurface *get(Surface *surf);

    /**
     * @returns @c true if managing a org_kde_plasma_surface.
     **/
    bool isValid() const;
    operator org_kde_plasma_surface *();
    operator org_kde_plasma_surface *() const;

    /**
     * Describes possible roles this PlasmaShellSurface can have.
     * The role can be used by the Wayland server to e.g. change the stacking order accordingly.
     **/
    enum class Role {
        Normal, ///< A normal Surface
        Desktop, ///< The Surface represents a desktop, normally stacked below all other surfaces
        Panel, ///< The Surface represents a panel (dock), normally stacked above normal surfaces
        OnScreenDisplay, ///< The Surface represents an on screen display, like a volume changed notification
        Notification, ///< The Surface represents a notification @since 5.24
        ToolTip, ///< The Surface represents a tooltip @since 5.24
        CriticalNotification, ///< The Surface represents a critical notification, like battery is running out @since 5.58
        AppletPopup, ///< The Surface used for applets
    };
    /**
     * Changes the requested Role to @p role.
     * @see role
     **/
    void setRole(Role role);
    /**
     * @returns The requested Role, default value is @c Role::Normal.
     * @see setRole
     **/
    Role role() const;
    /**
     * Requests to position this PlasmaShellSurface at @p point in global coordinates.
     **/
    void setPosition(const QPoint &point);

    /**
     * Request that the initial position of this surface will be under the cursor
     *
     * Has to be called before attaching any buffer to the corresponding surface.
     * @since 5.94
     **/
    void openUnderCursor();

    /**
     * Describes how a PlasmaShellSurface with role @c Role::Panel should behave.
     * @see Role
     **/
    enum class PanelBehavior {
        AlwaysVisible,
        AutoHide,
        WindowsCanCover,
        WindowsGoBelow,
    };
    /**
     * Sets the PanelBehavior for a PlasmaShellSurface with Role @c Role::Panel
     * @see setRole
     **/
    void setPanelBehavior(PanelBehavior behavior);

    /**
     * Setting this bit to the window, will make it say it prefers
     * to not be listed in the taskbar. Taskbar implementations
     * may or may not follow this hint.
     * @since 5.5
     */
    void setSkipTaskbar(bool skip);

    /**
     * Setting this bit on a window will indicate it does not prefer
     * to be included in a window switcher.
     * @since 5.47
     */
    void setSkipSwitcher(bool skip);

    /**
     * Requests to hide a surface with Role Panel and PanelBahvior AutoHide.
     *
     * Once the compositor has hidden the panel the signal {@link autoHidePanelHidden} gets
     * emitted. Once it is shown again the signal {@link autoHidePanelShown} gets emitted.
     *
     * To show the surface again from client side use {@link requestShowAutoHidingPanel}.
     *
     * @see autoHidePanelHidden
     * @see autoHidePanelShown
     * @see requestShowAutoHidingPanel
     * @since 5.28
     **/
    void requestHideAutoHidingPanel();

    /**
     * Requests to show a surface with Role Panel and PanelBahvior AutoHide.
     *
     * This request allows the client to show a surface which it previously
     * requested to be hidden with {@link requestHideAutoHidingPanel}.
     *
     * @see autoHidePanelHidden
     * @see autoHidePanelShown
     * @see requestHideAutoHidingPanel
     * @since 5.28
     **/
    void requestShowAutoHidingPanel();

    /**
     * Set whether a PlasmaShellSurface should get focus or not.
     *
     * By default some roles do not take focus. With this request the compositor
     * can be instructed to also pass focus.
     *
     * @param takesFocus Set to @c true if the surface should gain focus.
     * @since 5.28
     **/
    // KF6 TODO rename to make it generic
    void setPanelTakesFocus(bool takesFocus);

Q_SIGNALS:
    /**
     * Emitted when the compositor hid an auto hiding panel.
     * @see requestHideAutoHidingPanel
     * @see autoHidePanelShown
     * @see requestShowAutoHidingPanel
     * @since 5.28
     **/
    void autoHidePanelHidden();

    /**
     * Emitted when the compositor showed an auto hiding panel.
     * @see requestHideAutoHidingPanel
     * @see autoHidePanelHidden
     * @see requestShowAutoHidingPanel
     * @since 5.28
     **/
    void autoHidePanelShown();

private:
    friend class PlasmaShell;
    class Private;
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(KWayland::Client::PlasmaShellSurface::Role)
Q_DECLARE_METATYPE(KWayland::Client::PlasmaShellSurface::PanelBehavior)

#endif
