/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright © 2024 Joshua Lee <lee.son.wai@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <adwaita.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

/**
 * GsUpdatesPausedBannerFlags:
 * @GS_UPDATES_PAUSED_BANNER_FLAGS_NONE				No flags set
 * @GS_UPDATES_PAUSED_BANNER_FLAGS_METERED			Connection is metered
 * @GS_UPDATES_PAUSED_BANNER_FLAGS_NO_LARGE_DOWNLOADS		Connection prohibits large downloads
 * @GS_UPDATES_PAUSED_BANNER_FLAGS_POWER_SAVER			System is using Power Saver mode
 * @GS_UPDATES_PAUSED_BANNER_FLAGS_GAME_MODE			System is using GameMode
 *
 * The flags specifying the reason(s) automatic updates are paused.
 *
 * Since: 46
 **/

typedef enum {
	GS_UPDATES_PAUSED_BANNER_FLAGS_NONE			= 0,
	GS_UPDATES_PAUSED_BANNER_FLAGS_METERED			= 1 << 0,
	GS_UPDATES_PAUSED_BANNER_FLAGS_NO_LARGE_DOWNLOADS	= 1 << 1,
	GS_UPDATES_PAUSED_BANNER_FLAGS_POWER_SAVER		= 1 << 2,
	GS_UPDATES_PAUSED_BANNER_FLAGS_GAME_MODE		= 1 << 3,
} GsUpdatesPausedBannerFlags;

#define GS_TYPE_UPDATES_PAUSED_BANNER (gs_updates_paused_banner_get_type ())

G_DECLARE_FINAL_TYPE (GsUpdatesPausedBanner, gs_updates_paused_banner, GS, UPDATES_PAUSED_BANNER, AdwBin)

GtkWidget		*gs_updates_paused_banner_new			(void);

void			 gs_updates_paused_banner_set_flags		(GsUpdatesPausedBanner      *self,
									 GsUpdatesPausedBannerFlags  flags);

G_END_DECLS
