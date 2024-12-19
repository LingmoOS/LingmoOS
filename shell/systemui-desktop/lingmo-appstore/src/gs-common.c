/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013-2015 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2016-2019 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <glib/gi18n.h>
#include <gio/gdesktopappinfo.h>

#include <adwaita.h>
#ifndef TESTDATADIR
#include "gs-application.h"
#endif

#include "gs-common.h"

#ifdef HAVE_GSETTINGS_DESKTOP_SCHEMAS
#include <gdesktop-enums.h>
#endif

#include <langinfo.h>

void
gs_widget_remove_all (GtkWidget    *container,
                      GsRemoveFunc  remove_func)
{
	GtkWidget *child;
	while ((child = gtk_widget_get_first_child (container)) != NULL) {
		if (remove_func)
			remove_func (container, child);
		else
			gtk_widget_unparent (child);
	}
}

static void
grab_focus (GtkWidget *widget)
{
	g_signal_handlers_disconnect_by_func (widget, grab_focus, NULL);
	gtk_widget_grab_focus (widget);
}

void
gs_grab_focus_when_mapped (GtkWidget *widget)
{
	if (gtk_widget_get_mapped (widget))
		gtk_widget_grab_focus (widget);
	else
		g_signal_connect_after (widget, "map",
					G_CALLBACK (grab_focus), NULL);
}

void
gs_app_notify_installed (GsApp *app)
{
	g_autofree gchar *summary = NULL;
	const gchar *body = NULL;
	g_autoptr(GNotification) n = NULL;

	switch (gs_app_get_kind (app)) {
	case AS_COMPONENT_KIND_DESKTOP_APP:
		/* TRANSLATORS: this is the summary of a notification that an app
		 * has been successfully installed */
		summary = g_strdup_printf (_("%s is now installed"), gs_app_get_name (app));
		if (gs_app_has_quirk (app, GS_APP_QUIRK_NEEDS_REBOOT)) {
			/* TRANSLATORS: an app has been installed, but
			 * needs a reboot to complete the installation */
			body = _("A restart is required for the changes to take effect.");
		} else {
			/* TRANSLATORS: this is the body of a notification that an app
			 * has been successfully installed */
			body = _("App is ready to be used.");
		}
		break;
	default:
		if (gs_app_get_kind (app) == AS_COMPONENT_KIND_GENERIC &&
		    gs_app_get_special_kind (app) == GS_APP_SPECIAL_KIND_OS_UPDATE) {
			/* TRANSLATORS: this is the summary of a notification that OS updates
			* have been successfully installed */
			summary = g_strdup (_("System updates are now installed"));
			/* TRANSLATORS: this is the body of a notification that OS updates
			* have been successfully installed */
			body = _("Recently installed updates are available to review");
		} else {
			/* TRANSLATORS: this is the summary of a notification that a component
			* has been successfully installed */
			summary = g_strdup_printf (_("%s is now installed"), gs_app_get_name (app));
			if (gs_app_has_quirk (app, GS_APP_QUIRK_NEEDS_REBOOT)) {
				/* TRANSLATORS: an app has been installed, but
				* needs a reboot to complete the installation */
				body = _("A restart is required for the changes to take effect.");
			}
		}
		break;
	}
	n = g_notification_new (summary);
	if (body != NULL)
		g_notification_set_body (n, body);

	if (gs_app_has_quirk (app, GS_APP_QUIRK_NEEDS_REBOOT)) {
		/* TRANSLATORS: button text */
		g_notification_add_button_with_target (n, _("Restart"),
						       "app.reboot", NULL);
	} else if (gs_app_get_kind (app) == AS_COMPONENT_KIND_DESKTOP_APP) {
		/* TRANSLATORS: this is button that opens the newly installed app */
		g_autoptr(GsPlugin) plugin = gs_app_dup_management_plugin (app);
		const gchar *plugin_name = (plugin != NULL) ? gs_plugin_get_name (plugin) : "";
		g_notification_add_button_with_target (n, _("Launch"),
						       "app.launch", "(ss)",
						       gs_app_get_id (app),
						       plugin_name);
	}
	g_notification_set_default_action_and_target  (n, "app.details", "(ss)",
						       gs_app_get_unique_id (app), "");
	#ifdef TESTDATADIR
	g_application_send_notification (g_application_get_default (), "installed", n);
	#else
	gs_application_send_notification (GS_APPLICATION (g_application_get_default ()), "installed", n, 24 * 60);
	#endif
}

typedef enum {
	GS_APP_LICENSE_FREE		= 0,
	GS_APP_LICENSE_NONFREE		= 1,
	GS_APP_LICENSE_PATENT_CONCERN	= 2
} GsAppLicenseHint;

typedef struct
{
	gint response_id;
	GMainLoop *loop;
} RunInfo;

static void
shutdown_loop (RunInfo *run_info)
{
	if (g_main_loop_is_running (run_info->loop))
		g_main_loop_quit (run_info->loop);
}

static void
unmap_cb (GtkDialog *dialog,
          RunInfo   *run_info)
{
	shutdown_loop (run_info);
}

static void
response_cb (AdwMessageDialog *self,
             const gchar      *response,
             RunInfo          *run_info)
{
	if (g_strcmp0 (response, "accept") == 0)
		run_info->response_id = GTK_RESPONSE_OK;
	else if (g_strcmp0 (response, "install") == 0)
		run_info->response_id = GTK_RESPONSE_OK;
	else if (g_strcmp0 (response, "dont-warn-again") == 0)
		run_info->response_id = GTK_RESPONSE_YES;
	else
		run_info->response_id = GTK_RESPONSE_CANCEL;
	shutdown_loop (run_info);
}

static gboolean
gs_common_app_is_from_official_repository (GsApp *app,
					   GSettings *settings)
{
	g_auto(GStrv) official_repos = NULL;
	const gchar *origin = gs_app_get_origin (app);

	if (origin == NULL)
		return FALSE;
	official_repos = g_settings_get_strv (settings, "official-repos");
	return official_repos &&
	       gs_utils_strv_fnmatch ((gchar **) official_repos, origin);
}

GtkResponseType
gs_app_notify_unavailable (GsApp *app, GtkWindow *parent)
{
	GsAppLicenseHint hint = GS_APP_LICENSE_FREE;
	GtkWidget *dialog;
	const gchar *license;
	gboolean already_enabled = FALSE;	/* FIXME */
	g_autofree gchar *origin_ui = NULL;
	guint i;
	struct {
		const gchar	*str;
		GsAppLicenseHint hint;
	} keywords[] = {
		{ "NonFree",		GS_APP_LICENSE_NONFREE },
		{ "PatentConcern",	GS_APP_LICENSE_PATENT_CONCERN },
		{ "Proprietary",	GS_APP_LICENSE_NONFREE },
		{ NULL, 0 }
	};
	g_autoptr(GSettings) settings = NULL;
	g_autoptr(GString) body = NULL;
	const gchar *title;

	RunInfo run_info = {
		GTK_RESPONSE_NONE,
		NULL,
	};

	/* this is very crude */
	license = gs_app_get_license (app);
	if (license != NULL) {
		for (i = 0; keywords[i].str != NULL; i++) {
			if (g_strstr_len (license, -1, keywords[i].str) != NULL)
				hint |= keywords[i].hint;
		}
	} else {
		/* use the worst-case assumption */
		hint = GS_APP_LICENSE_NONFREE | GS_APP_LICENSE_PATENT_CONCERN;
	}

	/* check if the user has already dismissed */
	settings = g_settings_new ("org.lingmo.appstore");
	if (!g_settings_get_boolean (settings, "prompt-for-nonfree"))
		return GTK_RESPONSE_OK;

	if (already_enabled) {
		title = gs_common_app_is_from_official_repository (app, settings) ?
			/* TRANSLATORS: window title */
			_("Install Appstore?") :
			/* TRANSLATORS: window title */
			_("Install Third-Party Appstore?");
	} else {
		title = gs_common_app_is_from_official_repository (app, settings) ?
			/* TRANSLATORS: window title */
			_("Enable Appstore Repository?") :
			/* TRANSLATORS: window title */
			_("Enable Third-Party Appstore Repository?");
	}

	body = g_string_new ("");
	origin_ui = gs_app_dup_origin_ui (app, TRUE);

	if (hint & GS_APP_LICENSE_NONFREE) {
		g_string_append_printf (body,
					/* TRANSLATORS: the replacements are as follows:
					 * 1. App name, e.g. "Firefox"
					 * 2. Appstore repository name, e.g. fedora-optional
					 */
					_("%s is not <a href=\"https://en.wikipedia.org/wiki/Free_and_open-source_software\">"
					  "free and open source software</a>, "
					  "and is provided by “%s”."),
					gs_app_get_name (app),
					origin_ui);
	} else {
		g_string_append_printf (body,
					/* TRANSLATORS: the replacements are as follows:
					 * 1. App name, e.g. "Firefox"
					 * 2. Appstore repository name, e.g. fedora-optional */
					_("%s is provided by “%s”."),
					gs_app_get_name (app),
					origin_ui);
	}

	/* tell the use what needs to be done */
	if (!already_enabled) {
		g_string_append (body, " ");
		g_string_append (body,
				_("This software repository must be "
				  "enabled to continue installation."));
	}

	/* be aware of patent clauses */
	if (hint & GS_APP_LICENSE_PATENT_CONCERN) {
		g_string_append (body, "\n\n");
		if (gs_app_get_kind (app) != AS_COMPONENT_KIND_CODEC) {
			g_string_append_printf (body,
						/* TRANSLATORS: Laws are geographical, urgh... */
						_("It may be illegal to install "
						  "or use %s in some countries."),
						gs_app_get_name (app));
		} else {
			g_string_append (body,
					/* TRANSLATORS: Laws are geographical, urgh... */
					_("It may be illegal to install or use "
					  "this codec in some countries."));
		}
	}

	dialog = adw_message_dialog_new (parent,
					 title,
					 body->str);
	adw_message_dialog_set_body_use_markup (ADW_MESSAGE_DIALOG (dialog), TRUE);

	adw_message_dialog_add_response (ADW_MESSAGE_DIALOG (dialog),
					 "cancel",  _("_Cancel"));

	/* TRANSLATORS: this is button text to not ask about non-free content again */
	if (0) adw_message_dialog_add_response (ADW_MESSAGE_DIALOG (dialog), "dont-warn-again",  _("Don’t _Warn Again"));
	if (already_enabled) {
		adw_message_dialog_add_response (ADW_MESSAGE_DIALOG (dialog),
						 /* TRANSLATORS: button text */
						 "install", _("_Install"));

	} else {
		adw_message_dialog_add_response (ADW_MESSAGE_DIALOG (dialog),
						 /* TRANSLATORS: button text */
						 "install", _("Enable and _Install"));
	}

	/* Run */
	gtk_window_present (GTK_WINDOW (dialog));

	g_signal_connect (dialog, "response", G_CALLBACK (response_cb), &run_info);
	g_signal_connect (dialog, "unmap", G_CALLBACK (unmap_cb), &run_info);

	run_info.loop = g_main_loop_new (NULL, FALSE);
	g_main_loop_run (run_info.loop);
	g_clear_pointer (&run_info.loop, g_main_loop_unref);

	if (run_info.response_id == GTK_RESPONSE_YES) {
		run_info.response_id = GTK_RESPONSE_OK;
		g_settings_set_boolean (settings, "prompt-for-nonfree", FALSE);
	}
	return run_info.response_id;
}

gboolean
gs_utils_is_current_desktop (const gchar *name)
{
	const gchar *tmp;
	g_auto(GStrv) names = NULL;
	tmp = g_getenv ("XDG_CURRENT_DESKTOP");
	if (tmp == NULL)
		return FALSE;
	names = g_strsplit (tmp, ":", -1);
	return g_strv_contains ((const gchar * const *) names, name);
}

static void
gs_utils_widget_css_parsing_error_cb (GtkCssProvider *provider,
				      GtkCssSection *section,
				      GError *error,
				      gpointer user_data)
{
	const GtkCssLocation *start_location;

	start_location = gtk_css_section_get_start_location (section);
	g_warning ("CSS parse error %" G_GSIZE_FORMAT ":%" G_GSIZE_FORMAT ": %s",
		   start_location->lines + 1,
		   start_location->line_chars,
		   error->message);
}

/**
 * gs_utils_set_key_colors_in_css:
 * @css: some CSS
 * @app: a #GsApp to get the key colors from
 *
 * Replace placeholders in @css with the key colors from @app, returning a copy
 * of the CSS with the key colors inlined as `rgb()` literals.
 *
 * The key color placeholders are of the form `@keycolor-XX@`, where `XX` is a
 * two digit counter. The first counter (`00`) will be replaced with the first
 * key color in @app, the second counter (`01`) with the second, etc.
 *
 * CSS may be %NULL, in which case %NULL is returned.
 *
 * Returns: (transfer full): a copy of @css with the key color placeholders
 *     replaced, free with g_free()
 * Since: 40
 */
gchar *
gs_utils_set_key_colors_in_css (const gchar *css,
                                GsApp       *app)
{
	GArray *key_colors;
	g_autoptr(GString) css_new = NULL;

	if (css == NULL)
		return NULL;

	key_colors = gs_app_get_key_colors (app);

	/* Do we not need to do any replacements? */
	if (key_colors->len == 0 ||
	    g_strstr_len (css, -1, "@keycolor") == NULL)
		return g_strdup (css);

	/* replace key color values */
	css_new = g_string_new (css);
	for (guint j = 0; j < key_colors->len; j++) {
		const GdkRGBA *color = &g_array_index (key_colors, GdkRGBA, j);
		g_autofree gchar *key = NULL;
		g_autofree gchar *value = NULL;
		key = g_strdup_printf ("@keycolor-%02u@", j);
		value = g_strdup_printf ("rgb(%.0f,%.0f,%.0f)",
					 color->red * 255.f,
					 color->green * 255.f,
					 color->blue * 255.f);
		gs_utils_gstring_replace (css_new, key, value);
	}

	return g_string_free (g_steal_pointer (&css_new), FALSE);
}

/**
 * gs_utils_widget_set_css:
 * @widget: a widget
 * @provider: (inout) (transfer full) (not optional) (nullable): pointer to a
 *    #GtkCssProvider to use
 * @css: (nullable): CSS to set on the widget, or %NULL to clear custom CSS
 *
 * Set custom CSS on the given @widget instance. This uses the widget's name
 * property to identify it. If the widget is unnamed, which means its name is
 * %NULL, the empty string, or its type name, it will receive a unique name.
 *
 * @provider must be a pointer to a #GtkCssProvider pointer, typically within
 * your widget’s private data struct. This function will return a
 * #GtkCssProvider in the provided pointer, reusing any old @provider if
 * possible. When your widget is destroyed, you must destroy the returned
 * @provider. If @css is %NULL, this function will destroy the @provider.
 */
void
gs_utils_widget_set_css (GtkWidget *widget, GtkCssProvider **provider, const gchar *css)
{
	GdkDisplay *display;
	const gchar *widget_name;
	g_autoptr(GString) str = NULL;

	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (provider != NULL);
	g_return_if_fail (provider == NULL || *provider == NULL || GTK_IS_STYLE_PROVIDER (*provider));

	display = gtk_widget_get_display (widget);

	/* remove custom CSS if NULL */
	if (css == NULL) {
		if (*provider != NULL)
			gtk_style_context_remove_provider_for_display (display, GTK_STYLE_PROVIDER (*provider));
		g_clear_object (provider);
		return;
	}

	widget_name = gtk_widget_get_name (widget);

	/* give the widget a unique name if NULL */
	if (widget_name == NULL ||
	    *widget_name == '\0' ||
	    g_strcmp0 (widget_name, G_OBJECT_TYPE_NAME (widget)) == 0) {
		gchar tmp[26];  /* "widget-0x", up to 16 hex digits, and '\0'. */
		g_assert ((gsize) g_snprintf (tmp, sizeof (tmp), "widget-%p", widget) < sizeof (tmp));
		gtk_widget_set_name (widget, (const gchar *) tmp);
		widget_name = gtk_widget_get_name (widget);
	}

	/* prepare the CSS code */
	str = g_string_sized_new (1024);
	g_string_append_printf (str, "#%s {\n", widget_name);
	g_string_append_printf (str, "%s\n", css);
	g_string_append (str, "}");

	/* create a new provider if needed */
	if (*provider == NULL) {
		*provider = gtk_css_provider_new ();
		g_signal_connect (*provider, "parsing-error",
				  G_CALLBACK (gs_utils_widget_css_parsing_error_cb), NULL);
	}

	/* set up custom provider and store on the widget */
	#if GTK_CHECK_VERSION(4, 12, 0)
	gtk_css_provider_load_from_string (*provider, str->str);
	#else
	gtk_css_provider_load_from_data (*provider, str->str, -1);
	#endif
	gtk_style_context_add_provider_for_display (display, GTK_STYLE_PROVIDER (*provider),
						    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

static void
unset_focus (GtkWidget *widget, gpointer data)
{
	if (GTK_IS_WINDOW (widget))
		gtk_window_set_focus (GTK_WINDOW (widget), NULL);
}

/**
 * insert_details_widget:
 * @dialog: the message dialog where the widget will be inserted
 * @details: the detailed message text to display
 *
 * Inserts a widget displaying the detailed message into the message dialog.
 */
static void
insert_details_widget (AdwMessageDialog *dialog,
		       const gchar *details,
		       gboolean add_prefix)
{
	GtkWidget *box, *sw, *label;
	GtkWidget *tv;
	GtkTextBuffer *buffer;
	g_autoptr(GString) msg = NULL;

	g_assert (ADW_IS_MESSAGE_DIALOG (dialog));
	g_assert (details != NULL);

	gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);

	if (add_prefix) {
		msg = g_string_new ("");
		g_string_append_printf (msg, "%s\n\n%s",
					/* TRANSLATORS: these are show_detailed_error messages from the
					 * package manager no mortal is supposed to understand,
					 * but google might know what they mean */
					_("Detailed errors from the package manager follow:"),
					details);
	}

	box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	adw_message_dialog_set_extra_child (ADW_MESSAGE_DIALOG (dialog), box);

	label = gtk_label_new (_("Details"));
	gtk_widget_set_halign (label, GTK_ALIGN_START);
	gtk_widget_set_visible (label, TRUE);
	gtk_box_append (GTK_BOX (box), label);

	sw = gtk_scrolled_window_new ();
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
	                                GTK_POLICY_NEVER,
	                                GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_min_content_height (GTK_SCROLLED_WINDOW (sw), 150);
	gtk_widget_set_visible (sw, TRUE);

	tv = gtk_text_view_new ();
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (tv));
	gtk_text_view_set_editable (GTK_TEXT_VIEW (tv), FALSE);
	gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (tv), GTK_WRAP_WORD);
	gtk_widget_add_css_class (tv, "update-failed-details");
	gtk_text_buffer_set_text (buffer, msg ? msg->str : details, -1);
	gtk_widget_set_visible (tv, TRUE);

	gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (sw), tv);
	gtk_widget_set_vexpand (sw, TRUE);
	gtk_box_append (GTK_BOX (box), sw);

	g_signal_connect (dialog, "map", G_CALLBACK (unset_focus), NULL);
}

/**
 * gs_utils_show_error_dialog:
 * @parent: transient parent, or NULL for none
 * @title: the title for the dialog
 * @msg: the message for the dialog
 * @details: (allow-none): the detailed error message, or NULL for none
 *
 * Shows a message dialog for displaying error messages.
 */
void
gs_utils_show_error_dialog (GtkWindow *parent,
                            const gchar *title,
                            const gchar *msg,
                            const gchar *details)
{
	GtkWidget *dialog;

	dialog = adw_message_dialog_new (parent, title, msg);
	if (details != NULL)
		insert_details_widget (ADW_MESSAGE_DIALOG (dialog), details, TRUE);
	adw_message_dialog_add_response (ADW_MESSAGE_DIALOG (dialog),
					 /* TRANSLATORS: button text */
					 "close", _("_Close"));
	gtk_window_present (GTK_WINDOW (dialog));
}

#ifndef TESTDATADIR
static void
copy_error_text_clicked_cb (GtkButton *button,
			    GtkTextView *text_view)
{
	GdkClipboard *clipboard = gtk_widget_get_clipboard (GTK_WIDGET (text_view));
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (text_view);
	GtkTextIter start, end;
	g_autofree gchar *text = NULL;

	gtk_text_buffer_get_bounds (buffer, &start, &end);
	text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
	gdk_clipboard_set_text (clipboard, text);
}
#endif

/**
 * gs_utils_show_error_dialog_simple:
 * @parent: transient parent, or %NULL for none
 * @title: the title the dialog
 * @text: the detailed error text
 *
 * Shows a dialog with title @title to display an error
 * message @text.
 *
 * Since: 44
 */
void
gs_utils_show_error_dialog_simple (GtkWindow *parent,
				   const gchar *title,
				   const gchar *text)
{
#ifndef TESTDATADIR
	GtkWidget *window, *button, *container, *text_view, *vbox, *hbox, *label;
	g_autoptr(PangoAttrList) bold = NULL;

	g_return_if_fail (text != NULL);

	bold = pango_attr_list_new ();
	pango_attr_list_insert (bold, pango_attr_weight_new (PANGO_WEIGHT_BOLD));

	window = adw_window_new ();
	g_object_set (window,
		      "modal", TRUE,
		      "title", " ",
		      "destroy-with-parent", TRUE,
		      "default-width", 500,
		      "default-height", 350,
		      "resizable", TRUE,
		      "transient-for", parent,
		      NULL);
	vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
	adw_window_set_content (ADW_WINDOW (window), vbox);

	container = adw_header_bar_new ();
	gtk_widget_add_css_class (container, "flat");
	gtk_box_append (GTK_BOX (vbox), container);

	label = gtk_label_new (title);
	g_object_set (label,
		      "halign", GTK_ALIGN_CENTER,
		      "margin-start", 12,
		      "margin-end", 12,
		      "margin-top", 0,
		      "margin-bottom", 12,
		      "justify", GTK_JUSTIFY_CENTER,
		      "wrap-mode", PANGO_WRAP_WORD_CHAR,
		      "wrap", TRUE,
		      "width-chars", 40,
		      "max-width-chars", 40,
		      NULL);
	gtk_widget_add_css_class (label, "title-4");
	gtk_box_append (GTK_BOX (vbox), label);

	hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);
	g_object_set (hbox,
		      "halign", GTK_ALIGN_FILL,
		      "hexpand", TRUE,
		      "margin-start", 12,
		      "margin-end", 12,
		      "margin-bottom", 6,
		      NULL);
	gtk_box_append (GTK_BOX (vbox), hbox);

	label = gtk_label_new (_("Details"));
	g_object_set (label,
		      "halign", GTK_ALIGN_START,
		      "hexpand", TRUE,
		      "attributes", bold,
		      NULL);
	gtk_box_append (GTK_BOX (hbox), label);

	button = gtk_button_new_from_icon_name ("edit-copy-symbolic");
	g_object_set (button,
		      "focus-on-click", FALSE,
		      NULL);
	gtk_widget_set_halign (button, GTK_ALIGN_END);
	gtk_widget_add_css_class (button, "flat");
	gtk_box_append (GTK_BOX (hbox), button);

	container = gtk_scrolled_window_new ();
	g_object_set (container,
		      "hscrollbar-policy", GTK_POLICY_AUTOMATIC,
		      "vscrollbar-policy", GTK_POLICY_AUTOMATIC,
		      "margin-start", 12,
		      "margin-end", 12,
		      "margin-bottom", 12,
		      "has-frame", TRUE,
		      NULL);
	gtk_widget_add_css_class (container, "gs-rounded");
	gtk_box_append (GTK_BOX (vbox), container);

	text_view = gtk_text_view_new ();
	g_object_set (text_view,
		      "can-focus", FALSE,
		      "editable", FALSE,
		      "hexpand", TRUE,
		      "vexpand", TRUE,
		      "wrap-mode", GTK_WRAP_WORD_CHAR,
		      "right-margin", 12,
		      "left-margin", 12,
		      "top-margin", 12,
		      "bottom-margin", 12,
		      NULL);
	gtk_widget_add_css_class (text_view, "gs-rounded");
	gtk_text_buffer_set_text (gtk_text_view_get_buffer (GTK_TEXT_VIEW (text_view)), text, -1);
	gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (container), text_view);

	g_signal_connect (button, "clicked",
			  G_CALLBACK (copy_error_text_clicked_cb), text_view);

	gtk_widget_set_visible (window, TRUE);
#endif /* TESTDATADIR */
}

/**
 * gs_utils_ask_user_accepts:
 * @parent: (nullable): modal parent, or %NULL for none
 * @title: the title for the dialog
 * @msg: the message for the dialog
 * @details: (nullable): the detailed error message, or %NULL for none
 * @accept_label: (nullable): a label of the 'accept' button, or %NULL to use 'Accept'
 *
 * Shows a modal question dialog for displaying an accept/cancel question to the user.
 *
 * Returns: whether the user accepted the question
 *
 * Since: 42
 **/
gboolean
gs_utils_ask_user_accepts (GtkWindow *parent,
			   const gchar *title,
			   const gchar *msg,
			   const gchar *details,
			   const gchar *accept_label)
{
	GtkWidget *dialog;
	RunInfo run_info;

	g_return_val_if_fail (parent == NULL || GTK_IS_WINDOW (parent), FALSE);
	g_return_val_if_fail (title != NULL, FALSE);
	g_return_val_if_fail (msg != NULL, FALSE);

	if (accept_label == NULL || *accept_label == '\0') {
		/* Translators: an accept button label, in a Cancel/Accept dialog */
		accept_label = _("_Accept");
	}

	dialog = adw_message_dialog_new (parent, title, msg);
	if (details != NULL)
		insert_details_widget (ADW_MESSAGE_DIALOG (dialog), details, FALSE);
	adw_message_dialog_add_responses (ADW_MESSAGE_DIALOG (dialog),
					  /* TRANSLATORS: button text */
					  "cancel", _("_Cancel"),
					  /* TRANSLATORS: button text */
					  "accept", accept_label,
					  NULL);

	run_info.response_id = GTK_RESPONSE_NONE;
	run_info.loop = g_main_loop_new (NULL, FALSE);

	/* Run */
	gtk_window_present (GTK_WINDOW (dialog));

	g_signal_connect (dialog, "response", G_CALLBACK (response_cb), &run_info);
	g_signal_connect (dialog, "unmap", G_CALLBACK (unmap_cb), &run_info);

	g_main_loop_run (run_info.loop);
	g_clear_pointer (&run_info.loop, g_main_loop_unref);

	return run_info.response_id == GTK_RESPONSE_OK;
}

/**
 * gs_utils_get_error_value:
 * @error: A GError
 *
 * Gets the machine-readable value stored in the error message.
 * The machine readable string is after the first "@", e.g.
 * message = "Requires authentication with @aaa"
 *
 * Returns: a string, or %NULL
 */
const gchar *
gs_utils_get_error_value (const GError *error)
{
	gchar *str;
	if (error == NULL)
		return NULL;
	str = g_strstr_len (error->message, -1, "@");
	if (str == NULL)
		return NULL;
	return (const gchar *) str + 1;
}

/**
 * gs_utils_build_unique_id_kind:
 * @kind: A #AsComponentKind
 * @id: An application ID
 *
 * Converts the ID valid into a wildcard unique ID of a specific kind.
 * If @id is already a unique ID, then it is returned unchanged.
 *
 * Returns: (transfer full): a unique ID, or %NULL
 */
gchar *
gs_utils_build_unique_id_kind (AsComponentKind kind, const gchar *id)
{
	if (as_utils_data_id_valid (id))
		return g_strdup (id);
	return gs_utils_build_unique_id (AS_COMPONENT_SCOPE_UNKNOWN,
					 AS_BUNDLE_KIND_UNKNOWN,
					 NULL,
					 id,
					 NULL);
}

/**
 * gs_utils_list_has_component_fuzzy:
 * @list: A #GsAppList
 * @app: A #GsApp
 *
 * Finds out if any app in the list would match a given app,
 * where the match is valid for a matching D-Bus bus name,
 * the label in the UI or the same icon.
 *
 * This function is normally used to work out if the source should be shown
 * in a GsAppRow.
 *
 * Returns: %TRUE if the app is visually the "same"
 */
gboolean
gs_utils_list_has_component_fuzzy (GsAppList *list, GsApp *app)
{
	guint i;
	GsApp *tmp;

	for (i = 0; i < gs_app_list_length (list); i++) {
		tmp = gs_app_list_index (list, i);

		/* ignore if the same object */
		if (app == tmp)
			continue;

		/* ignore with the same source */
		if (g_strcmp0 (gs_app_get_origin_hostname (tmp),
			       gs_app_get_origin_hostname (app)) == 0) {
			continue;
		}

		/* same D-Bus ID */
		if (g_strcmp0 (gs_app_get_id (tmp),
			       gs_app_get_id (app)) == 0) {
			return TRUE;
		}

		/* same name */
		if (g_strcmp0 (gs_app_get_name (tmp),
			       gs_app_get_name (app)) == 0) {
			return TRUE;
		}
	}
	return FALSE;
}

void
gs_utils_reboot_notify (GsAppList *list,
			gboolean is_install)
{
	g_autoptr(GNotification) n = NULL;
	g_autofree gchar *tmp = NULL;
	const gchar *app_name = NULL;
	const gchar *title;
	const gchar *body;

	if (gs_app_list_length (list) == 1) {
		GsApp *app = gs_app_list_index (list, 0);
		if (gs_app_get_kind (app) == AS_COMPONENT_KIND_DESKTOP_APP) {
			app_name = gs_app_get_name (app);
			if (!*app_name)
				app_name = NULL;
		}
	}

	if (is_install) {
		if (app_name) {
			/* TRANSLATORS: The '%s' is replaced with the app name */
			tmp = g_strdup_printf ("An app “%s” has been installed", app_name);
			title = tmp;
		} else {
			/* TRANSLATORS: we've just live-updated some apps */
			title = ngettext ("An update has been installed",
					  "Updates have been installed",
					  gs_app_list_length (list));
		}
	} else if (app_name) {
		/* TRANSLATORS: The '%s' is replaced with the app name */
		tmp = g_strdup_printf ("An app “%s” has been removed", app_name);
		title = tmp;
	} else {
		/* TRANSLATORS: we've just removed some apps */
		title = ngettext ("An app has been removed",
				  "Apps have been removed",
				  gs_app_list_length (list));
	}

	/* TRANSLATORS: the new apps will not be run until we restart */
	body = ngettext ("A restart is required for it to take effect.",
	                 "A restart is required for them to take effect.",
	                 gs_app_list_length (list));

	n = g_notification_new (title);
	g_notification_set_body (n, body);
	/* TRANSLATORS: button text */
	g_notification_add_button (n, _("Not Now"), "app.nop");
	/* TRANSLATORS: button text */
	g_notification_add_button_with_target (n, _("Restart"), "app.reboot", NULL);
	g_notification_set_default_action_and_target (n, "app.set-mode", "s", "updates");
	g_notification_set_priority (n, G_NOTIFICATION_PRIORITY_URGENT);
	#ifdef TESTDATADIR
	g_application_send_notification (g_application_get_default (), "restart-required", n);
	#else
	gs_application_send_notification (GS_APPLICATION (g_application_get_default ()), "restart-required", n, 0);
	#endif
}

/**
 * gs_utils_split_time_difference:
 * @unix_time_seconds: Time since the epoch in seconds
 * @out_minutes_ago: (out) (nullable): how many minutes elapsed
 * @out_hours_ago: (out) (nullable): how many hours elapsed
 * @out_days_ago: (out) (nullable): how many days elapsed
 * @out_weeks_ago: (out) (nullable): how many weeks elapsed
 * @out_months_ago: (out) (nullable): how many months elapsed
 * @out_years_ago: (out) (nullable): how many years elapsed
 *
 * Calculates the difference between the @unix_time_seconds and the current time
 * and splits it into separate values.
 *
 * Returns: whether the out parameters had been set
 *
 * Since: 41
 **/
gboolean
gs_utils_split_time_difference (gint64 unix_time_seconds,
				gint *out_minutes_ago,
				gint *out_hours_ago,
				gint *out_days_ago,
				gint *out_weeks_ago,
				gint *out_months_ago,
				gint *out_years_ago)
{
	gint minutes_ago, hours_ago, days_ago;
	gint weeks_ago, months_ago, years_ago;
	g_autoptr(GDateTime) date_time = NULL;
	g_autoptr(GDateTime) now = NULL;
	GTimeSpan timespan;

	if (unix_time_seconds <= 0)
		return FALSE;

	date_time = g_date_time_new_from_unix_local (unix_time_seconds);
	now = g_date_time_new_now_local ();
	timespan = g_date_time_difference (now, date_time);

	minutes_ago = (gint) (timespan / G_TIME_SPAN_MINUTE);
	hours_ago = (gint) (timespan / G_TIME_SPAN_HOUR);
	days_ago = (gint) (timespan / G_TIME_SPAN_DAY);
	weeks_ago = days_ago / 7;
	months_ago = days_ago / 30;
	years_ago = weeks_ago / 52;

	if (out_minutes_ago)
		*out_minutes_ago = minutes_ago;
	if (out_hours_ago)
		*out_hours_ago = hours_ago;
	if (out_days_ago)
		*out_days_ago = days_ago;
	if (out_weeks_ago)
		*out_weeks_ago = weeks_ago;
	if (out_months_ago)
		*out_months_ago = months_ago;
	if (out_years_ago)
		*out_years_ago = years_ago;

	return TRUE;
}

/**
 * gs_utils_time_to_string:
 * @unix_time_seconds: Time since the epoch in seconds
 *
 * Converts a time to a string such as "5 minutes ago" or "2 weeks ago"
 *
 * Returns: (transfer full): the time string, or %NULL if @unix_time_seconds is
 *   not valid
 */
gchar *
gs_utils_time_to_string (gint64 unix_time_seconds)
{
	gint minutes_ago, hours_ago, days_ago;
	gint weeks_ago, months_ago, years_ago;

	if (!gs_utils_split_time_difference (unix_time_seconds,
		&minutes_ago, &hours_ago, &days_ago,
		&weeks_ago, &months_ago, &years_ago))
		return NULL;

	if (minutes_ago < 5) {
		/* TRANSLATORS: something happened less than 5 minutes ago */
		return g_strdup (_("Just now"));
	} else if (hours_ago < 1)
		return g_strdup_printf (ngettext ("%d minute ago",
						  "%d minutes ago", minutes_ago),
					minutes_ago);
	else if (days_ago < 1)
		return g_strdup_printf (ngettext ("%d hour ago",
						  "%d hours ago", hours_ago),
					hours_ago);
	else if (days_ago < 15)
		return g_strdup_printf (ngettext ("%d day ago",
						  "%d days ago", days_ago),
					days_ago);
	else if (weeks_ago < 8)
		return g_strdup_printf (ngettext ("%d week ago",
						  "%d weeks ago", weeks_ago),
					weeks_ago);
	else if (years_ago < 1)
		return g_strdup_printf (ngettext ("%d month ago",
						  "%d months ago", months_ago),
					months_ago);
	else
		return g_strdup_printf (ngettext ("%d year ago",
						  "%d years ago", years_ago),
					years_ago);
}

static void
gs_utils_reboot_call_done_cb (GObject *source,
			      GAsyncResult *res,
			      gpointer user_data)
{
	g_autoptr(GError) local_error = NULL;

	/* get result */
	if (gs_utils_invoke_reboot_finish (source, res, &local_error))
		return;
	if (g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_CANCELLED))
		g_debug ("Calling reboot had been cancelled");
	else if (local_error != NULL)
		g_warning ("Calling reboot failed: %s", local_error->message);
}

static void
gs_utils_invoke_reboot_ready3_cb (GObject *source_object,
				  GAsyncResult *result,
				  gpointer user_data)
{
	g_autoptr(GTask) task = user_data;
	g_autoptr(GVariant) ret_val = NULL;
	g_autoptr(GError) local_error = NULL;

	ret_val = g_dbus_connection_call_finish (G_DBUS_CONNECTION (source_object), result, &local_error);
	if (ret_val != NULL) {
		g_task_return_boolean (task, TRUE);
	} else {
		const gchar *method_name = g_task_get_task_data (task);
		g_dbus_error_strip_remote_error (local_error);
		g_prefix_error (&local_error, "Failed to call %s: ", method_name);
		g_task_return_error (task, g_steal_pointer (&local_error));
	}
}

static void
gs_utils_invoke_reboot_ready2_got_session_bus_cb (GObject *source_object,
						  GAsyncResult *result,
						  gpointer user_data)
{
	g_autoptr(GTask) task = user_data;
	g_autoptr(GDBusConnection) bus = NULL;
	g_autoptr(GError) local_error = NULL;
	GCancellable *cancellable;

	bus = g_bus_get_finish (result, &local_error);
	if (bus == NULL) {
		g_dbus_error_strip_remote_error (local_error);
		g_prefix_error_literal (&local_error, "Failed to get D-Bus session bus: ");
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	cancellable = g_task_get_cancellable (task);

	/* Make sure file buffers are written to the disk before invoking reboot */
	sync ();

	g_task_set_task_data (task, (gpointer) "org.lingmo.SessionManager.Reboot", NULL);
	g_dbus_connection_call (bus,
				"org.lingmo.SessionManager",
				"/org/lingmo/SessionManager",
				"org.lingmo.SessionManager",
				"Reboot",
				NULL, NULL, G_DBUS_CALL_FLAGS_NONE,
				G_MAXINT, cancellable,
				gs_utils_invoke_reboot_ready3_cb,
				g_steal_pointer (&task));
}

static void
gs_utils_invoke_reboot_ready2_cb (GObject *source_object,
				  GAsyncResult *result,
				  gpointer user_data)
{
	g_autoptr(GTask) task = user_data;
	g_autoptr(GVariant) ret_val = NULL;
	g_autoptr(GError) local_error = NULL;

	ret_val = g_dbus_connection_call_finish (G_DBUS_CONNECTION (source_object), result, &local_error);
	if (ret_val != NULL) {
		g_task_return_boolean (task, TRUE);
	} else {
		GCancellable *cancellable;
		const gchar *method_name = g_task_get_task_data (task);

		g_dbus_error_strip_remote_error (local_error);
		g_prefix_error (&local_error, "Failed to call %s: ", method_name);

		if (g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
			g_task_return_error (task, g_steal_pointer (&local_error));
			return;
		}

		g_debug ("%s", local_error->message);
		g_clear_error (&local_error);

		cancellable = g_task_get_cancellable (task);

		g_bus_get (G_BUS_TYPE_SESSION, cancellable,
			   gs_utils_invoke_reboot_ready2_got_session_bus_cb,
			   g_steal_pointer (&task));
	}
}

static void
gs_utils_invoke_reboot_ready1_got_system_bus_cb (GObject *source_object,
						  GAsyncResult *result,
						  gpointer user_data)
{
	g_autoptr(GTask) task = user_data;
	g_autoptr(GDBusConnection) bus = NULL;
	g_autoptr(GError) local_error = NULL;
	GCancellable *cancellable;

	bus = g_bus_get_finish (result, &local_error);
	if (bus == NULL) {
		g_dbus_error_strip_remote_error (local_error);
		g_prefix_error_literal (&local_error, "Failed to get D-Bus system bus: ");
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	cancellable = g_task_get_cancellable (task);

	/* Make sure file buffers are written to the disk before invoking reboot */
	sync ();

	g_task_set_task_data (task, (gpointer) "org.freedesktop.login1.Manager.Reboot", NULL);
	g_dbus_connection_call (bus,
				"org.freedesktop.login1",
				"/org/freedesktop/login1",
				"org.freedesktop.login1.Manager",
				"Reboot",
				g_variant_new ("(b)", TRUE), /* interactive */
				NULL, G_DBUS_CALL_FLAGS_NONE,
				G_MAXINT, cancellable,
				gs_utils_invoke_reboot_ready2_cb,
				g_steal_pointer (&task));
}

static void
gs_utils_invoke_reboot_ready1_cb (GObject *source_object,
				  GAsyncResult *result,
				  gpointer user_data)
{
	g_autoptr(GTask) task = user_data;
	g_autoptr(GVariant) ret_val = NULL;
	g_autoptr(GError) local_error = NULL;

	ret_val = g_dbus_connection_call_finish (G_DBUS_CONNECTION (source_object), result, &local_error);
	if (ret_val != NULL) {
		g_task_return_boolean (task, TRUE);
	} else {
		GCancellable *cancellable;
		const gchar *method_name = g_task_get_task_data (task);

		g_dbus_error_strip_remote_error (local_error);
		g_prefix_error (&local_error, "Failed to call %s: ", method_name);

		if (g_error_matches (local_error, G_IO_ERROR, G_IO_ERROR_CANCELLED)) {
			g_task_return_error (task, g_steal_pointer (&local_error));
			return;
		}

		g_debug ("%s", local_error->message);
		g_clear_error (&local_error);

		cancellable = g_task_get_cancellable (task);

		g_bus_get (G_BUS_TYPE_SYSTEM, cancellable,
			   gs_utils_invoke_reboot_ready1_got_system_bus_cb,
			   g_steal_pointer (&task));
	}
}

static void
gs_utils_invoke_reboot_got_session_bus_cb (GObject *source_object,
					   GAsyncResult *result,
					   gpointer user_data)
{
	g_autoptr(GTask) task = user_data;
	g_autoptr(GDBusConnection) bus = NULL;
	g_autoptr(GError) local_error = NULL;
	GCancellable *cancellable;
	const gchar *xdg_desktop;
	gboolean call_session_manager = FALSE;

	bus = g_bus_get_finish (result, &local_error);
	if (bus == NULL) {
		g_dbus_error_strip_remote_error (local_error);
		g_prefix_error_literal (&local_error, "Failed to get D-Bus session bus: ");
		g_task_return_error (task, g_steal_pointer (&local_error));
		return;
	}

	/* Make sure file buffers are written to the disk before invoking reboot */
	sync ();

	cancellable = g_task_get_cancellable (task);

	xdg_desktop = g_getenv ("XDG_CURRENT_DESKTOP");
	if (xdg_desktop != NULL) {
		if (strstr (xdg_desktop, "KDE")) {
			g_task_set_task_data (task, (gpointer) "org.kde.Shutdown.logoutAndReboot", NULL);
			g_dbus_connection_call (bus,
						"org.kde.Shutdown",
						"/Shutdown",
						"org.kde.Shutdown",
						"logoutAndReboot",
						NULL, NULL, G_DBUS_CALL_FLAGS_NONE,
						G_MAXINT, cancellable,
						gs_utils_invoke_reboot_ready1_cb,
						g_steal_pointer (&task));
		} else if (strstr (xdg_desktop, "LXDE")) {
			g_task_set_task_data (task, (gpointer) "org.lxde.SessionManager.RequestReboot", NULL);
			g_dbus_connection_call (bus,
						"org.lxde.SessionManager",
						"/org/lxde/SessionManager",
						"org.lxde.SessionManager",
						"RequestReboot",
						NULL, NULL, G_DBUS_CALL_FLAGS_NONE,
						G_MAXINT, cancellable,
						gs_utils_invoke_reboot_ready1_cb,
						g_steal_pointer (&task));
		} else if (strstr (xdg_desktop, "MATE")) {
			g_task_set_task_data (task, (gpointer) "org.lingmo.SessionManager.RequestReboot", NULL);
			g_dbus_connection_call (bus,
						"org.lingmo.SessionManager",
						"/org/lingmo/SessionManager",
						"org.lingmo.SessionManager",
						"RequestReboot",
						NULL, NULL, G_DBUS_CALL_FLAGS_NONE,
						G_MAXINT, cancellable,
						gs_utils_invoke_reboot_ready1_cb,
						g_steal_pointer (&task));
		} else if (strstr (xdg_desktop, "XFCE")) {
			g_task_set_task_data (task, (gpointer) "org.xfce.Session.Manager.Restart", NULL);
			g_dbus_connection_call (bus,
						"org.xfce.SessionManager",
						"/org/xfce/SessionManager",
						"org.xfce.Session.Manager",
						"Restart",
						g_variant_new ("(b)", TRUE), /* allow_save */
						NULL, G_DBUS_CALL_FLAGS_NONE,
						G_MAXINT, cancellable,
						gs_utils_invoke_reboot_ready1_cb,
						g_steal_pointer (&task));
		} else {
			/* Let the "LINGMO" and "X-Cinnamon" be the default */
			call_session_manager = TRUE;
		}
	} else {
		call_session_manager = TRUE;
	}

	if (call_session_manager) {
		g_task_set_task_data (task, (gpointer) "org.lingmo.SessionManager.Reboot", NULL);
		g_dbus_connection_call (bus,
					"org.lingmo.SessionManager",
					"/org/lingmo/SessionManager",
					"org.lingmo.SessionManager",
					"Reboot",
					NULL, NULL, G_DBUS_CALL_FLAGS_NONE,
					G_MAXINT, cancellable,
					gs_utils_invoke_reboot_ready3_cb,
					g_steal_pointer (&task));
	}
}

/**
 * gs_utils_invoke_reboot_async:
 * @cancellable: (nullable): a %GCancellable for the call, or %NULL
 * @ready_callback: (nullable): a callback to be called after the call is finished, or %NULL
 * @user_data: user data for the @ready_callback
 *
 * Asynchronously invokes a reboot request. Finish the operation
 * with gs_utils_invoke_reboot_finish().
 *
 * When the @ready_callback is %NULL, a default callback is used, which shows
 * a runtime warning (using g_warning) on the console when the call fails.
 *
 * Since: 41
 **/
void
gs_utils_invoke_reboot_async (GCancellable *cancellable,
			      GAsyncReadyCallback ready_callback,
			      gpointer user_data)
{
	g_autoptr(GTask) task = NULL;

	if (!ready_callback)
		ready_callback = gs_utils_reboot_call_done_cb;

	task = g_task_new (NULL, cancellable, ready_callback, user_data);
	g_task_set_source_tag (task, gs_utils_invoke_reboot_async);

	g_bus_get (G_BUS_TYPE_SESSION, cancellable,
		   gs_utils_invoke_reboot_got_session_bus_cb,
		   g_steal_pointer (&task));
}

/**
 * gs_utils_invoke_reboot_finish:
 * @source_object: the source object provided in the ready callback
 * @result: the result object provided in the ready callback
 * @error: a #GError, or %NULL
 *
 * Finishes gs_utils_invoke_reboot_async() call.
 *
 * Returns: Whether succeeded. If failed, the @error is set.
 *
 * Since: 43
 **/
gboolean
gs_utils_invoke_reboot_finish (GObject *source_object,
			       GAsyncResult *result,
			       GError **error)
{
	g_return_val_if_fail (G_IS_TASK (result), FALSE);
	g_return_val_if_fail (g_task_is_valid (result, source_object), FALSE);
	g_return_val_if_fail (g_task_get_source_tag (G_TASK (result)) == gs_utils_invoke_reboot_async, FALSE);

	return g_task_propagate_boolean (G_TASK (result), error);
}

/**
 * gs_utils_format_size:
 * @size_bytes: size to format, in bytes
 * @out_is_markup: (out) (not nullable): stores whther the returned string is a markup
 *
 * Similar to `g_format_size`, only splits the value and the unit into
 * separate parts and draws the unit with a smaller font, in case
 * the relevant code is available in GLib while compiling.
 *
 * The @out_is_markup is always set, providing the information about
 * used format of the returned string.
 *
 * Returns: (transfer full): a new string, containing the @size_bytes formatted as string
 *
 * Since: 43
 **/
gchar *
gs_utils_format_size (guint64 size_bytes,
		      gboolean *out_is_markup)
{
#ifdef HAVE_G_FORMAT_SIZE_ONLY_VALUE
	g_autofree gchar *value_str = g_format_size_full (size_bytes, G_FORMAT_SIZE_ONLY_VALUE);
	g_autofree gchar *unit_str = g_format_size_full (size_bytes, G_FORMAT_SIZE_ONLY_UNIT);
	g_autofree gchar *value_escaped = g_markup_escape_text (value_str, -1);
	g_autofree gchar *unit_escaped = g_markup_printf_escaped ("<span font_size='x-small'>%s</span>", unit_str);
	g_return_val_if_fail (out_is_markup != NULL, NULL);
	*out_is_markup = TRUE;
	/* Translators: This is to construct a disk size string consisting of the value and its unit, while
	 * the unit is drawn with a smaller font. If you need to flip the order, then you can use "%2$s %1$s".
	 * Make sure you'll preserve the no break space between the values.
	 * Example result: "13.0 MB" */
	return g_strdup_printf (C_("format-size", "%s\302\240%s"), value_escaped, unit_escaped);
#else /* HAVE_G_FORMAT_SIZE_ONLY_VALUE */
	g_return_val_if_fail (out_is_markup != NULL, NULL);
	*out_is_markup = FALSE;
	return g_format_size (size_bytes);
#endif /* HAVE_G_FORMAT_SIZE_ONLY_VALUE */
}

/**
 * gs_show_uri:
 * @parent: (nullable): parent window
 * @uri: the uri to show
 *
 * This function launches the default application for showing
 * a given uri, or shows an error dialog if that fails.
 *
 * Since: 44
 **/
void
gs_show_uri (GtkWindow *parent,
	     const char *uri)
{
	g_autoptr (GFile) file = NULL;
	g_autoptr (GtkFileLauncher) launcher = NULL;

	file = g_file_new_for_uri (uri);
	launcher = gtk_file_launcher_new (file);
	gtk_file_launcher_launch (launcher, parent, NULL, NULL, NULL);
}

/**
 * gs_utils_get_app_data_dir:
 * @app: a #GsApp
 *
 * Returns @app's data directory, if known. It also checks
 * whether the directory exists and returns %NULL, when not.
 *
 * Returns: (nullable) (transfer full): existing @app's data directory,
 *    or #NULL, when not known or does not exist
 *
 * Since: 45
 **/
gchar *
gs_utils_get_app_data_dir (GsApp *app)
{
	g_return_val_if_fail (GS_IS_APP (app), NULL);

	if (gs_app_get_id (app) == NULL)
		return NULL;

	/* do this only for Flatpak for now */
	if (gs_app_get_bundle_kind (app) == AS_BUNDLE_KIND_FLATPAK) {
		g_autofree gchar *data_dir = g_build_filename (g_get_home_dir (), ".var", "app", gs_app_get_id (app), NULL);
		if (g_file_test (data_dir, G_FILE_TEST_EXISTS))
			return g_steal_pointer (&data_dir);
		return NULL;
	}

	return NULL;
}

/**
 * gs_utils_remove_app_data_dir:
 * @app: a #GsApp
 * @plugin_loader: a #GsPluginLoader where to pass any errors
 *
 * Removes @app's data dir, as returned from gs_utils_get_app_data_dir().
 * The function does nothing when the app has no or unknown data dir.
 * Any errors of the removal are passed to the @plugin_loader, while
 * it's not an error when the directory does not exist.
 *
 * Returns: %TRUE, when @app's data dir had been successfully removed
 *
 * Since: 45
 **/
gboolean
gs_utils_remove_app_data_dir (GsApp *app,
			      GsPluginLoader *plugin_loader)
{
	g_autofree gchar *dir = NULL;
	g_autoptr(GError) error = NULL;

	g_return_val_if_fail (GS_IS_APP (app), FALSE);
	g_return_val_if_fail (GS_IS_PLUGIN_LOADER (plugin_loader), FALSE);

	dir = gs_utils_get_app_data_dir (app);
	if (dir == NULL)
		return FALSE;

	gs_utils_rmtree (dir, &error);

	if (error != NULL && !g_error_matches (error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND)) {
		gs_utils_error_convert_gio (&error);
		gs_plugin_loader_claim_error (plugin_loader,  NULL, GS_PLUGIN_ACTION_UNKNOWN,
					      app, TRUE, error);
		return FALSE;
	}

	return TRUE;
}
