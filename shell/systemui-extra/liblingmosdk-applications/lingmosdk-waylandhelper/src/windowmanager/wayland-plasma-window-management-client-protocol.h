/* Generated by wayland-scanner 1.22.0 */

#ifndef PLASMA_WINDOW_MANAGEMENT_CLIENT_PROTOCOL_H
#define PLASMA_WINDOW_MANAGEMENT_CLIENT_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>
#include "wayland-client.h"

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @page page_plasma_window_management The plasma_window_management protocol
 * @section page_ifaces_plasma_window_management Interfaces
 * - @subpage page_iface_org_kde_plasma_window_management - application windows management
 * - @subpage page_iface_org_kde_plasma_window - interface to control application windows
 * - @subpage page_iface_org_kde_plasma_activation_feedback - activation feedback
 * - @subpage page_iface_org_kde_plasma_activation - 
 * @section page_copyright_plasma_window_management Copyright
 * <pre>
 *
 * SPDX-FileCopyrightText: 2013-2014 Pier Luigi Fiorini
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 * </pre>
 */
struct org_kde_plasma_activation;
struct org_kde_plasma_activation_feedback;
struct org_kde_plasma_window;
struct org_kde_plasma_window_management;
struct wl_output;
struct wl_surface;

#ifndef ORG_KDE_PLASMA_WINDOW_MANAGEMENT_INTERFACE
#define ORG_KDE_PLASMA_WINDOW_MANAGEMENT_INTERFACE
/**
 * @page page_iface_org_kde_plasma_window_management org_kde_plasma_window_management
 * @section page_iface_org_kde_plasma_window_management_desc Description
 *
 * This interface manages application windows.
 * It provides requests to show and hide the desktop and emits
 * an event every time a window is created so that the client can
 * use it to manage the window.
 *
 * Only one client can bind this interface at a time.
 * @section page_iface_org_kde_plasma_window_management_api API
 * See @ref iface_org_kde_plasma_window_management.
 */
/**
 * @defgroup iface_org_kde_plasma_window_management The org_kde_plasma_window_management interface
 *
 * This interface manages application windows.
 * It provides requests to show and hide the desktop and emits
 * an event every time a window is created so that the client can
 * use it to manage the window.
 *
 * Only one client can bind this interface at a time.
 */
extern const struct wl_interface org_kde_plasma_window_management_interface;
#endif
#ifndef ORG_KDE_PLASMA_WINDOW_INTERFACE
#define ORG_KDE_PLASMA_WINDOW_INTERFACE
/**
 * @page page_iface_org_kde_plasma_window org_kde_plasma_window
 * @section page_iface_org_kde_plasma_window_desc Description
 *
 * Manages and control an application window.
 *
 * Only one client can bind this interface at a time.
 * @section page_iface_org_kde_plasma_window_api API
 * See @ref iface_org_kde_plasma_window.
 */
/**
 * @defgroup iface_org_kde_plasma_window The org_kde_plasma_window interface
 *
 * Manages and control an application window.
 *
 * Only one client can bind this interface at a time.
 */
extern const struct wl_interface org_kde_plasma_window_interface;
#endif
#ifndef ORG_KDE_PLASMA_ACTIVATION_FEEDBACK_INTERFACE
#define ORG_KDE_PLASMA_ACTIVATION_FEEDBACK_INTERFACE
/**
 * @page page_iface_org_kde_plasma_activation_feedback org_kde_plasma_activation_feedback
 * @section page_iface_org_kde_plasma_activation_feedback_desc Description
 *
 * The activation manager interface provides a way to get notified
 * when an application is about to be activated.
 * @section page_iface_org_kde_plasma_activation_feedback_api API
 * See @ref iface_org_kde_plasma_activation_feedback.
 */
/**
 * @defgroup iface_org_kde_plasma_activation_feedback The org_kde_plasma_activation_feedback interface
 *
 * The activation manager interface provides a way to get notified
 * when an application is about to be activated.
 */
extern const struct wl_interface org_kde_plasma_activation_feedback_interface;
#endif
#ifndef ORG_KDE_PLASMA_ACTIVATION_INTERFACE
#define ORG_KDE_PLASMA_ACTIVATION_INTERFACE
/**
 * @page page_iface_org_kde_plasma_activation org_kde_plasma_activation
 * @section page_iface_org_kde_plasma_activation_api API
 * See @ref iface_org_kde_plasma_activation.
 */
/**
 * @defgroup iface_org_kde_plasma_activation The org_kde_plasma_activation interface
 */
extern const struct wl_interface org_kde_plasma_activation_interface;
#endif

#ifndef ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_ENUM
#define ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_ENUM
enum org_kde_plasma_window_management_state {
	ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_ACTIVE = 1 << 0,
	ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MINIMIZED = 1 << 1,
	ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MAXIMIZED = 1 << 2,
	ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_FULLSCREEN = 1 << 3,
	ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_KEEP_ABOVE = 1 << 4,
	ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_KEEP_BELOW = 1 << 5,
	ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_ON_ALL_DESKTOPS = 1 << 6,
	ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_DEMANDS_ATTENTION = 1 << 7,
	ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_CLOSEABLE = 1 << 8,
	ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MINIMIZABLE = 1 << 9,
	ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MAXIMIZABLE = 1 << 10,
	ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_FULLSCREENABLE = 1 << 11,
	/**
	 * @since 2
	 */
	ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_SKIPTASKBAR = 1 << 12,
	/**
	 * @since 3
	 */
	ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_SHADEABLE = 1 << 13,
	/**
	 * @since 3
	 */
	ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_SHADED = 1 << 14,
	/**
	 * @since 3
	 */
	ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MOVABLE = 1 << 15,
	/**
	 * @since 3
	 */
	ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_RESIZABLE = 1 << 16,
	/**
	 * @since 3
	 */
	ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_VIRTUAL_DESKTOP_CHANGEABLE = 1 << 17,
	/**
	 * @since 9
	 */
	ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_SKIPSWITCHER = 1 << 18,
};
/**
 * @ingroup iface_org_kde_plasma_window_management
 */
#define ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_SKIPTASKBAR_SINCE_VERSION 2
/**
 * @ingroup iface_org_kde_plasma_window_management
 */
#define ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_SHADEABLE_SINCE_VERSION 3
/**
 * @ingroup iface_org_kde_plasma_window_management
 */
#define ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_SHADED_SINCE_VERSION 3
/**
 * @ingroup iface_org_kde_plasma_window_management
 */
#define ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_MOVABLE_SINCE_VERSION 3
/**
 * @ingroup iface_org_kde_plasma_window_management
 */
#define ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_RESIZABLE_SINCE_VERSION 3
/**
 * @ingroup iface_org_kde_plasma_window_management
 */
#define ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_VIRTUAL_DESKTOP_CHANGEABLE_SINCE_VERSION 3
/**
 * @ingroup iface_org_kde_plasma_window_management
 */
#define ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_SKIPSWITCHER_SINCE_VERSION 9
#endif /* ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STATE_ENUM */

#ifndef ORG_KDE_PLASMA_WINDOW_MANAGEMENT_SHOW_DESKTOP_ENUM
#define ORG_KDE_PLASMA_WINDOW_MANAGEMENT_SHOW_DESKTOP_ENUM
enum org_kde_plasma_window_management_show_desktop {
	ORG_KDE_PLASMA_WINDOW_MANAGEMENT_SHOW_DESKTOP_DISABLED = 0,
	ORG_KDE_PLASMA_WINDOW_MANAGEMENT_SHOW_DESKTOP_ENABLED = 1,
};
#endif /* ORG_KDE_PLASMA_WINDOW_MANAGEMENT_SHOW_DESKTOP_ENUM */

/**
 * @ingroup iface_org_kde_plasma_window_management
 * @struct org_kde_plasma_window_management_listener
 */
struct org_kde_plasma_window_management_listener {
	/**
	 * notify the client when the show desktop mode is entered/left
	 *
	 * This event will be sent whenever the show desktop mode
	 * changes. E.g. when it is entered or left.
	 *
	 * On binding the interface the current state is sent.
	 * @param state new state
	 */
	void (*show_desktop_changed)(void *data,
				     struct org_kde_plasma_window_management *org_kde_plasma_window_management,
				     uint32_t state);
	/**
	 * notify the client that a window was mapped
	 *
	 * This event will be sent immediately after a window is mapped.
	 * @param id Deprecated: internal window Id
	 */
	void (*window)(void *data,
		       struct org_kde_plasma_window_management *org_kde_plasma_window_management,
		       uint32_t id);
	/**
	 * notify the client when stacking order changed
	 *
	 * This event will be sent when stacking order changed and on
	 * bind
	 * @param ids internal windows id array
	 * @since 11
	 */
	void (*stacking_order_changed)(void *data,
				       struct org_kde_plasma_window_management *org_kde_plasma_window_management,
				       struct wl_array *ids);
	/**
	 * notify the client when stacking order changed
	 *
	 * This event will be sent when stacking order changed and on
	 * bind
	 * @param uuids internal windows id ;-separated
	 * @since 12
	 */
	void (*stacking_order_uuid_changed)(void *data,
					    struct org_kde_plasma_window_management *org_kde_plasma_window_management,
					    const char *uuids);
	/**
	 * notify the client that a window was mapped
	 *
	 * This event will be sent immediately after a window is mapped.
	 * @param id Deprecated: internal window Id
	 * @param uuid internal window uuid
	 * @since 13
	 */
	void (*window_with_uuid)(void *data,
				 struct org_kde_plasma_window_management *org_kde_plasma_window_management,
				 uint32_t id,
				 const char *uuid);
};

/**
 * @ingroup iface_org_kde_plasma_window_management
 */
static inline int
org_kde_plasma_window_management_add_listener(struct org_kde_plasma_window_management *org_kde_plasma_window_management,
					      const struct org_kde_plasma_window_management_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) org_kde_plasma_window_management,
				     (void (**)(void)) listener, data);
}

#define ORG_KDE_PLASMA_WINDOW_MANAGEMENT_SHOW_DESKTOP 0
#define ORG_KDE_PLASMA_WINDOW_MANAGEMENT_GET_WINDOW 1
#define ORG_KDE_PLASMA_WINDOW_MANAGEMENT_GET_WINDOW_BY_UUID 2

/**
 * @ingroup iface_org_kde_plasma_window_management
 */
#define ORG_KDE_PLASMA_WINDOW_MANAGEMENT_SHOW_DESKTOP_CHANGED_SINCE_VERSION 1
/**
 * @ingroup iface_org_kde_plasma_window_management
 */
#define ORG_KDE_PLASMA_WINDOW_MANAGEMENT_WINDOW_SINCE_VERSION 1
/**
 * @ingroup iface_org_kde_plasma_window_management
 */
#define ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STACKING_ORDER_CHANGED_SINCE_VERSION 11
/**
 * @ingroup iface_org_kde_plasma_window_management
 */
#define ORG_KDE_PLASMA_WINDOW_MANAGEMENT_STACKING_ORDER_UUID_CHANGED_SINCE_VERSION 12
/**
 * @ingroup iface_org_kde_plasma_window_management
 */
#define ORG_KDE_PLASMA_WINDOW_MANAGEMENT_WINDOW_WITH_UUID_SINCE_VERSION 13

/**
 * @ingroup iface_org_kde_plasma_window_management
 */
#define ORG_KDE_PLASMA_WINDOW_MANAGEMENT_SHOW_DESKTOP_SINCE_VERSION 1
/**
 * @ingroup iface_org_kde_plasma_window_management
 */
#define ORG_KDE_PLASMA_WINDOW_MANAGEMENT_GET_WINDOW_SINCE_VERSION 1
/**
 * @ingroup iface_org_kde_plasma_window_management
 */
#define ORG_KDE_PLASMA_WINDOW_MANAGEMENT_GET_WINDOW_BY_UUID_SINCE_VERSION 12

/** @ingroup iface_org_kde_plasma_window_management */
static inline void
org_kde_plasma_window_management_set_user_data(struct org_kde_plasma_window_management *org_kde_plasma_window_management, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) org_kde_plasma_window_management, user_data);
}

/** @ingroup iface_org_kde_plasma_window_management */
static inline void *
org_kde_plasma_window_management_get_user_data(struct org_kde_plasma_window_management *org_kde_plasma_window_management)
{
	return wl_proxy_get_user_data((struct wl_proxy *) org_kde_plasma_window_management);
}

static inline uint32_t
org_kde_plasma_window_management_get_version(struct org_kde_plasma_window_management *org_kde_plasma_window_management)
{
	return wl_proxy_get_version((struct wl_proxy *) org_kde_plasma_window_management);
}

/** @ingroup iface_org_kde_plasma_window_management */
static inline void
org_kde_plasma_window_management_destroy(struct org_kde_plasma_window_management *org_kde_plasma_window_management)
{
	wl_proxy_destroy((struct wl_proxy *) org_kde_plasma_window_management);
}

/**
 * @ingroup iface_org_kde_plasma_window_management
 *
 * Tell the compositor to show/hide the desktop.
 */
static inline void
org_kde_plasma_window_management_show_desktop(struct org_kde_plasma_window_management *org_kde_plasma_window_management, uint32_t state)
{
	wl_proxy_marshal_flags((struct wl_proxy *) org_kde_plasma_window_management,
			 ORG_KDE_PLASMA_WINDOW_MANAGEMENT_SHOW_DESKTOP, NULL, wl_proxy_get_version((struct wl_proxy *) org_kde_plasma_window_management), 0, state);
}

/**
 * @ingroup iface_org_kde_plasma_window_management
 * Deprecated: use get_window_by_uuid
 */
static inline struct org_kde_plasma_window *
org_kde_plasma_window_management_get_window(struct org_kde_plasma_window_management *org_kde_plasma_window_management, uint32_t internal_window_id)
{
	struct wl_proxy *id;

	id = wl_proxy_marshal_flags((struct wl_proxy *) org_kde_plasma_window_management,
			 ORG_KDE_PLASMA_WINDOW_MANAGEMENT_GET_WINDOW, &org_kde_plasma_window_interface, wl_proxy_get_version((struct wl_proxy *) org_kde_plasma_window_management), 0, NULL, internal_window_id);

	return (struct org_kde_plasma_window *) id;
}

/**
 * @ingroup iface_org_kde_plasma_window_management
 */
static inline struct org_kde_plasma_window *
org_kde_plasma_window_management_get_window_by_uuid(struct org_kde_plasma_window_management *org_kde_plasma_window_management, const char *internal_window_uuid)
{
	struct wl_proxy *id;

	id = wl_proxy_marshal_flags((struct wl_proxy *) org_kde_plasma_window_management,
			 ORG_KDE_PLASMA_WINDOW_MANAGEMENT_GET_WINDOW_BY_UUID, &org_kde_plasma_window_interface, wl_proxy_get_version((struct wl_proxy *) org_kde_plasma_window_management), 0, NULL, internal_window_uuid);

	return (struct org_kde_plasma_window *) id;
}

/**
 * @ingroup iface_org_kde_plasma_window
 * @struct org_kde_plasma_window_listener
 */
struct org_kde_plasma_window_listener {
	/**
	 * window title has been changed
	 *
	 * This event will be sent as soon as the window title is
	 * changed.
	 * @param title window title
	 */
	void (*title_changed)(void *data,
			      struct org_kde_plasma_window *org_kde_plasma_window,
			      const char *title);
	/**
	 * application identifier has been changed
	 *
	 * This event will be sent as soon as the application identifier
	 * is changed.
	 */
	void (*app_id_changed)(void *data,
			       struct org_kde_plasma_window *org_kde_plasma_window,
			       const char *app_id);
	/**
	 * window state has been changed
	 *
	 * This event will be sent as soon as the window state changes.
	 *
	 * Values for state argument are described by
	 * org_kde_plasma_window_management.state.
	 * @param flags bitfield of state flags
	 */
	void (*state_changed)(void *data,
			      struct org_kde_plasma_window *org_kde_plasma_window,
			      uint32_t flags);
	/**
	 * window was moved to another workspace
	 *
	 * DEPRECATED: use virtual_desktop_entered and
	 * virtual_desktop_left instead This event will be sent when a
	 * window is moved to another virtual desktop.
	 *
	 * It is not sent if it becomes visible on all virtual desktops
	 * though.
	 * @param number zero based virtual desktop number
	 */
	void (*virtual_desktop_changed)(void *data,
					struct org_kde_plasma_window *org_kde_plasma_window,
					int32_t number);
	/**
	 * window's icon name changed
	 *
	 * This event will be sent whenever the themed icon name changes.
	 * May be null.
	 * @param name the new themed icon name
	 */
	void (*themed_icon_name_changed)(void *data,
					 struct org_kde_plasma_window *org_kde_plasma_window,
					 const char *name);
	/**
	 * window's surface was unmapped
	 *
	 * This event will be sent immediately after the window is closed
	 * and its surface is unmapped.
	 */
	void (*unmapped)(void *data,
			 struct org_kde_plasma_window *org_kde_plasma_window);
	/**
	 * All initial known state is submitted
	 *
	 * This event will be sent immediately after all initial state
	 * been sent to the client. If the Plasma window is already
	 * unmapped, the unmapped event will be sent before the
	 * initial_state event.
	 * @since 4
	 */
	void (*initial_state)(void *data,
			      struct org_kde_plasma_window *org_kde_plasma_window);
	/**
	 * The parent window changed
	 *
	 * This event will be sent whenever the parent window of this
	 * org_kde_plasma_window changes. The passed parent is another
	 * org_kde_plasma_window and this org_kde_plasma_window is a
	 * transient window to the parent window. If the parent argument is
	 * null, this org_kde_plasma_window does not have a parent window.
	 * @param parent The parent window
	 * @since 5
	 */
	void (*parent_window)(void *data,
			      struct org_kde_plasma_window *org_kde_plasma_window,
			      struct org_kde_plasma_window *parent);
	/**
	 * The geometry of this window in absolute coordinates
	 *
	 * This event will be sent whenever the window geometry of this
	 * org_kde_plasma_window changes. The coordinates are in absolute
	 * coordinates of the windowing system.
	 * @param x x position of the org_kde_plasma_window
	 * @param y y position of the org_kde_plasma_window
	 * @param width width of the org_kde_plasma_window
	 * @param height height of the org_kde_plasma_window
	 * @since 6
	 */
	void (*geometry)(void *data,
			 struct org_kde_plasma_window *org_kde_plasma_window,
			 int32_t x,
			 int32_t y,
			 uint32_t width,
			 uint32_t height);
	/**
	 * The icon of the window changed
	 *
	 * This event will be sent whenever the icon of the window
	 * changes, but there is no themed icon name. Common examples are
	 * Xwayland windows which have a pixmap based icon.
	 *
	 * The client can request the icon using get_icon.
	 * @since 7
	 */
	void (*icon_changed)(void *data,
			     struct org_kde_plasma_window *org_kde_plasma_window);
	/**
	 * process id of application owning the window has changed
	 *
	 * This event will be sent when the compositor has set the
	 * process id this window belongs to. This should be set once
	 * before the initial_state is sent.
	 * @param pid process id
	 */
	void (*pid_changed)(void *data,
			    struct org_kde_plasma_window *org_kde_plasma_window,
			    uint32_t pid);
	/**
	 * the window entered a new virtual desktop
	 *
	 * This event will be sent when the window has entered a new
	 * virtual desktop. The window can be on more than one desktop, or
	 * none: then is considered on all of them.
	 * @param id desktop id
	 * @since 8
	 */
	void (*virtual_desktop_entered)(void *data,
					struct org_kde_plasma_window *org_kde_plasma_window,
					const char *id);
	/**
	 * the window left a virtual desktop
	 *
	 * This event will be sent when the window left a virtual
	 * desktop. If the window leaves all desktops, it can be considered
	 * on all. If the window gets manually added on all desktops, the
	 * server has to send virtual_desktop_left for every previous
	 * desktop it was in for the window to be really considered on all
	 * desktops.
	 * @param is desktop id
	 * @since 8
	 */
	void (*virtual_desktop_left)(void *data,
				     struct org_kde_plasma_window *org_kde_plasma_window,
				     const char *is);
	/**
	 * notify the client that the current appmenu changed
	 *
	 * This event will be sent after the application menu for the
	 * window has changed.
	 * @since 10
	 */
	void (*application_menu)(void *data,
				 struct org_kde_plasma_window *org_kde_plasma_window,
				 const char *service_name,
				 const char *object_path);
	/**
	 * the window entered an activity
	 *
	 * This event will be sent when the window has entered an
	 * activity. The window can be on more than one activity, or none:
	 * then is considered on all of them.
	 * @param id activity id
	 * @since 14
	 */
	void (*activity_entered)(void *data,
				 struct org_kde_plasma_window *org_kde_plasma_window,
				 const char *id);
	/**
	 * the window left an activity
	 *
	 * This event will be sent when the window left an activity. If
	 * the window leaves all activities, it will be considered on all.
	 * If the window gets manually added on all activities, the server
	 * has to send activity_left for every previous activity it was in
	 * for the window to be really considered on all activities.
	 * @param id activity id
	 * @since 14
	 */
	void (*activity_left)(void *data,
			      struct org_kde_plasma_window *org_kde_plasma_window,
			      const char *id);
	/**
	 * X11 resource name has changed
	 *
	 * This event will be sent when the X11 resource name of the
	 * window has changed. This is only set for XWayland windows.
	 * @param resource_name resource name
	 * @since 16
	 */
	void (*resource_name_changed)(void *data,
				      struct org_kde_plasma_window *org_kde_plasma_window,
				      const char *resource_name);
};

/**
 * @ingroup iface_org_kde_plasma_window
 */
static inline int
org_kde_plasma_window_add_listener(struct org_kde_plasma_window *org_kde_plasma_window,
				   const struct org_kde_plasma_window_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) org_kde_plasma_window,
				     (void (**)(void)) listener, data);
}

#define ORG_KDE_PLASMA_WINDOW_SET_STATE 0
#define ORG_KDE_PLASMA_WINDOW_SET_VIRTUAL_DESKTOP 1
#define ORG_KDE_PLASMA_WINDOW_SET_MINIMIZED_GEOMETRY 2
#define ORG_KDE_PLASMA_WINDOW_UNSET_MINIMIZED_GEOMETRY 3
#define ORG_KDE_PLASMA_WINDOW_CLOSE 4
#define ORG_KDE_PLASMA_WINDOW_REQUEST_MOVE 5
#define ORG_KDE_PLASMA_WINDOW_REQUEST_RESIZE 6
#define ORG_KDE_PLASMA_WINDOW_DESTROY 7
#define ORG_KDE_PLASMA_WINDOW_GET_ICON 8
#define ORG_KDE_PLASMA_WINDOW_REQUEST_ENTER_VIRTUAL_DESKTOP 9
#define ORG_KDE_PLASMA_WINDOW_REQUEST_ENTER_NEW_VIRTUAL_DESKTOP 10
#define ORG_KDE_PLASMA_WINDOW_REQUEST_LEAVE_VIRTUAL_DESKTOP 11
#define ORG_KDE_PLASMA_WINDOW_REQUEST_ENTER_ACTIVITY 12
#define ORG_KDE_PLASMA_WINDOW_REQUEST_LEAVE_ACTIVITY 13
#define ORG_KDE_PLASMA_WINDOW_SEND_TO_OUTPUT 14

/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_TITLE_CHANGED_SINCE_VERSION 1
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_APP_ID_CHANGED_SINCE_VERSION 1
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_STATE_CHANGED_SINCE_VERSION 1
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_VIRTUAL_DESKTOP_CHANGED_SINCE_VERSION 1
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_THEMED_ICON_NAME_CHANGED_SINCE_VERSION 1
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_UNMAPPED_SINCE_VERSION 1
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_INITIAL_STATE_SINCE_VERSION 4
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_PARENT_WINDOW_SINCE_VERSION 5
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_GEOMETRY_SINCE_VERSION 6
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_ICON_CHANGED_SINCE_VERSION 7
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_PID_CHANGED_SINCE_VERSION 1
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_VIRTUAL_DESKTOP_ENTERED_SINCE_VERSION 8
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_VIRTUAL_DESKTOP_LEFT_SINCE_VERSION 8
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_APPLICATION_MENU_SINCE_VERSION 10
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_ACTIVITY_ENTERED_SINCE_VERSION 14
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_ACTIVITY_LEFT_SINCE_VERSION 14
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_RESOURCE_NAME_CHANGED_SINCE_VERSION 16

/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_SET_STATE_SINCE_VERSION 1
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_SET_VIRTUAL_DESKTOP_SINCE_VERSION 1
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_SET_MINIMIZED_GEOMETRY_SINCE_VERSION 1
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_UNSET_MINIMIZED_GEOMETRY_SINCE_VERSION 1
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_CLOSE_SINCE_VERSION 1
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_REQUEST_MOVE_SINCE_VERSION 3
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_REQUEST_RESIZE_SINCE_VERSION 3
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_DESTROY_SINCE_VERSION 4
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_GET_ICON_SINCE_VERSION 7
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_REQUEST_ENTER_VIRTUAL_DESKTOP_SINCE_VERSION 8
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_REQUEST_ENTER_NEW_VIRTUAL_DESKTOP_SINCE_VERSION 8
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_REQUEST_LEAVE_VIRTUAL_DESKTOP_SINCE_VERSION 8
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_REQUEST_ENTER_ACTIVITY_SINCE_VERSION 14
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_REQUEST_LEAVE_ACTIVITY_SINCE_VERSION 14
/**
 * @ingroup iface_org_kde_plasma_window
 */
#define ORG_KDE_PLASMA_WINDOW_SEND_TO_OUTPUT_SINCE_VERSION 15

/** @ingroup iface_org_kde_plasma_window */
static inline void
org_kde_plasma_window_set_user_data(struct org_kde_plasma_window *org_kde_plasma_window, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) org_kde_plasma_window, user_data);
}

/** @ingroup iface_org_kde_plasma_window */
static inline void *
org_kde_plasma_window_get_user_data(struct org_kde_plasma_window *org_kde_plasma_window)
{
	return wl_proxy_get_user_data((struct wl_proxy *) org_kde_plasma_window);
}

static inline uint32_t
org_kde_plasma_window_get_version(struct org_kde_plasma_window *org_kde_plasma_window)
{
	return wl_proxy_get_version((struct wl_proxy *) org_kde_plasma_window);
}

/**
 * @ingroup iface_org_kde_plasma_window
 *
 * Set window state.
 *
 * Values for state argument are described by org_kde_plasma_window_management.state
 * and can be used together in a bitfield. The flags bitfield describes which flags are
 * supposed to be set, the state bitfield the value for the set flags
 */
static inline void
org_kde_plasma_window_set_state(struct org_kde_plasma_window *org_kde_plasma_window, uint32_t flags, uint32_t state)
{
	wl_proxy_marshal_flags((struct wl_proxy *) org_kde_plasma_window,
			 ORG_KDE_PLASMA_WINDOW_SET_STATE, NULL, wl_proxy_get_version((struct wl_proxy *) org_kde_plasma_window), 0, flags, state);
}

/**
 * @ingroup iface_org_kde_plasma_window
 *
 * Deprecated: use enter_virtual_desktop
 * Maps the window to a different virtual desktop.
 *
 * To show the window on all virtual desktops, call the
 * org_kde_plasma_window.set_state request and specify a on_all_desktops
 * state in the bitfield.
 */
static inline void
org_kde_plasma_window_set_virtual_desktop(struct org_kde_plasma_window *org_kde_plasma_window, uint32_t number)
{
	wl_proxy_marshal_flags((struct wl_proxy *) org_kde_plasma_window,
			 ORG_KDE_PLASMA_WINDOW_SET_VIRTUAL_DESKTOP, NULL, wl_proxy_get_version((struct wl_proxy *) org_kde_plasma_window), 0, number);
}

/**
 * @ingroup iface_org_kde_plasma_window
 *
 * Sets the geometry of the taskbar entry for this window.
 * The geometry is relative to a panel in particular.
 */
static inline void
org_kde_plasma_window_set_minimized_geometry(struct org_kde_plasma_window *org_kde_plasma_window, struct wl_surface *panel, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
	wl_proxy_marshal_flags((struct wl_proxy *) org_kde_plasma_window,
			 ORG_KDE_PLASMA_WINDOW_SET_MINIMIZED_GEOMETRY, NULL, wl_proxy_get_version((struct wl_proxy *) org_kde_plasma_window), 0, panel, x, y, width, height);
}

/**
 * @ingroup iface_org_kde_plasma_window
 *
 * Remove the task geometry information for a particular panel.
 */
static inline void
org_kde_plasma_window_unset_minimized_geometry(struct org_kde_plasma_window *org_kde_plasma_window, struct wl_surface *panel)
{
	wl_proxy_marshal_flags((struct wl_proxy *) org_kde_plasma_window,
			 ORG_KDE_PLASMA_WINDOW_UNSET_MINIMIZED_GEOMETRY, NULL, wl_proxy_get_version((struct wl_proxy *) org_kde_plasma_window), 0, panel);
}

/**
 * @ingroup iface_org_kde_plasma_window
 *
 * Close this window.
 */
static inline void
org_kde_plasma_window_close(struct org_kde_plasma_window *org_kde_plasma_window)
{
	wl_proxy_marshal_flags((struct wl_proxy *) org_kde_plasma_window,
			 ORG_KDE_PLASMA_WINDOW_CLOSE, NULL, wl_proxy_get_version((struct wl_proxy *) org_kde_plasma_window), 0);
}

/**
 * @ingroup iface_org_kde_plasma_window
 *
 * Request an interactive move for this window.
 */
static inline void
org_kde_plasma_window_request_move(struct org_kde_plasma_window *org_kde_plasma_window)
{
	wl_proxy_marshal_flags((struct wl_proxy *) org_kde_plasma_window,
			 ORG_KDE_PLASMA_WINDOW_REQUEST_MOVE, NULL, wl_proxy_get_version((struct wl_proxy *) org_kde_plasma_window), 0);
}

/**
 * @ingroup iface_org_kde_plasma_window
 *
 * Request an interactive resize for this window.
 */
static inline void
org_kde_plasma_window_request_resize(struct org_kde_plasma_window *org_kde_plasma_window)
{
	wl_proxy_marshal_flags((struct wl_proxy *) org_kde_plasma_window,
			 ORG_KDE_PLASMA_WINDOW_REQUEST_RESIZE, NULL, wl_proxy_get_version((struct wl_proxy *) org_kde_plasma_window), 0);
}

/**
 * @ingroup iface_org_kde_plasma_window
 *
 * Removes the resource bound for this org_kde_plasma_window.
 */
static inline void
org_kde_plasma_window_destroy(struct org_kde_plasma_window *org_kde_plasma_window)
{
	wl_proxy_marshal_flags((struct wl_proxy *) org_kde_plasma_window,
			 ORG_KDE_PLASMA_WINDOW_DESTROY, NULL, wl_proxy_get_version((struct wl_proxy *) org_kde_plasma_window), WL_MARSHAL_FLAG_DESTROY);
}

/**
 * @ingroup iface_org_kde_plasma_window
 *
 * The compositor will write the window icon into the provided file descriptor.
 * The data is a serialized QIcon with QDataStream.
 */
static inline void
org_kde_plasma_window_get_icon(struct org_kde_plasma_window *org_kde_plasma_window, int32_t fd)
{
	wl_proxy_marshal_flags((struct wl_proxy *) org_kde_plasma_window,
			 ORG_KDE_PLASMA_WINDOW_GET_ICON, NULL, wl_proxy_get_version((struct wl_proxy *) org_kde_plasma_window), 0, fd);
}

/**
 * @ingroup iface_org_kde_plasma_window
 *
 * Make the window enter a virtual desktop. A window can enter more
 * than one virtual desktop. if the id is empty or invalid, no action will be performed.
 */
static inline void
org_kde_plasma_window_request_enter_virtual_desktop(struct org_kde_plasma_window *org_kde_plasma_window, const char *id)
{
	wl_proxy_marshal_flags((struct wl_proxy *) org_kde_plasma_window,
			 ORG_KDE_PLASMA_WINDOW_REQUEST_ENTER_VIRTUAL_DESKTOP, NULL, wl_proxy_get_version((struct wl_proxy *) org_kde_plasma_window), 0, id);
}

/**
 * @ingroup iface_org_kde_plasma_window
 * RFC: do this with an empty id to request_enter_virtual_desktop?
 * Make the window enter a new virtual desktop. If the server consents the request,
 * it will create a new virtual desktop and assign the window to it.
 */
static inline void
org_kde_plasma_window_request_enter_new_virtual_desktop(struct org_kde_plasma_window *org_kde_plasma_window)
{
	wl_proxy_marshal_flags((struct wl_proxy *) org_kde_plasma_window,
			 ORG_KDE_PLASMA_WINDOW_REQUEST_ENTER_NEW_VIRTUAL_DESKTOP, NULL, wl_proxy_get_version((struct wl_proxy *) org_kde_plasma_window), 0);
}

/**
 * @ingroup iface_org_kde_plasma_window
 *
 * Make the window exit a virtual desktop. If it exits all desktops it will be considered on all of them.
 */
static inline void
org_kde_plasma_window_request_leave_virtual_desktop(struct org_kde_plasma_window *org_kde_plasma_window, const char *id)
{
	wl_proxy_marshal_flags((struct wl_proxy *) org_kde_plasma_window,
			 ORG_KDE_PLASMA_WINDOW_REQUEST_LEAVE_VIRTUAL_DESKTOP, NULL, wl_proxy_get_version((struct wl_proxy *) org_kde_plasma_window), 0, id);
}

/**
 * @ingroup iface_org_kde_plasma_window
 *
 * Make the window enter an activity. A window can enter more activity. If the id is empty or invalid, no action will be performed.
 */
static inline void
org_kde_plasma_window_request_enter_activity(struct org_kde_plasma_window *org_kde_plasma_window, const char *id)
{
	wl_proxy_marshal_flags((struct wl_proxy *) org_kde_plasma_window,
			 ORG_KDE_PLASMA_WINDOW_REQUEST_ENTER_ACTIVITY, NULL, wl_proxy_get_version((struct wl_proxy *) org_kde_plasma_window), 0, id);
}

/**
 * @ingroup iface_org_kde_plasma_window
 *
 * Make the window exit a an activity. If it exits all activities it will be considered on all of them.
 */
static inline void
org_kde_plasma_window_request_leave_activity(struct org_kde_plasma_window *org_kde_plasma_window, const char *id)
{
	wl_proxy_marshal_flags((struct wl_proxy *) org_kde_plasma_window,
			 ORG_KDE_PLASMA_WINDOW_REQUEST_LEAVE_ACTIVITY, NULL, wl_proxy_get_version((struct wl_proxy *) org_kde_plasma_window), 0, id);
}

/**
 * @ingroup iface_org_kde_plasma_window
 *
 * Requests this window to be displayed in a specific output.
 */
static inline void
org_kde_plasma_window_send_to_output(struct org_kde_plasma_window *org_kde_plasma_window, struct wl_output *output)
{
	wl_proxy_marshal_flags((struct wl_proxy *) org_kde_plasma_window,
			 ORG_KDE_PLASMA_WINDOW_SEND_TO_OUTPUT, NULL, wl_proxy_get_version((struct wl_proxy *) org_kde_plasma_window), 0, output);
}

/**
 * @ingroup iface_org_kde_plasma_activation_feedback
 * @struct org_kde_plasma_activation_feedback_listener
 */
struct org_kde_plasma_activation_feedback_listener {
	/**
	 * notify that an app is starting
	 *
	 * Will be issued when an app is set to be activated. It offers
	 * an instance of org_kde_plasma_activation that will tell us the
	 * app_id and the extent of the activation.
	 */
	void (*activation)(void *data,
			   struct org_kde_plasma_activation_feedback *org_kde_plasma_activation_feedback,
			   struct org_kde_plasma_activation *id);
};

/**
 * @ingroup iface_org_kde_plasma_activation_feedback
 */
static inline int
org_kde_plasma_activation_feedback_add_listener(struct org_kde_plasma_activation_feedback *org_kde_plasma_activation_feedback,
						const struct org_kde_plasma_activation_feedback_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) org_kde_plasma_activation_feedback,
				     (void (**)(void)) listener, data);
}

#define ORG_KDE_PLASMA_ACTIVATION_FEEDBACK_DESTROY 0

/**
 * @ingroup iface_org_kde_plasma_activation_feedback
 */
#define ORG_KDE_PLASMA_ACTIVATION_FEEDBACK_ACTIVATION_SINCE_VERSION 1

/**
 * @ingroup iface_org_kde_plasma_activation_feedback
 */
#define ORG_KDE_PLASMA_ACTIVATION_FEEDBACK_DESTROY_SINCE_VERSION 1

/** @ingroup iface_org_kde_plasma_activation_feedback */
static inline void
org_kde_plasma_activation_feedback_set_user_data(struct org_kde_plasma_activation_feedback *org_kde_plasma_activation_feedback, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) org_kde_plasma_activation_feedback, user_data);
}

/** @ingroup iface_org_kde_plasma_activation_feedback */
static inline void *
org_kde_plasma_activation_feedback_get_user_data(struct org_kde_plasma_activation_feedback *org_kde_plasma_activation_feedback)
{
	return wl_proxy_get_user_data((struct wl_proxy *) org_kde_plasma_activation_feedback);
}

static inline uint32_t
org_kde_plasma_activation_feedback_get_version(struct org_kde_plasma_activation_feedback *org_kde_plasma_activation_feedback)
{
	return wl_proxy_get_version((struct wl_proxy *) org_kde_plasma_activation_feedback);
}

/**
 * @ingroup iface_org_kde_plasma_activation_feedback
 *
 * Destroy the activation manager object. The activation objects introduced
 * by this manager object will be unaffected.
 */
static inline void
org_kde_plasma_activation_feedback_destroy(struct org_kde_plasma_activation_feedback *org_kde_plasma_activation_feedback)
{
	wl_proxy_marshal_flags((struct wl_proxy *) org_kde_plasma_activation_feedback,
			 ORG_KDE_PLASMA_ACTIVATION_FEEDBACK_DESTROY, NULL, wl_proxy_get_version((struct wl_proxy *) org_kde_plasma_activation_feedback), WL_MARSHAL_FLAG_DESTROY);
}

/**
 * @ingroup iface_org_kde_plasma_activation
 * @struct org_kde_plasma_activation_listener
 */
struct org_kde_plasma_activation_listener {
	/**
	 * Offers the app_id
	 *
	 * 
	 * @param app_id application id, as described in xdg_activation_v1
	 */
	void (*app_id)(void *data,
		       struct org_kde_plasma_activation *org_kde_plasma_activation,
		       const char *app_id);
	/**
	 * Notifies about activation finished, either by activation or because it got invalidated
	 *
	 * 
	 */
	void (*finished)(void *data,
			 struct org_kde_plasma_activation *org_kde_plasma_activation);
};

/**
 * @ingroup iface_org_kde_plasma_activation
 */
static inline int
org_kde_plasma_activation_add_listener(struct org_kde_plasma_activation *org_kde_plasma_activation,
				       const struct org_kde_plasma_activation_listener *listener, void *data)
{
	return wl_proxy_add_listener((struct wl_proxy *) org_kde_plasma_activation,
				     (void (**)(void)) listener, data);
}

#define ORG_KDE_PLASMA_ACTIVATION_DESTROY 0

/**
 * @ingroup iface_org_kde_plasma_activation
 */
#define ORG_KDE_PLASMA_ACTIVATION_APP_ID_SINCE_VERSION 1
/**
 * @ingroup iface_org_kde_plasma_activation
 */
#define ORG_KDE_PLASMA_ACTIVATION_FINISHED_SINCE_VERSION 1

/**
 * @ingroup iface_org_kde_plasma_activation
 */
#define ORG_KDE_PLASMA_ACTIVATION_DESTROY_SINCE_VERSION 1

/** @ingroup iface_org_kde_plasma_activation */
static inline void
org_kde_plasma_activation_set_user_data(struct org_kde_plasma_activation *org_kde_plasma_activation, void *user_data)
{
	wl_proxy_set_user_data((struct wl_proxy *) org_kde_plasma_activation, user_data);
}

/** @ingroup iface_org_kde_plasma_activation */
static inline void *
org_kde_plasma_activation_get_user_data(struct org_kde_plasma_activation *org_kde_plasma_activation)
{
	return wl_proxy_get_user_data((struct wl_proxy *) org_kde_plasma_activation);
}

static inline uint32_t
org_kde_plasma_activation_get_version(struct org_kde_plasma_activation *org_kde_plasma_activation)
{
	return wl_proxy_get_version((struct wl_proxy *) org_kde_plasma_activation);
}

/**
 * @ingroup iface_org_kde_plasma_activation
 *
 * Notify the compositor that the org_kde_plasma_activation object will no
 * longer be used.
 */
static inline void
org_kde_plasma_activation_destroy(struct org_kde_plasma_activation *org_kde_plasma_activation)
{
	wl_proxy_marshal_flags((struct wl_proxy *) org_kde_plasma_activation,
			 ORG_KDE_PLASMA_ACTIVATION_DESTROY, NULL, wl_proxy_get_version((struct wl_proxy *) org_kde_plasma_activation), WL_MARSHAL_FLAG_DESTROY);
}

#ifdef  __cplusplus
}
#endif

#endif
