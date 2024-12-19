/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2022 Red Hat <www.redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

/**
 * GsAppPermissionsFlags:
 * @GS_APP_PERMISSIONS_FLAGS_DEVICES: App can access general purpose devices
 *   such as webcams or gaming controllers.
 * @GS_APP_PERMISSIONS_FLAGS_SYSTEM_DEVICES: App can access non-physical
 *   privileged system devices, such as `/dev/shm` or `/dev/kvm` (Since: 44)
 * @GS_APP_PERMISSIONS_FLAGS_SCREEN: App can access screen contents
 *   without asking, e.g. by reading Pipewire ScreenCast streams (Since: 46)
 * @GS_APP_PERMISSIONS_FLAGS_INPUT_DEVICES: App can access input devices, under `/dev/input` (Since: 46)
 * @GS_APP_PERMISSIONS_FLAGS_PULSEAUDIO_DEVICES: App can access PulseAudio devices (Since: 46)
 *
 * Flags to indicate what permissions an app requires, at a high level.
 */
typedef enum {
	GS_APP_PERMISSIONS_FLAGS_NONE			= 0,
	GS_APP_PERMISSIONS_FLAGS_NETWORK 		= 1 << 1,
	GS_APP_PERMISSIONS_FLAGS_SYSTEM_BUS   		= 1 << 2,
	GS_APP_PERMISSIONS_FLAGS_SESSION_BUS		= 1 << 3,
	GS_APP_PERMISSIONS_FLAGS_DEVICES 		= 1 << 4,
	GS_APP_PERMISSIONS_FLAGS_HOME_FULL 		= 1 << 5,
	GS_APP_PERMISSIONS_FLAGS_HOME_READ		= 1 << 6,
	GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_FULL	= 1 << 7,
	GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_READ	= 1 << 8,
	GS_APP_PERMISSIONS_FLAGS_DOWNLOADS_FULL 	= 1 << 9,
	GS_APP_PERMISSIONS_FLAGS_DOWNLOADS_READ		= 1 << 10,
	GS_APP_PERMISSIONS_FLAGS_SETTINGS		= 1 << 11,
	GS_APP_PERMISSIONS_FLAGS_X11			= 1 << 12,
	GS_APP_PERMISSIONS_FLAGS_ESCAPE_SANDBOX		= 1 << 13,
	GS_APP_PERMISSIONS_FLAGS_FILESYSTEM_OTHER	= 1 << 14,
	GS_APP_PERMISSIONS_FLAGS_SYSTEM_DEVICES		= 1 << 15,
	GS_APP_PERMISSIONS_FLAGS_SCREEN			= 1 << 16,
	GS_APP_PERMISSIONS_FLAGS_INPUT_DEVICES		= 1 << 17,
	GS_APP_PERMISSIONS_FLAGS_PULSEAUDIO_DEVICES	= 1 << 18,
	GS_APP_PERMISSIONS_FLAGS_LAST  /*< skip >*/
} GsAppPermissionsFlags;

#define LIMITED_PERMISSIONS (GS_APP_PERMISSIONS_FLAGS_SETTINGS | \
			GS_APP_PERMISSIONS_FLAGS_NETWORK | \
			GS_APP_PERMISSIONS_FLAGS_DOWNLOADS_READ | \
			GS_APP_PERMISSIONS_FLAGS_DOWNLOADS_FULL)
#define MEDIUM_PERMISSIONS (LIMITED_PERMISSIONS | \
			GS_APP_PERMISSIONS_FLAGS_X11)

#define GS_TYPE_APP_PERMISSIONS (gs_app_permissions_get_type ())

G_DECLARE_FINAL_TYPE (GsAppPermissions, gs_app_permissions, GS, APP_PERMISSIONS, GObject)

GsAppPermissions	*gs_app_permissions_new		(void);
void			 gs_app_permissions_seal	(GsAppPermissions	*self);
gboolean		 gs_app_permissions_is_sealed	(GsAppPermissions	*self);
void			 gs_app_permissions_set_flags	(GsAppPermissions	*self,
							 GsAppPermissionsFlags	 flags);
GsAppPermissionsFlags	 gs_app_permissions_get_flags	(GsAppPermissions	*self);
void			 gs_app_permissions_add_flag	(GsAppPermissions	*self,
							 GsAppPermissionsFlags	 flags);
void			 gs_app_permissions_remove_flag	(GsAppPermissions	*self,
							 GsAppPermissionsFlags	 flags);
void			 gs_app_permissions_add_filesystem_read
							(GsAppPermissions	*self,
							 const gchar		*filename);
const GPtrArray		*gs_app_permissions_get_filesystem_read
							(GsAppPermissions	*self);
gboolean		 gs_app_permissions_contains_filesystem_read
							(GsAppPermissions *self,
							 const gchar *filename);
void			 gs_app_permissions_add_filesystem_full
							(GsAppPermissions	*self,
							 const gchar		*filename);
const GPtrArray		*gs_app_permissions_get_filesystem_full
							(GsAppPermissions	*self);
gboolean		 gs_app_permissions_contains_filesystem_full
							(GsAppPermissions *self,
							 const gchar *filename);

G_END_DECLS
