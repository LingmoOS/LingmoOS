/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2018 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2018 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <config.h>

#include "gs-flatpak-app.h"
#include "gs-flatpak-transaction.h"

struct _GsFlatpakTransaction {
	FlatpakTransaction	 parent_instance;
	GHashTable		*refhash;	/* ref:GsApp */
	GError			*first_operation_error;
	gboolean		 stop_on_first_error;
};

enum {
	SIGNAL_REF_TO_APP,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (GsFlatpakTransaction, gs_flatpak_transaction, FLATPAK_TYPE_TRANSACTION)

typedef enum {
	PROP_STOP_ON_FIRST_ERROR = 1,
} GsFlatpakTransactionProperty;

static GParamSpec *props[PROP_STOP_ON_FIRST_ERROR + 1] = { NULL, };

static void
gs_flatpak_transaction_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
	GsFlatpakTransaction *self = GS_FLATPAK_TRANSACTION (object);

	switch ((GsFlatpakTransactionProperty) prop_id) {
	case PROP_STOP_ON_FIRST_ERROR:
		g_value_set_boolean (value, self->stop_on_first_error);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_flatpak_transaction_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
	GsFlatpakTransaction *self = GS_FLATPAK_TRANSACTION (object);

	switch ((GsFlatpakTransactionProperty) prop_id) {
	case PROP_STOP_ON_FIRST_ERROR:
		/* Construct only. */
		self->stop_on_first_error = g_value_get_boolean (value);
		g_object_notify_by_pspec (object, props[prop_id]);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_flatpak_transaction_finalize (GObject *object)
{
	GsFlatpakTransaction *self;
	g_return_if_fail (GS_IS_FLATPAK_TRANSACTION (object));
	self = GS_FLATPAK_TRANSACTION (object);

	g_assert (self != NULL);
	g_hash_table_unref (self->refhash);
	if (self->first_operation_error != NULL)
		g_error_free (self->first_operation_error);

	G_OBJECT_CLASS (gs_flatpak_transaction_parent_class)->finalize (object);
}

GsApp *
gs_flatpak_transaction_get_app_by_ref (FlatpakTransaction *transaction, const gchar *ref)
{
	GsFlatpakTransaction *self = GS_FLATPAK_TRANSACTION (transaction);
	return g_hash_table_lookup (self->refhash, ref);
}

static void
gs_flatpak_transaction_add_app_internal (GsFlatpakTransaction *self, GsApp *app)
{
	g_autofree gchar *ref = gs_flatpak_app_get_ref_display (app);
	g_hash_table_insert (self->refhash, g_steal_pointer (&ref), g_object_ref (app));
}

void
gs_flatpak_transaction_add_app (FlatpakTransaction *transaction, GsApp *app)
{
	GsFlatpakTransaction *self = GS_FLATPAK_TRANSACTION (transaction);
	gs_flatpak_transaction_add_app_internal (self, app);
	if (gs_app_get_runtime (app) != NULL)
		gs_flatpak_transaction_add_app_internal (self, gs_app_get_runtime (app));
}

static GsApp *
_ref_to_app (GsFlatpakTransaction *self, const gchar *ref)
{
	GsApp *app = g_hash_table_lookup (self->refhash, ref);
	if (app != NULL)
		return g_object_ref (app);
	g_signal_emit (self, signals[SIGNAL_REF_TO_APP], 0, ref, &app);

	/* Cache the result */
	if (app != NULL)
		g_hash_table_insert (self->refhash, g_strdup (ref), g_object_ref (app));

	return app;
}

static void
_transaction_operation_set_app (FlatpakTransactionOperation *op, GsApp *app)
{
	g_object_set_data_full (G_OBJECT (op), "GsApp",
				g_object_ref (app), (GDestroyNotify) g_object_unref);
}

static GsApp *
_transaction_operation_get_app (FlatpakTransactionOperation *op)
{
	return g_object_get_data (G_OBJECT (op), "GsApp");
}

gboolean
gs_flatpak_transaction_run (FlatpakTransaction *transaction,
                            GCancellable *cancellable,
                            GError **error)

{
	GsFlatpakTransaction *self = GS_FLATPAK_TRANSACTION (transaction);
	g_autoptr(GError) error_local = NULL;

	if (!flatpak_transaction_run (transaction, cancellable, &error_local)) {
		/* whole transaction failed; restore the state for all the apps involved */
		g_autolist(GObject) ops = flatpak_transaction_get_operations (transaction);
		for (GList *l = ops; l != NULL; l = l->next) {
			FlatpakTransactionOperation *op = l->data;
			const gchar *ref = flatpak_transaction_operation_get_ref (op);
			g_autoptr(GsApp) app = _ref_to_app (self, ref);
			if (app == NULL) {
				g_warning ("failed to find app for %s", ref);
				continue;
			}
			gs_app_set_state_recover (app);
		}

		if (self->first_operation_error != NULL) {
			g_propagate_error (error, g_steal_pointer (&self->first_operation_error));
			return FALSE;
		} else {
			g_propagate_error (error, g_steal_pointer (&error_local));
			return FALSE;
		}
	}

	return TRUE;
}

static gboolean
_transaction_ready (FlatpakTransaction *transaction)
{
	GsFlatpakTransaction *self = GS_FLATPAK_TRANSACTION (transaction);
	g_autolist(GObject) ops = NULL;

	/* nothing to do */
	ops = flatpak_transaction_get_operations (transaction);
	if (ops == NULL)
		return TRUE; // FIXME: error?
	for (GList *l = ops; l != NULL; l = l->next) {
		FlatpakTransactionOperation *op = l->data;
		const gchar *ref = flatpak_transaction_operation_get_ref (op);
		g_autoptr(GsApp) app = _ref_to_app (self, ref);
		if (app != NULL) {
			_transaction_operation_set_app (op, app);
			/* if we're updating a component, then mark all the apps
			 * involved to ensure updating the button state */
			if (flatpak_transaction_operation_get_operation_type (op) == FLATPAK_TRANSACTION_OPERATION_UPDATE) {
				if (gs_app_get_state (app) == GS_APP_STATE_UNKNOWN ||
				    gs_app_get_state (app) == GS_APP_STATE_INSTALLED)
					gs_app_set_state (app, GS_APP_STATE_UPDATABLE_LIVE);

				gs_app_set_state (app, GS_APP_STATE_INSTALLING);
			}
		}

		/* Debug dump. */
		{
			GPtrArray *related_to_ops = flatpak_transaction_operation_get_related_to_ops (op);
			g_autoptr(GString) debug_message = g_string_new ("");

			g_string_append_printf (debug_message,
						"%s: op %p, app %s (%p), download size %" G_GUINT64_FORMAT ", related-to:",
						G_STRFUNC, op,
						app ? gs_app_get_unique_id (app) : "?",
						app,
						flatpak_transaction_operation_get_download_size (op));
			for (gsize i = 0; related_to_ops != NULL && i < related_to_ops->len; i++) {
				FlatpakTransactionOperation *related_to_op = g_ptr_array_index (related_to_ops, i);
				g_string_append_printf (debug_message,
							"\n ├ %s (%p)", flatpak_transaction_operation_get_ref (related_to_op), related_to_op);
			}
			g_string_append (debug_message, "\n └ (end)");
			g_debug ("%s", debug_message->str);
		}
	}
	return TRUE;
}

typedef struct
{
	GsFlatpakTransaction *transaction;  /* (owned) */
	FlatpakTransactionOperation *operation;  /* (owned) */
	GsApp *app;  /* (owned) */
} ProgressData;

static void
progress_data_free (ProgressData *data)
{
	g_clear_object (&data->operation);
	g_clear_object (&data->app);
	g_clear_object (&data->transaction);
	g_free (data);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (ProgressData, progress_data_free)

static gboolean
op_is_related_to_op (FlatpakTransactionOperation *op,
                     FlatpakTransactionOperation *root_op)
{
	GPtrArray *related_to_ops;  /* (element-type FlatpakTransactionOperation) */

	if (op == root_op)
		return TRUE;

	related_to_ops = flatpak_transaction_operation_get_related_to_ops (op);
	for (gsize i = 0; related_to_ops != NULL && i < related_to_ops->len; i++) {
		FlatpakTransactionOperation *related_to_op = g_ptr_array_index (related_to_ops, i);
		if (related_to_op == root_op || op_is_related_to_op (related_to_op, root_op))
			return TRUE;
	}

	return FALSE;
}

static guint64
saturated_uint64_add (guint64 a, guint64 b)
{
	return (a <= G_MAXUINT64 - b) ? a + b : G_MAXUINT64;
}

/*
 * update_progress_for_op:
 * @self: a #GsFlatpakTransaction
 * @current_progress: progress reporting object
 * @ops: results of calling flatpak_transaction_get_operations() on @self, for performance
 * @current_op: the #FlatpakTransactionOperation which the @current_progress is
 *    for; this is the operation currently being run by libflatpak
 * @root_op: the #FlatpakTransactionOperation at the root of the operation subtree
 *    to calculate progress for
 *
 * Calculate and update the #GsApp:progress for each app associated with
 * @root_op in a flatpak transaction. This will include the #GsApp for the app
 * being installed (for example), but also the #GsApps for all of its runtimes
 * and locales, and any other dependencies of them.
 *
 * Each #GsApp:progress is calculated based on the sum of the progress of all
 * the apps related to that one — so the progress for an app will factor in the
 * progress for all its runtimes.
 */
static void
update_progress_for_op (GsFlatpakTransaction        *self,
                        FlatpakTransactionProgress  *current_progress,
                        GList                       *ops,
                        FlatpakTransactionOperation *current_op,
                        FlatpakTransactionOperation *root_op)
{
	g_autoptr(GsApp) root_app = NULL;
	guint64 related_prior_download_bytes = 0;
	guint64 related_download_bytes = 0;
	guint64 current_bytes_transferred = flatpak_transaction_progress_get_bytes_transferred (current_progress);
	gboolean seen_current_op = FALSE, seen_root_op = FALSE;
	gboolean root_op_skipped = flatpak_transaction_operation_get_is_skipped (root_op);
	guint percent;

	/* If @root_op is being skipped and its GsApp isn't being
	 * installed/removed, don't update the progress on it. It may be that
	 * @root_op is the runtime of an app and the app is the thing the
	 * transaction was created for.
	 */
	if (root_op_skipped) {
		/* _transaction_operation_set_app() is only called on non-skipped ops */
		const gchar *ref = flatpak_transaction_operation_get_ref (root_op);
		root_app = _ref_to_app (self, ref);
		if (root_app == NULL) {
			g_warning ("Couldn't find GsApp for transaction operation %s",
			           flatpak_transaction_operation_get_ref (root_op));
			return;
		}
		if (gs_app_get_state (root_app) != GS_APP_STATE_INSTALLING &&
		    gs_app_get_state (root_app) != GS_APP_STATE_REMOVING &&
		    gs_app_get_state (root_app) != GS_APP_STATE_DOWNLOADING)
			return;
	} else {
		GsApp *unskipped_root_app = _transaction_operation_get_app (root_op);
		if (unskipped_root_app == NULL) {
			g_warning ("Couldn't find GsApp for transaction operation %s",
			           flatpak_transaction_operation_get_ref (root_op));
			return;
		}
		root_app = g_object_ref (unskipped_root_app);
	}

	/* This relies on ops in a #FlatpakTransaction being run in the order
	 * they’re returned by flatpak_transaction_get_operations(), which is true. */
	for (GList *l = ops; l != NULL; l = l->next) {
		FlatpakTransactionOperation *op = FLATPAK_TRANSACTION_OPERATION (l->data);
		guint64 op_download_size = flatpak_transaction_operation_get_download_size (op);

		if (op == current_op)
			seen_current_op = TRUE;
		if (op == root_op)
			seen_root_op = TRUE;

		/* Currently libflatpak doesn't return skipped ops in
		 * flatpak_transaction_get_operations(), but check just in case.
		 */
		if (op == root_op && root_op_skipped)
			continue;

		if (op_is_related_to_op (op, root_op)) {
			/* Saturate instead of overflowing */
			related_download_bytes = saturated_uint64_add (related_download_bytes, op_download_size);
			if (!seen_current_op)
				related_prior_download_bytes = saturated_uint64_add (related_prior_download_bytes, op_download_size);
		}
	}

	g_assert (related_prior_download_bytes <= related_download_bytes);
	g_assert (seen_root_op || root_op_skipped);

	/* Avoid overflows when converting to percent, at the cost of losing
	 * some precision in the least significant digits. */
	if (related_prior_download_bytes > G_MAXUINT64 / 100 ||
	    current_bytes_transferred > G_MAXUINT64 / 100) {
		related_prior_download_bytes /= 100;
		    current_bytes_transferred /= 100;
		    related_download_bytes /= 100;
	}

	/* Update the progress of @root_app. */
	if (related_download_bytes > 0)
		percent = ((related_prior_download_bytes * 100 / related_download_bytes) +
		           (current_bytes_transferred * 100 / related_download_bytes));
	else
		percent = 0;

	if (gs_app_get_progress (root_app) == 100 ||
	    gs_app_get_progress (root_app) == GS_APP_PROGRESS_UNKNOWN ||
	    gs_app_get_progress (root_app) <= percent) {
		gs_app_set_progress (root_app, percent);
	} else {
		g_warning ("ignoring percentage %u%% -> %u%% as going down on app %s",
			   gs_app_get_progress (root_app), percent,
			   gs_app_get_unique_id (root_app));
	}
}

static void
update_progress_for_op_recurse_up (GsFlatpakTransaction        *self,
				   FlatpakTransactionProgress  *progress,
				   GList                       *ops,
				   FlatpakTransactionOperation *current_op,
				   FlatpakTransactionOperation *root_op)
{
	GPtrArray *related_to_ops = flatpak_transaction_operation_get_related_to_ops (root_op);

	/* Update progress for @root_op */
	update_progress_for_op (self, progress, ops, current_op, root_op);

	/* Update progress for ops related to @root_op, e.g. apps whose runtime is @root_op */
	for (gsize i = 0; related_to_ops != NULL && i < related_to_ops->len; i++) {
		FlatpakTransactionOperation *related_to_op = g_ptr_array_index (related_to_ops, i);
		update_progress_for_op_recurse_up (self, progress, ops, current_op, related_to_op);
	}
}

static void
_transaction_progress_changed_cb (FlatpakTransactionProgress *progress,
				  gpointer user_data)
{
	ProgressData *data = user_data;
	GsApp *app = data->app;
	GsFlatpakTransaction *self = data->transaction;
	g_autolist(FlatpakTransactionOperation) ops = NULL;

	if (flatpak_transaction_progress_get_is_estimating (progress)) {
		/* "Estimating" happens while fetching the metadata, which
		 * flatpak arbitrarily decides happens during the first 5% of
		 * each operation. At this point, no more detailed progress
		 * information is available. */
		gs_app_set_progress (app, GS_APP_PROGRESS_UNKNOWN);
		return;
	}

	/* Update the progress on this app, and then do the same for each
	 * related parent app up the hierarchy. For example, @data->operation
	 * could be for a runtime which was added to the transaction because of
	 * an app — so we need to update the progress on the app too.
	 *
	 * It’s important to note that a new @data->progress is created by
	 * libflatpak for each @data->operation, and there are multiple
	 * operations in a transaction. There is no #FlatpakTransactionProgress
	 * which represents the progress of the whole transaction.
	 *
	 * There may be arbitrary many levels of related-to ops. For example,
	 * one common situation would be to install an app which needs a new
	 * runtime, and that runtime needs a locale to be installed, which would
	 * give three levels of related-to relation:
	 *    locale → runtime → app → (null)
	 *
	 * In addition, libflatpak may decide to skip some operations (if they
	 * turn out to not be necessary). These skipped operations are not
	 * included in the list returned by flatpak_transaction_get_operations(),
	 * but they can be accessed via
	 * flatpak_transaction_operation_get_related_to_ops(), so have to be
	 * ignored manually.
	 */
	ops = flatpak_transaction_get_operations (FLATPAK_TRANSACTION (self));
	update_progress_for_op_recurse_up (self, progress, ops, data->operation, data->operation);
}

static const gchar *
_flatpak_transaction_operation_type_to_string (FlatpakTransactionOperationType ot)
{
	if (ot == FLATPAK_TRANSACTION_OPERATION_INSTALL)
		return "install";
	if (ot == FLATPAK_TRANSACTION_OPERATION_UPDATE)
		return "update";
	if (ot == FLATPAK_TRANSACTION_OPERATION_INSTALL_BUNDLE)
		return "install-bundle";
	if (ot == FLATPAK_TRANSACTION_OPERATION_UNINSTALL)
		return "uninstall";
	return NULL;
}

static void
progress_data_free_closure (gpointer  user_data,
                            GClosure *closure)
{
	progress_data_free (user_data);
}

static void
_transaction_new_operation (FlatpakTransaction *transaction,
			    FlatpakTransactionOperation *operation,
			    FlatpakTransactionProgress *progress)
{
	GsApp *app;
	g_autoptr(ProgressData) progress_data = NULL;

	/* find app */
	app = _transaction_operation_get_app (operation);
	if (app == NULL) {
		FlatpakTransactionOperationType ot;
		ot = flatpak_transaction_operation_get_operation_type (operation);
		g_warning ("failed to find app for %s during %s",
			   flatpak_transaction_operation_get_ref (operation),
			   _flatpak_transaction_operation_type_to_string (ot));
		return;
	}

	/* report progress */
	progress_data = g_new0 (ProgressData, 1);
	progress_data->transaction = GS_FLATPAK_TRANSACTION (g_object_ref (transaction));
	progress_data->app = g_object_ref (app);
	progress_data->operation = g_object_ref (operation);

	g_signal_connect_data (progress, "changed",
			       G_CALLBACK (_transaction_progress_changed_cb),
			       g_steal_pointer (&progress_data),
			       progress_data_free_closure,
			       0  /* flags */);
	flatpak_transaction_progress_set_update_frequency (progress, 500); /* FIXME? */

	/* set app status */
	switch (flatpak_transaction_operation_get_operation_type (operation)) {
	case FLATPAK_TRANSACTION_OPERATION_INSTALL:
		if (gs_app_get_state (app) == GS_APP_STATE_UNKNOWN)
			gs_app_set_state (app, GS_APP_STATE_AVAILABLE);
		gs_app_set_state (app, GS_APP_STATE_INSTALLING);
		break;
	case FLATPAK_TRANSACTION_OPERATION_INSTALL_BUNDLE:
		if (gs_app_get_state (app) == GS_APP_STATE_UNKNOWN)
			gs_app_set_state (app, GS_APP_STATE_AVAILABLE_LOCAL);
		gs_app_set_state (app, GS_APP_STATE_INSTALLING);
		break;
	case FLATPAK_TRANSACTION_OPERATION_UPDATE:
		if (gs_app_get_state (app) == GS_APP_STATE_UNKNOWN ||
		    gs_app_get_state (app) == GS_APP_STATE_INSTALLED)
			gs_app_set_state (app, GS_APP_STATE_UPDATABLE_LIVE);
		gs_app_set_state (app, GS_APP_STATE_INSTALLING);
		break;
	case FLATPAK_TRANSACTION_OPERATION_UNINSTALL:
		gs_app_set_state (app, GS_APP_STATE_REMOVING);
		break;
	default:
		break;
	}
}

static gboolean
later_op_also_related (GList                       *ops,
		       FlatpakTransactionOperation *current_op,
		       FlatpakTransactionOperation *related_to_current_op)
{
	/* Here we're determining if anything in @ops which comes after
	 * @current_op is related to @related_to_current_op and not skipped
	 * (but all @ops are not skipped so no need to check explicitly)
	 */
	gboolean found_later_op = FALSE, seen_current_op = FALSE;
	for (GList *l = ops; l != NULL; l = l->next) {
		FlatpakTransactionOperation *op = l->data;
		GPtrArray *related_to_ops;
		if (current_op == op) {
			seen_current_op = TRUE;
			continue;
		}
		if (!seen_current_op)
			continue;

		related_to_ops = flatpak_transaction_operation_get_related_to_ops (op);
		for (gsize i = 0; related_to_ops != NULL && i < related_to_ops->len; i++) {
			FlatpakTransactionOperation *related_to_op = g_ptr_array_index (related_to_ops, i);
			if (related_to_op == related_to_current_op) {
				g_assert (flatpak_transaction_operation_get_is_skipped (related_to_op));
				found_later_op = TRUE;
			}
		}
	}

	return found_later_op;
}

static void
set_skipped_related_apps_to_installed (GsFlatpakTransaction        *self,
				       FlatpakTransaction          *transaction,
				       FlatpakTransactionOperation *operation)
{
	/* It's possible the thing being updated/installed, @operation, is a
	 * related ref (e.g. extension or runtime) of an app which itself doesn't
	 * need an update and therefore won't have _transaction_operation_done()
	 * called for it directly. So we have to set the main app to installed
	 * here.
	*/
	g_autolist(GObject) ops = flatpak_transaction_get_operations (transaction);
	GPtrArray *related_to_ops = flatpak_transaction_operation_get_related_to_ops (operation);

	for (gsize i = 0; related_to_ops != NULL && i < related_to_ops->len; i++) {
		FlatpakTransactionOperation *related_to_op = g_ptr_array_index (related_to_ops, i);
		if (flatpak_transaction_operation_get_is_skipped (related_to_op)) {
			const gchar *ref;
			g_autoptr(GsApp) related_to_app = NULL;

			/* Check that no later op is also related to related_to_op, in
			 * which case we want to let that operation finish before setting
			 * the main app to installed.
			 */
			if (later_op_also_related (ops, operation, related_to_op))
				continue;

			ref = flatpak_transaction_operation_get_ref (related_to_op);
			related_to_app = _ref_to_app (self, ref);
			if (related_to_app != NULL)
				gs_app_set_state (related_to_app, GS_APP_STATE_INSTALLED);
		}
	}
}

static void
_transaction_operation_done (FlatpakTransaction *transaction,
			     FlatpakTransactionOperation *operation,
			     const gchar *commit,
			     FlatpakTransactionResult details)
{
	GsFlatpakTransaction *self = GS_FLATPAK_TRANSACTION (transaction);

	/* invalidate */
	GsApp *app = _transaction_operation_get_app (operation);
	if (app == NULL) {
		g_warning ("failed to find app for %s",
			   flatpak_transaction_operation_get_ref (operation));
		return;
	}
	switch (flatpak_transaction_operation_get_operation_type (operation)) {
	case FLATPAK_TRANSACTION_OPERATION_INSTALL:
	case FLATPAK_TRANSACTION_OPERATION_INSTALL_BUNDLE:
		gs_app_set_state (app, GS_APP_STATE_INSTALLED);

		set_skipped_related_apps_to_installed (self, transaction, operation);
		break;
	case FLATPAK_TRANSACTION_OPERATION_UPDATE:
		gs_app_set_version (app, gs_app_get_update_version (app));
		gs_app_set_update_details_markup (app, NULL);
		gs_app_set_update_urgency (app, AS_URGENCY_KIND_UNKNOWN);
		gs_app_set_update_version (app, NULL);
		/* force getting the new runtime */
		gs_app_remove_kudo (app, GS_APP_KUDO_SANDBOXED);
                /* downloaded, but not yet installed */
		if (flatpak_transaction_get_no_deploy (transaction))
			gs_app_set_state (app, GS_APP_STATE_UPDATABLE_LIVE);
		else
			gs_app_set_state (app, GS_APP_STATE_INSTALLED);

		set_skipped_related_apps_to_installed (self, transaction, operation);
		break;
	case FLATPAK_TRANSACTION_OPERATION_UNINSTALL:
		/* we don't actually know if this app is re-installable */
		gs_flatpak_app_set_commit (app, NULL);
		gs_app_set_state (app, GS_APP_STATE_UNKNOWN);
		break;
	default:
		gs_app_set_state (app, GS_APP_STATE_UNKNOWN);
		break;
	}
}

static gboolean
_transaction_operation_error (FlatpakTransaction *transaction,
			      FlatpakTransactionOperation *operation,
			      const GError *error,
			      FlatpakTransactionErrorDetails detail)
{
	GsFlatpakTransaction *self = GS_FLATPAK_TRANSACTION (transaction);
	FlatpakTransactionOperationType operation_type = flatpak_transaction_operation_get_operation_type (operation);
	GsApp *app = _transaction_operation_get_app (operation);
	const gchar *ref = flatpak_transaction_operation_get_ref (operation);

	gs_app_set_state_recover (app);

	if (g_error_matches (error, FLATPAK_ERROR, FLATPAK_ERROR_SKIPPED)) {
		g_debug ("skipped to %s %s: %s",
		         _flatpak_transaction_operation_type_to_string (operation_type),
		         ref,
		         error->message);
		return TRUE; /* continue */
	}

	/* If the transaction has been cancelled, bail out early rather
	 * than continuing to try operations which are all cancelled. */
	if (g_error_matches (error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
		g_debug ("Transaction cancelled; stopping it");

		return FALSE;  /* stop */
	}

	if (detail & FLATPAK_TRANSACTION_ERROR_DETAILS_NON_FATAL) {
		g_warning ("failed to %s %s (non fatal): %s",
		           _flatpak_transaction_operation_type_to_string (operation_type),
		           ref,
		           error->message);
		return TRUE; /* continue */
	}

	if (self->first_operation_error == NULL) {
		g_propagate_error (&self->first_operation_error,
		                   g_error_copy (error));
		if (app != NULL)
			gs_utils_error_add_app_id (&self->first_operation_error, app);
	}

	if (!(detail & FLATPAK_TRANSACTION_ERROR_DETAILS_NON_FATAL) &&
	    self->stop_on_first_error)
		return FALSE;  /* stop */

	return TRUE; /* continue */
}

static int
_transaction_choose_remote_for_ref (FlatpakTransaction *transaction,
				    const char *for_ref,
				    const char *runtime_ref,
				    const char * const *remotes)
{
	//FIXME: do something smarter
	return 0;
}

static void
_transaction_end_of_lifed (FlatpakTransaction *transaction,
			   const gchar *ref,
			   const gchar *reason,
			   const gchar *rebase)
{
	if (rebase) {
		g_message ("%s is end-of-life, in favor of %s", ref, rebase);
	} else if (reason) {
		g_message ("%s is end-of-life, with reason: %s", ref, reason);
	}
	//FIXME: show something in the UI
}

static gboolean
_transaction_end_of_lifed_with_rebase (FlatpakTransaction  *transaction,
				       const gchar         *remote,
				       const gchar         *ref,
				       const gchar         *reason,
				       const gchar         *rebased_to_ref,
				       const gchar        **previous_ids)
{
	GsFlatpakTransaction *self = GS_FLATPAK_TRANSACTION (transaction);

	if (rebased_to_ref) {
		g_message ("%s is end-of-life, in favor of %s", ref, rebased_to_ref);
	} else if (reason) {
		g_message ("%s is end-of-life, with reason: %s", ref, reason);
	}

	if (rebased_to_ref && remote) {
		g_autoptr(GError) local_error = NULL;

#if FLATPAK_CHECK_VERSION(1, 15, 6)
		if (!flatpak_transaction_add_rebase_and_uninstall (transaction, remote, rebased_to_ref, ref,
								   NULL, previous_ids, &local_error)) {
#else
		if (!flatpak_transaction_add_rebase (transaction, remote, rebased_to_ref,
						     NULL, previous_ids, &local_error) ||
		    !flatpak_transaction_add_uninstall (transaction, ref, &local_error)) {
			/* NOT_INSTALLED error is expected in case the op that triggered this was install not update */
			if (g_error_matches (local_error, FLATPAK_ERROR, FLATPAK_ERROR_NOT_INSTALLED))
				g_clear_error (&local_error);
			else
#endif
			if (self->first_operation_error == NULL)
				g_propagate_prefixed_error (&self->first_operation_error,
							    g_steal_pointer (&local_error),
							    "Failed to rebase %s to %s: ",
							    ref, rebased_to_ref);

			return FALSE;
		}

		/* Note: A message about the rename will be shown in the UI
		 * thanks to code in gs_flatpak_refine_appstream() which
		 * sets gs_app_set_renamed_from().
		 */
		return TRUE;
	}

	return FALSE;
}

static gboolean
_transaction_add_new_remote (FlatpakTransaction *transaction,
			     FlatpakTransactionRemoteReason reason,
			     const char *from_id,
			     const char *remote_name,
			     const char *url)
{
	/* additional apps */
	if (reason == FLATPAK_TRANSACTION_REMOTE_GENERIC_REPO) {
		g_debug ("configuring %s as new generic remote", url);
		return TRUE; //FIXME?
	}

	/* runtime deps always make sense */
	if (reason == FLATPAK_TRANSACTION_REMOTE_RUNTIME_DEPS) {
		g_debug ("configuring %s as new remote for deps", url);
		return TRUE;
	}

	return FALSE;
}

static void
gs_flatpak_transaction_class_init (GsFlatpakTransactionClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	FlatpakTransactionClass *transaction_class = FLATPAK_TRANSACTION_CLASS (klass);

	object_class->get_property = gs_flatpak_transaction_get_property;
	object_class->set_property = gs_flatpak_transaction_set_property;
	object_class->finalize = gs_flatpak_transaction_finalize;

	transaction_class->ready = _transaction_ready;
	transaction_class->add_new_remote = _transaction_add_new_remote;
	transaction_class->new_operation = _transaction_new_operation;
	transaction_class->operation_done = _transaction_operation_done;
	transaction_class->operation_error = _transaction_operation_error;
	transaction_class->choose_remote_for_ref = _transaction_choose_remote_for_ref;
	transaction_class->end_of_lifed = _transaction_end_of_lifed;
	transaction_class->end_of_lifed_with_rebase = _transaction_end_of_lifed_with_rebase;

	/**
	 * GsFlatpakTransaction:stop-on-first-error:
	 *
	 * Stop the transaction on the first fatal error. If %FALSE, the
	 * transaction will continue running and ignore subsequent errors. Some
	 * operations may be automatically skipped if they are related to
	 * operations which have errored.
	 *
	 * Typically this should be %TRUE. It may be %FALSE for transactions
	 * where lots of apps are being updated, as typically updates should be
	 * mostly independent of each other, and we want as many of them to
	 * be attempted as possible.
	 *
	 * Since: 44
	 */
	props[PROP_STOP_ON_FIRST_ERROR] =
		g_param_spec_boolean ("stop-on-first-error",
				      "Stop on First Error",
				      "Stop the transaction on the first fatal error.",
				      TRUE,
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				      G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (props), props);

	signals[SIGNAL_REF_TO_APP] =
		g_signal_new ("ref-to-app",
			      G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
			      0, NULL, NULL, NULL, G_TYPE_OBJECT, 1, G_TYPE_STRING);
}

static void
gs_flatpak_transaction_init (GsFlatpakTransaction *self)
{
	self->refhash = g_hash_table_new_full (g_str_hash, g_str_equal,
					       g_free, (GDestroyNotify) g_object_unref);
	self->stop_on_first_error = TRUE;
}

FlatpakTransaction *
gs_flatpak_transaction_new (FlatpakInstallation	*installation,
			    gboolean stop_on_first_error,
			    GCancellable *cancellable,
			    GError **error)
{
	GsFlatpakTransaction *self;
	self = g_initable_new (GS_TYPE_FLATPAK_TRANSACTION,
			       cancellable, error,
			       "installation", installation,
			       "stop-on-first-error", stop_on_first_error,
			       NULL);
	if (self == NULL)
		return NULL;
	return FLATPAK_TRANSACTION (self);
}
