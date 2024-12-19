/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2015-2018 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <glib/gi18n.h>
#include <libsoup/soup.h>

#include "gs-repo-row.h"

typedef struct
{
	GsApp		*repo;
	GtkWidget	*name_label;
	GtkWidget	*hostname_label;
	GtkWidget	*comment_label;
	GtkWidget	*remove_button;
	GtkWidget	*disable_switch;
	gulong		 switch_handler_id;
	guint		 refresh_idle_id;
	guint		 busy_counter;
	gboolean	 supports_remove;
	gboolean	 supports_enable_disable;
	gboolean	 always_allow_enable_disable;
	gboolean	 related_loaded;
} GsRepoRowPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GsRepoRow, gs_repo_row, GTK_TYPE_LIST_BOX_ROW)

typedef enum {
	PROP_RELATED_LOADED = 1,
} GsRepoRowProperty;

enum {
	SIGNAL_REMOVE_CLICKED,
	SIGNAL_SWITCH_CLICKED,
	SIGNAL_LAST
};

static GParamSpec *obj_props[PROP_RELATED_LOADED + 1] = { NULL, };
static guint signals [SIGNAL_LAST] = { 0 };

static void
refresh_ui (GsRepoRow *row)
{
	GsRepoRowPrivate *priv = gs_repo_row_get_instance_private (row);
	GtkListBox *listbox;
	gboolean active = FALSE;
	gboolean state_sensitive = FALSE;
	gboolean busy = priv->busy_counter> 0;
	gboolean is_provenance;
	gboolean is_compulsory;

	if (priv->repo == NULL) {
		gtk_widget_set_sensitive (priv->disable_switch, FALSE);
		gtk_switch_set_active (GTK_SWITCH (priv->disable_switch), FALSE);
		return;
	}

	g_signal_handler_block (priv->disable_switch, priv->switch_handler_id);
	gtk_widget_set_sensitive (priv->disable_switch, TRUE);

	switch (gs_app_get_state (priv->repo)) {
	case GS_APP_STATE_AVAILABLE:
	case GS_APP_STATE_AVAILABLE_LOCAL:
		active = FALSE;
		state_sensitive = TRUE;
		break;
	case GS_APP_STATE_INSTALLED:
		active = TRUE;
		break;
	case GS_APP_STATE_INSTALLING:
	case GS_APP_STATE_DOWNLOADING:
		active = TRUE;
		busy = TRUE;
		break;
	case GS_APP_STATE_REMOVING:
		active = FALSE;
		busy = TRUE;
		break;
	case GS_APP_STATE_UNAVAILABLE:
		g_signal_handler_unblock (priv->disable_switch, priv->switch_handler_id);
		listbox = GTK_LIST_BOX (gtk_widget_get_parent (GTK_WIDGET (row)));
		g_assert (listbox != NULL);
		gtk_list_box_remove (listbox, GTK_WIDGET (row));
		return;
	default:
		state_sensitive = TRUE;
		break;
	}

	is_provenance = gs_app_has_quirk (priv->repo, GS_APP_QUIRK_PROVENANCE);
	is_compulsory = gs_app_has_quirk (priv->repo, GS_APP_QUIRK_COMPULSORY);

	/* Disable for the system repos, if installed */
	gtk_widget_set_sensitive (priv->disable_switch, priv->supports_enable_disable && (state_sensitive || !is_compulsory || priv->always_allow_enable_disable));
	gtk_widget_set_visible (priv->remove_button, priv->supports_remove && !is_provenance && !is_compulsory);

	/* Set only the 'state' to visually indicate the state is not saved yet */
	if (busy)
		gtk_switch_set_state (GTK_SWITCH (priv->disable_switch), active);
	else
		gtk_switch_set_active (GTK_SWITCH (priv->disable_switch), active);

	g_signal_handler_unblock (priv->disable_switch, priv->switch_handler_id);
}

static gboolean
refresh_idle (gpointer user_data)
{
	g_autofree GWeakRef *weak_ref = user_data;
	g_autoptr(GsRepoRow) row = NULL;

	row = g_weak_ref_get (weak_ref);
	if (row != NULL) {
		GsRepoRowPrivate *priv = gs_repo_row_get_instance_private (row);

		priv->refresh_idle_id = 0;

		/* The row can be removed from the list box between scheduling the idle
		   callback and dispatching it. */
		if (gtk_widget_get_parent (GTK_WIDGET (row)) != NULL)
			refresh_ui (row);
	}

	g_weak_ref_clear (weak_ref);

	return G_SOURCE_REMOVE;
}

static void
repo_state_changed_cb (GsApp *repo, GParamSpec *pspec, GsRepoRow *row)
{
	GsRepoRowPrivate *priv = gs_repo_row_get_instance_private (row);
	GWeakRef *weak_ref;

	if (priv->refresh_idle_id > 0)
		return;

	weak_ref = g_new0 (GWeakRef, 1);
	g_weak_ref_init (weak_ref, row);
	priv->refresh_idle_id = g_idle_add (refresh_idle, weak_ref);
}

static gchar *
get_repo_installed_text (GsApp *repo)
{
	GsAppList *related;
	guint cnt_addon = 0;
	guint cnt_apps = 0;
	g_autofree gchar *addons_text = NULL;
	g_autofree gchar *apps_text = NULL;

	related = gs_app_get_related (repo);
	for (guint i = 0; i < gs_app_list_length (related); i++) {
		GsApp *app_tmp = gs_app_list_index (related, i);
		switch (gs_app_get_kind (app_tmp)) {
		case AS_COMPONENT_KIND_WEB_APP:
		case AS_COMPONENT_KIND_DESKTOP_APP:
			cnt_apps++;
			break;
		case AS_COMPONENT_KIND_FONT:
		case AS_COMPONENT_KIND_CODEC:
		case AS_COMPONENT_KIND_INPUT_METHOD:
		case AS_COMPONENT_KIND_ADDON:
			cnt_addon++;
			break;
		default:
			break;
		}
	}

	if (cnt_addon == 0) {
		/* TRANSLATORS: This string states how many apps have been
		 * installed from a particular repo, and is displayed on a row
		 * describing that repo. The placeholder is the number of apps. */
		return g_strdup_printf (ngettext ("%u app installed",
		                                  "%u apps installed",
		                                  cnt_apps), cnt_apps);
	}
	if (cnt_apps == 0) {
		/* TRANSLATORS: This string states how many add-ons have been
		 * installed from a particular repo, and is displayed on a row
		 * describing that repo. The placeholder is the number of add-ons. */
		return g_strdup_printf (ngettext ("%u add-on installed",
		                                  "%u add-ons installed",
		                                  cnt_addon), cnt_addon);
	}

	/* TRANSLATORS: This string is used to construct the 'X apps
	   and Y add-ons installed' sentence, stating how many things have been
	 * installed from a particular repo. It’s displayed on a row describing
	 * that repo. The placeholder is the number of apps, and the translated
	 * string will be substituted in for the first placeholder in the
	 * string “%s and %s installed”. */
	apps_text = g_strdup_printf (ngettext ("%u app",
	                                       "%u apps",
	                                       cnt_apps), cnt_apps);
	/* TRANSLATORS: This string is used to construct the 'X apps
	   and Y add-ons installed' sentence, stating how many things have been
	 * installed from a particular repo. It’s displayed on a row describing
	 * that repo. The placeholder is the number of add-ons, and the translated
	 * string will be substituted in for the second placeholder in the
	 * string “%s and %s installed”. */
	addons_text = g_strdup_printf (ngettext ("%u add-on",
	                                         "%u add-ons",
	                                         cnt_addon), cnt_addon);
	/* TRANSLATORS: This string is used to construct the 'X apps
	   and Y add-ons installed' sentence, stating how many things have been
	 * installed from a particular repo. It’s displayed on a row describing
	 * that repo. The first placeholder is the translated string “%u app” or
	 * “%u apps”. The second placeholder is the translated string “%u add-on”
	 * or “%u add-ons”.
	 *
	 * The choice of plural form for this string is determined by the total
	 * number of apps plus add-ons. For example,
	 *  - “1 app and 2 add-ons installed” - uses count 3
	 *  - “2 apps and 1 add-on installed” - uses count 3
	 *  - “4 apps and 5 add-ons installed” - uses count 9
	 */
	return g_strdup_printf (ngettext ("%s and %s installed",
	                                  "%s and %s installed",
	                                  cnt_apps + cnt_addon),
	                                  apps_text, addons_text);
}

static void
refresh_comment_label (GsRepoRow *self)
{
	GsRepoRowPrivate *priv = gs_repo_row_get_instance_private (self);
	g_autofree gchar *comment = NULL;
	const gchar *tmp;

	if (priv->related_loaded)
		comment = get_repo_installed_text (priv->repo);
	else
		comment = g_strdup (_("Checking installed software…"));

	tmp = gs_app_get_metadata_item (priv->repo, "LingmoAppstore::InstallationKind");
	if (tmp != NULL && *tmp != '\0') {
		gchar *cnt;

		/* Translators: The first '%s' is replaced with installation kind, like in case of Flatpak 'User Installation',
		      the second '%s' is replaced with a text like '10 apps installed'. */
		cnt = g_strdup_printf (C_("repo-row", "%s • %s"), tmp, comment);
		g_clear_pointer (&comment, g_free);
		comment = cnt;
	}

	gtk_label_set_label (GTK_LABEL (priv->comment_label), comment);
}

static void
gs_repo_row_set_repo (GsRepoRow *self, GsApp *repo)
{
	GsRepoRowPrivate *priv = gs_repo_row_get_instance_private (self);
	g_autoptr(GsPlugin) plugin = NULL;
	const gchar *tmp;

	g_assert (priv->repo == NULL);

	priv->repo = g_object_ref (repo);
	g_signal_connect_object (priv->repo, "notify::state",
	                         G_CALLBACK (repo_state_changed_cb),
	                         self, 0);

	plugin = gs_app_dup_management_plugin (repo);
	if (plugin) {
		GsPluginClass *plugin_class = GS_PLUGIN_GET_CLASS (plugin);
		priv->supports_remove = plugin_class != NULL && plugin_class->remove_repository_async != NULL;
		priv->supports_enable_disable = plugin_class != NULL &&
			plugin_class->enable_repository_async != NULL &&
			plugin_class->disable_repository_async != NULL;
	} else {
		priv->supports_remove = FALSE;
		priv->supports_enable_disable = FALSE;
	}

	gtk_label_set_label (GTK_LABEL (priv->name_label), gs_app_get_name (repo));

	gtk_widget_set_visible (priv->hostname_label, FALSE);

	tmp = gs_app_get_url (repo, AS_URL_KIND_HOMEPAGE);
	if (tmp != NULL && *tmp != '\0') {
		g_autoptr(GUri) uri = NULL;

		uri = g_uri_parse (tmp, SOUP_HTTP_URI_FLAGS, NULL);
		if (uri && g_uri_get_host (uri) != NULL && *g_uri_get_host (uri) != '\0') {
			gtk_label_set_label (GTK_LABEL (priv->hostname_label), g_uri_get_host (uri));
			gtk_widget_set_visible (priv->hostname_label, TRUE);
		}
	}

	refresh_comment_label (self);
	refresh_ui (self);
}

GsApp *
gs_repo_row_get_repo (GsRepoRow *self)
{
	GsRepoRowPrivate *priv = gs_repo_row_get_instance_private (self);
	g_return_val_if_fail (GS_IS_REPO_ROW (self), NULL);
	return priv->repo;
}

static void
disable_switch_clicked_cb (GtkWidget *widget,
			   GParamSpec *param,
			   GsRepoRow *row)
{
	g_return_if_fail (GS_IS_REPO_ROW (row));
	gs_repo_row_emit_switch_clicked (row);
}

static void
gs_repo_row_remove_button_clicked_cb (GtkWidget *button,
				      GsRepoRow *row)
{
	GsRepoRowPrivate *priv = gs_repo_row_get_instance_private (row);

	g_return_if_fail (GS_IS_REPO_ROW (row));

	if (priv->repo == NULL || priv->busy_counter)
		return;

	g_signal_emit (row, signals[SIGNAL_REMOVE_CLICKED], 0);
}

static void
gs_repo_row_get_property (GObject *object,
			  guint prop_id,
			  GValue *value,
			  GParamSpec *pspec)
{
	GsRepoRow *self = GS_REPO_ROW (object);

	switch ((GsRepoRowProperty) prop_id) {
	case PROP_RELATED_LOADED:
		g_value_set_boolean (value, gs_repo_row_get_related_loaded (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_repo_row_set_property (GObject *object,
			  guint prop_id,
			  const GValue *value,
			  GParamSpec *pspec)
{
	GsRepoRow *self = GS_REPO_ROW (object);

	switch ((GsRepoRowProperty) prop_id) {
	case PROP_RELATED_LOADED:
		gs_repo_row_set_related_loaded (self, g_value_get_boolean (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_repo_row_dispose (GObject *object)
{
	GsRepoRow *self = GS_REPO_ROW (object);
	GsRepoRowPrivate *priv = gs_repo_row_get_instance_private (self);

	if (priv->repo != NULL) {
		g_signal_handlers_disconnect_by_func (priv->repo, repo_state_changed_cb, self);
		g_clear_object (&priv->repo);
	}

	if (priv->refresh_idle_id != 0) {
		g_source_remove (priv->refresh_idle_id);
		priv->refresh_idle_id = 0;
	}

	G_OBJECT_CLASS (gs_repo_row_parent_class)->dispose (object);
}

static void
gs_repo_row_init (GsRepoRow *self)
{
	GsRepoRowPrivate *priv = gs_repo_row_get_instance_private (self);
	GtkWidget *image;

	gtk_widget_init_template (GTK_WIDGET (self));
	priv->switch_handler_id = g_signal_connect (priv->disable_switch, "notify::active",
						    G_CALLBACK (disable_switch_clicked_cb), self);
	image = gtk_image_new_from_icon_name ("user-trash-symbolic");
	gtk_button_set_child (GTK_BUTTON (priv->remove_button), image);
	gtk_widget_set_tooltip_text(priv->remove_button, _("Remove"));
	g_signal_connect (priv->remove_button, "clicked",
		G_CALLBACK (gs_repo_row_remove_button_clicked_cb), self);
}

static void
gs_repo_row_class_init (GsRepoRowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gs_repo_row_get_property;
	object_class->set_property = gs_repo_row_set_property;
	object_class->dispose = gs_repo_row_dispose;

	/**
	 * GsRepoRow:related-loaded:
	 *
	 * Whether the related apps for this repo have been successfully
	 * loaded. If so, the number of apps/installed apps is shown in
	 * the row.
	 *
	 * Since: 45
	 */
	obj_props[PROP_RELATED_LOADED] =
		g_param_spec_boolean ("related-loaded", NULL, NULL,
				      FALSE,
				      G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);

	signals [SIGNAL_REMOVE_CLICKED] =
		g_signal_new ("remove-clicked",
		              G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (GsRepoRowClass, remove_clicked),
		              NULL, NULL, g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0, G_TYPE_NONE);

	signals [SIGNAL_SWITCH_CLICKED] =
		g_signal_new ("switch-clicked",
		              G_TYPE_FROM_CLASS (object_class), G_SIGNAL_RUN_LAST,
		              G_STRUCT_OFFSET (GsRepoRowClass, switch_clicked),
		              NULL, NULL, g_cclosure_marshal_VOID__VOID,
		              G_TYPE_NONE, 0, G_TYPE_NONE);

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-repo-row.ui");

	gtk_widget_class_bind_template_child_private (widget_class, GsRepoRow, name_label);
	gtk_widget_class_bind_template_child_private (widget_class, GsRepoRow, hostname_label);
	gtk_widget_class_bind_template_child_private (widget_class, GsRepoRow, comment_label);
	gtk_widget_class_bind_template_child_private (widget_class, GsRepoRow, remove_button);
	gtk_widget_class_bind_template_child_private (widget_class, GsRepoRow, disable_switch);
}

/*
 * gs_repo_row_new:
 * @repo: a #GsApp to represent the repo in the new row
 * @always_allow_enable_disable: always allow enabled/disable of the @repo
 *
 * The @always_allow_enable_disable, when %TRUE, means that the @repo in this row
 * can be always enabled/disabled by the user, if supported by the related plugin,
 * regardless of the other heuristics, which can avoid the repo enable/disable.
 *
 * Returns: (transfer full): a newly created #GsRepoRow
 */
GtkWidget *
gs_repo_row_new (GsApp *repo,
		 gboolean always_allow_enable_disable)
{
	GsRepoRow *row = g_object_new (GS_TYPE_REPO_ROW, NULL);
	GsRepoRowPrivate *priv = gs_repo_row_get_instance_private (row);
	priv->always_allow_enable_disable = always_allow_enable_disable;
	gs_repo_row_set_repo (row, repo);
	return GTK_WIDGET (row);
}

static void
gs_repo_row_change_busy (GsRepoRow *self,
			 gboolean value)
{
	GsRepoRowPrivate *priv = gs_repo_row_get_instance_private (self);

	g_return_if_fail (GS_IS_REPO_ROW (self));

	if (value)
		g_return_if_fail (priv->busy_counter + 1 > priv->busy_counter);
	else
		g_return_if_fail (priv->busy_counter > 0);

	priv->busy_counter += (value ? 1 : -1);

	if (value && priv->busy_counter == 1)
		gtk_widget_set_sensitive (priv->disable_switch, FALSE);
	else if (!value && !priv->busy_counter)
		refresh_ui (self);
}

/**
 * gs_repo_row_mark_busy:
 * @row: a #GsRepoRow
 *
 * Mark the @row as busy, that is the @row has pending operation(s).
 * Unmark the @row as busy with gs_repo_row_unmark_busy() once
 * the operation is done. This can be called mutliple times, only call
 * the gs_repo_row_unmark_busy() as many times as this function had
 * been called.
 *
 * Since: 41
 **/
void
gs_repo_row_mark_busy (GsRepoRow *row)
{
	gs_repo_row_change_busy (row, TRUE);
}

/**
 * gs_repo_row_unmark_busy:
 * @row: a #GsRepoRow
 *
 * A pair function for gs_repo_row_mark_busy().
 *
 * Since: 41
 **/
void
gs_repo_row_unmark_busy (GsRepoRow *row)
{
	gs_repo_row_change_busy (row, FALSE);
}

/**
 * gs_repo_row_get_is_busy:
 * @row: a #GsRepoRow
 *
 * Returns: %TRUE, when there is any pending operation for the @row
 *
 * Since: 41
 **/
gboolean
gs_repo_row_get_is_busy (GsRepoRow *row)
{
	GsRepoRowPrivate *priv = gs_repo_row_get_instance_private (row);

	g_return_val_if_fail (GS_IS_REPO_ROW (row), FALSE);

	return priv->busy_counter > 0;
}

/**
 * gs_repo_row_emit_switch_clicked:
 * @self: a #GsRepoRow
 *
 * Emits the GsRepoRow:switch-clicked signal, if applicable.
 *
 * Since: 41
 **/
void
gs_repo_row_emit_switch_clicked (GsRepoRow *self)
{
	GsRepoRowPrivate *priv = gs_repo_row_get_instance_private (self);

	g_return_if_fail (GS_IS_REPO_ROW (self));

	if (priv->repo == NULL || priv->busy_counter > 0 ||
	    !gtk_widget_get_visible (priv->disable_switch) ||
	    !gtk_widget_get_sensitive (priv->disable_switch))
		return;

	g_signal_emit (self, signals[SIGNAL_SWITCH_CLICKED], 0);
}

gboolean
gs_repo_row_get_related_loaded (GsRepoRow *self)
{
	GsRepoRowPrivate *priv = gs_repo_row_get_instance_private (self);

	g_return_val_if_fail (GS_IS_REPO_ROW (self), FALSE);

	return priv->related_loaded;
}

void
gs_repo_row_set_related_loaded (GsRepoRow *self,
				gboolean value)
{
	GsRepoRowPrivate *priv = gs_repo_row_get_instance_private (self);

	g_return_if_fail (GS_IS_REPO_ROW (self));

	if (!priv->related_loaded == !value)
		return;

	priv->related_loaded = value;

	refresh_comment_label (self);

	g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_RELATED_LOADED]);
}
