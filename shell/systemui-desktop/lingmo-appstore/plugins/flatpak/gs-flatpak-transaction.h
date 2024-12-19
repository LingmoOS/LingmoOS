/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2018 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <lingmo-appstore.h>
#include <flatpak.h>

G_BEGIN_DECLS

typedef enum {
	GS_FLATPAK_ERROR_MODE_IGNORE_ERRORS = 0,
	GS_FLATPAK_ERROR_MODE_STOP_ON_FIRST_ERROR = 1,
} GsFlatpakErrorMode;

#define GS_TYPE_FLATPAK_TRANSACTION (gs_flatpak_transaction_get_type ())

G_DECLARE_FINAL_TYPE (GsFlatpakTransaction, gs_flatpak_transaction, GS, FLATPAK_TRANSACTION, FlatpakTransaction)

FlatpakTransaction	*gs_flatpak_transaction_new		(FlatpakInstallation	*installation,
								 gboolean		 stop_on_first_error,
								 GCancellable		*cancellable,
								 GError			**error);
GsApp			*gs_flatpak_transaction_get_app_by_ref	(FlatpakTransaction	*transaction,
								 const gchar		*ref);
void			 gs_flatpak_transaction_add_app		(FlatpakTransaction	*transaction,
								 GsApp			*app);
gboolean		 gs_flatpak_transaction_run		(FlatpakTransaction	*transaction,
								 GCancellable		*cancellable,
								 GError			**error);

G_END_DECLS
