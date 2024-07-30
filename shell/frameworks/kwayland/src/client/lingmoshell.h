/*
    SPDX-FileCopyrightText: 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#ifndef WAYLAND_LINGMOSHELL_H
#define WAYLAND_LINGMOSHELL_H

#include <QObject>
#include <QSize>

#include "KWayland/Client/kwaylandclient_export.h"

struct wl_surface;
struct org_kde_lingmo_shell;
struct org_kde_lingmo_surface;

namespace KWayland
{
namespace Client
{
class EventQueue;
class Surface;
class LingmoShellSurface;

/**
 * @short Wrapper for the org_kde_lingmo_shell interface.
 *
 * This class provides a convenient wrapper for the org_kde_lingmo_shell interface.
 * It's main purpose is to create a LingmoShellSurface.
 *
 * To use this class one needs to interact with the Registry. There are two
 * possible ways to create the Shell interface:
 * @code
 * LingmoShell *s = registry->createLingmoShell(name, version);
 * @endcode
 *
 * This creates the LingmoShell and sets it up directly. As an alternative this
 * can also be done in a more low level way:
 * @code
 * LingmoShell *s = new LingmoShell;
 * s->setup(registry->bindLingmoShell(name, version));
 * @endcode
 *
 * The LingmoShell can be used as a drop-in replacement for any org_kde_lingmo_shell
 * pointer as it provides matching cast operators.
 *
 * @see Registry
 * @see LingmoShellSurface
 **/
class KWAYLANDCLIENT_EXPORT LingmoShell : public QObject
{
    Q_OBJECT
public:
    explicit LingmoShell(QObject *parent = nullptr);
    ~LingmoShell() override;

    /**
     * @returns @c true if managing a org_kde_lingmo_shell.
     **/
    bool isValid() const;
    /**
     * Releases the org_kde_lingmo_shell interface.
     * After the interface has been released the LingmoShell instance is no
     * longer valid and can be setup with another org_kde_lingmo_shell interface.
     *
     * Right before the interface is released the signal interfaceAboutToBeReleased is emitted.
     * @see interfaceAboutToBeReleased
     **/
    void release();
    /**
     * Destroys the data held by this LingmoShell.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. Once the connection becomes invalid, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_lingmo_shell interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the Registry which created this
     * LingmoShell gets destroyed.
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
    void setup(org_kde_lingmo_shell *shell);

    /**
     * Sets the @p queue to use for creating a Surface.
     **/
    void setEventQueue(EventQueue *queue);
    /**
     * @returns The event queue to use for creating a Surface.
     **/
    EventQueue *eventQueue();

    /**
     * Creates a LingmoShellSurface for the given @p surface and sets it up.
     *
     * If a LingmoShellSurface for the given @p surface has already been created
     * a pointer to the existing one is returned instead of creating a new surface.
     *
     * @param surface The native surface to create the LingmoShellSurface for
     * @param parent The parent to use for the LingmoShellSurface
     * @returns created LingmoShellSurface
     **/
    LingmoShellSurface *createSurface(wl_surface *surface, QObject *parent = nullptr);
    /**
     * Creates a LingmoShellSurface for the given @p surface and sets it up.
     *
     * If a LingmoShellSurface for the given @p surface has already been created
     * a pointer to the existing one is returned instead of creating a new surface.
     *
     * @param surface The Surface to create the LingmoShellSurface for
     * @param parent The parent to use for the LingmoShellSurface
     * @returns created LingmoShellSurface
     **/
    LingmoShellSurface *createSurface(Surface *surface, QObject *parent = nullptr);

    operator org_kde_lingmo_shell *();
    operator org_kde_lingmo_shell *() const;

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
     * Registry::createLingmoShell
     *
     * @since 5.5
     **/
    void removed();

private:
    class Private;
    QScopedPointer<Private> d;
};

/**
 * @short Wrapper for the org_kde_lingmo_surface interface.
 *
 * This class is a convenient wrapper for the org_kde_lingmo_surface interface.
 *
 * To create an instance use LingmoShell::createSurface.
 *
 * A LingmoShellSurface is a privileged Surface which can add further hints to the
 * Wayland server about it's position and the usage role. The Wayland server is allowed
 * to ignore all requests.
 *
 * Even if a LingmoShellSurface is created for a Surface a normal ShellSurface (or similar)
 * needs to be created to have the Surface mapped as a window by the Wayland server.
 *
 * @see LingmoShell
 * @see Surface
 **/
class KWAYLANDCLIENT_EXPORT LingmoShellSurface : public QObject
{
    Q_OBJECT
public:
    explicit LingmoShellSurface(QObject *parent);
    ~LingmoShellSurface() override;

    /**
     * Releases the org_kde_lingmo_surface interface.
     * After the interface has been released the LingmoShellSurface instance is no
     * longer valid and can be setup with another org_kde_lingmo_surface interface.
     *
     * This method is automatically invoked when the LingmoShell which created this
     * LingmoShellSurface gets released.
     **/
    void release();
    /**
     * Destroys the data held by this LingmoShellSurface.
     * This method is supposed to be used when the connection to the Wayland
     * server goes away. If the connection is not valid anymore, it's not
     * possible to call release anymore as that calls into the Wayland
     * connection and the call would fail. This method cleans up the data, so
     * that the instance can be deleted or set up to a new org_kde_lingmo_surface interface
     * once there is a new connection available.
     *
     * This method is automatically invoked when the LingmoShell which created this
     * LingmoShellSurface gets destroyed.
     *
     * @see release
     **/
    void destroy();
    /**
     * Setup this LingmoShellSurface to manage the @p surface.
     * There is normally no need to call this method as it's invoked by
     * LingmoShell::createSurface.
     **/
    void setup(org_kde_lingmo_surface *surface);

    /**
     * @returns the LingmoShellSurface * associated with surface,
     * if any, nullptr if not found.
     * @since 5.6
     */
    static LingmoShellSurface *get(Surface *surf);

    /**
     * @returns @c true if managing a org_kde_lingmo_surface.
     **/
    bool isValid() const;
    operator org_kde_lingmo_surface *();
    operator org_kde_lingmo_surface *() const;

    /**
     * Describes possible roles this LingmoShellSurface can have.
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
     * Requests to position this LingmoShellSurface at @p point in global coordinates.
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
     * Describes how a LingmoShellSurface with role @c Role::Panel should behave.
     * @see Role
     **/
    enum class PanelBehavior {
        AlwaysVisible,
        AutoHide,
        WindowsCanCover,
        WindowsGoBelow,
    };
    /**
     * Sets the PanelBehavior for a LingmoShellSurface with Role @c Role::Panel
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
     * Set whether a LingmoShellSurface should get focus or not.
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
    friend class LingmoShell;
    class Private;
    QScopedPointer<Private> d;
};

}
}

Q_DECLARE_METATYPE(KWayland::Client::LingmoShellSurface::Role)
Q_DECLARE_METATYPE(KWayland::Client::LingmoShellSurface::PanelBehavior)

#endif
