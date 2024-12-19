/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Red Hat <www.redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib.h>
#include <gio/gio.h>

#include "gs-plugin-loader.h"

G_BEGIN_DECLS

#define GS_TYPE_FEDORA_THIRD_PARTY (gs_fedora_third_party_get_type ())

G_DECLARE_FINAL_TYPE (GsFedoraThirdParty, gs_fedora_third_party, GS, FEDORA_THIRD_PARTY, GObject)

typedef enum _GsFedoraThirdPartyState {
	GS_FEDORA_THIRD_PARTY_STATE_UNKNOWN,
	GS_FEDORA_THIRD_PARTY_STATE_ENABLED,
	GS_FEDORA_THIRD_PARTY_STATE_DISABLED,
	GS_FEDORA_THIRD_PARTY_STATE_ASK
} GsFedoraThirdPartyState;

GsFedoraThirdParty *
		gs_fedora_third_party_new	(GsPluginLoader		*plugin_loader);
gboolean	gs_fedora_third_party_is_available
						(GsFedoraThirdParty	*self);
void		gs_fedora_third_party_invalidate(GsFedoraThirdParty	*self);
void		gs_fedora_third_party_query	(GsFedoraThirdParty	*self,
						 GCancellable		*cancellable,
						 GAsyncReadyCallback	 callback,
						 gpointer		 user_data);
gboolean	gs_fedora_third_party_query_finish
						(GsFedoraThirdParty	*self,
						 GAsyncResult		*result,
						 GsFedoraThirdPartyState *out_state,
						 GError			**error);
gboolean	gs_fedora_third_party_query_sync(GsFedoraThirdParty	*self,
						 GsFedoraThirdPartyState *out_state,
						 GCancellable		*cancellable,
						 GError			**error);
void		gs_fedora_third_party_switch	(GsFedoraThirdParty	*self,
						 gboolean		 enable,
						 gboolean		 config_only,
						 GCancellable		*cancellable,
						 GAsyncReadyCallback	 callback,
						 gpointer		 user_data);
gboolean	gs_fedora_third_party_switch_finish
						(GsFedoraThirdParty	*self,
						 GAsyncResult		*result,
						 GError			**error);
gboolean	gs_fedora_third_party_switch_sync
						(GsFedoraThirdParty	*self,
						 gboolean		 enable,
						 gboolean		 config_only,
						 GCancellable		*cancellable,
						 GError			**error);
void		gs_fedora_third_party_opt_out	(GsFedoraThirdParty	*self,
						 GCancellable		*cancellable,
						 GAsyncReadyCallback	 callback,
						 gpointer		 user_data);
gboolean	gs_fedora_third_party_opt_out_finish
						(GsFedoraThirdParty	*self,
						 GAsyncResult		*result,
						 GError			**error);
gboolean	gs_fedora_third_party_opt_out_sync
						(GsFedoraThirdParty	*self,
						 GCancellable		*cancellable,
						 GError			**error);
void		gs_fedora_third_party_list	(GsFedoraThirdParty	*self,
						 GCancellable		*cancellable,
						 GAsyncReadyCallback	 callback,
						 gpointer		 user_data);
gboolean	gs_fedora_third_party_list_finish
						(GsFedoraThirdParty	*self,
						 GAsyncResult		*result,
						 GHashTable		**out_repos,
						 GError			**error);
gboolean	gs_fedora_third_party_list_sync	(GsFedoraThirdParty	*self,
						 GHashTable		**out_repos,
						 GCancellable		*cancellable,
						 GError			**error);

/* Utility functions */
gboolean	gs_fedora_third_party_util_is_third_party_repo
						(GHashTable		*third_party_repos,
						 const gchar		*origin,
						 const gchar		*management_plugin);

G_END_DECLS
