/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2017 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <stdio.h>
#include <unistd.h>

#include "gs-os-release.h"
#include "gs-debug.h"

struct _GsDebug
{
	GObject		  parent_instance;

	gchar		**domains;  /* (owned) (nullable), read-only after construction, guaranteed to be %NULL if empty */
	gboolean	  verbose;  /* (atomic) */
	gboolean	  use_time;  /* read-only after construction */
};

G_DEFINE_TYPE (GsDebug, gs_debug, G_TYPE_OBJECT)

static GLogWriterOutput
gs_log_writer_console (GLogLevelFlags log_level,
		       const GLogField *fields,
		       gsize n_fields,
		       gpointer user_data)
{
	GsDebug *debug = GS_DEBUG (user_data);
	gboolean verbose;
	const gchar * const *domains = NULL;
	const gchar *log_domain = NULL;
	const gchar *log_message = NULL;
	g_autofree gchar *tmp = NULL;
	g_autoptr(GString) domain = NULL;

	domains = (const gchar * const *) debug->domains;
	verbose = g_atomic_int_get (&debug->verbose);

	/* check enabled, fast path without parsing fields */
	if ((log_level == G_LOG_LEVEL_DEBUG ||
	     log_level == G_LOG_LEVEL_INFO) &&
	    !verbose &&
	    debug->domains == NULL)
		return G_LOG_WRITER_HANDLED;

	/* get data from arguments */
	for (gsize i = 0; i < n_fields; i++) {
		if (g_strcmp0 (fields[i].key, "MESSAGE") == 0) {
			log_message = fields[i].value;
			continue;
		}
		if (g_strcmp0 (fields[i].key, "GLIB_DOMAIN") == 0) {
			log_domain = fields[i].value;
			continue;
		}
	}

	/* check enabled, slower path */
	if ((log_level == G_LOG_LEVEL_DEBUG ||
	     log_level == G_LOG_LEVEL_INFO) &&
	    !verbose &&
	    debug->domains != NULL &&
	    g_strcmp0 (debug->domains[0], "all") != 0 &&
	    (log_domain == NULL || !g_strv_contains (domains, log_domain)))
		return G_LOG_WRITER_HANDLED;

	/* this is really verbose */
	if ((g_strcmp0 (log_domain, "dconf") == 0 ||
	     g_strcmp0 (log_domain, "GLib-GIO") == 0 ||
	     g_strcmp0 (log_domain, "GLib-Net") == 0 ||
	     g_strcmp0 (log_domain, "GdkPixbuf") == 0) &&
	    log_level == G_LOG_LEVEL_DEBUG)
		return G_LOG_WRITER_HANDLED;

	/* time header */
	if (debug->use_time) {
		g_autoptr(GDateTime) dt = g_date_time_new_now_utc ();
		tmp = g_strdup_printf ("%02i:%02i:%02i:%03i",
				       g_date_time_get_hour (dt),
				       g_date_time_get_minute (dt),
				       g_date_time_get_second (dt),
				       g_date_time_get_microsecond (dt) / 1000);
	}

	/* make these shorter */
	if (g_strcmp0 (log_domain, "PackageKit") == 0) {
		log_domain = "PK";
	} else if (g_strcmp0 (log_domain, "GsPlugin") == 0) {
		log_domain = "Gs";
	}

	/* pad out domain */
	domain = g_string_new (log_domain);
	for (guint i = domain->len; i < 3; i++)
		g_string_append (domain, " ");

	switch (log_level) {
	case G_LOG_LEVEL_ERROR:
	case G_LOG_LEVEL_CRITICAL:
	case G_LOG_LEVEL_WARNING:
		/* to screen */
		if (isatty (fileno (stderr)) == 1) {
			/* critical in red */
			if (tmp != NULL)
				g_printerr ("%c[%dm%s ", 0x1B, 32, tmp);
			g_printerr ("%s ", domain->str);
			g_printerr ("%c[%dm%s\n%c[%dm", 0x1B, 31, log_message, 0x1B, 0);
		} else { /* to file */
			if (tmp != NULL)
				g_printerr ("%s ", tmp);
			g_printerr ("%s ", domain->str);
			g_printerr ("%s\n", log_message);
		}
		break;
	default:
		/* to screen */
		if (isatty (fileno (stdout)) == 1) {
			/* debug in blue */
			if (tmp != NULL)
				g_print ("%c[%dm%s ", 0x1B, 32, tmp);
			g_print ("%s ", domain->str);
			g_print ("%c[%dm%s\n%c[%dm", 0x1B, 34, log_message, 0x1B, 0);
			break;
		} else { /* to file */
			if (tmp != NULL)
				g_print ("%s ", tmp);
			g_print ("%s ", domain->str);
			g_print ("%s\n", log_message);
		}
	}

	/* success */
	return G_LOG_WRITER_HANDLED;
}

static GLogWriterOutput
gs_log_writer_journald (GLogLevelFlags log_level,
                        const GLogField *fields,
                        gsize n_fields,
                        gpointer user_data)
{
	GsDebug *debug = GS_DEBUG (user_data);

	switch (log_level) {
	case G_LOG_LEVEL_ERROR:
	case G_LOG_LEVEL_CRITICAL:
	case G_LOG_LEVEL_WARNING:
	case G_LOG_LEVEL_MESSAGE:
		/* important enough to force to the journal */
		return g_log_writer_journald (log_level, fields, n_fields, user_data);
		break;
	case G_LOG_LEVEL_INFO:
	case G_LOG_LEVEL_DEBUG:
		/* Not important enough unless verbose mode has been explicitly
		 * enabled. */
		if (g_atomic_int_get (&debug->verbose))
			return g_log_writer_journald (log_level, fields, n_fields, user_data);
		else
			return G_LOG_WRITER_HANDLED;
	default:
		break;
	}

	return G_LOG_WRITER_UNHANDLED;
}

static GLogWriterOutput
gs_debug_log_writer (GLogLevelFlags log_level,
		     const GLogField *fields,
		     gsize n_fields,
		     gpointer user_data)
{
	if (g_log_writer_is_journald (fileno (stderr)))
		return gs_log_writer_journald (log_level, fields, n_fields, user_data);
	else
		return gs_log_writer_console (log_level, fields, n_fields, user_data);
}

static void
gs_debug_finalize (GObject *object)
{
	GsDebug *debug = GS_DEBUG (object);

	g_clear_pointer (&debug->domains, g_strfreev);

	G_OBJECT_CLASS (gs_debug_parent_class)->finalize (object);
}

static void
gs_debug_class_init (GsDebugClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = gs_debug_finalize;
}

static void
gs_debug_init (GsDebug *debug)
{
	g_log_set_writer_func (gs_debug_log_writer,
			       g_object_ref (debug),
			       (GDestroyNotify) g_object_unref);
}

/**
 * gs_debug_new:
 * @domains: (transfer full) (nullable): a #GStrv of debug log domains to output,
 *     or `{ "all", NULL }` to output all debug log domains; %NULL is equivalent
 *     to an empty array
 * @verbose: whether to output log debug messages
 * @use_time: whether to output a timestamp with each log message
 *
 * Create a new #GsDebug with the given configuration.
 *
 * Ownership of @domains is transferred to this function. It will be freed with
 * g_strfreev() when the #GsDebug is destroyed.
 *
 * Returns: (transfer full): a new #GsDebug
 * Since: 40
 */
GsDebug *
gs_debug_new (gchar    **domains,
              gboolean   verbose,
              gboolean   use_time)
{
	g_autoptr(GsDebug) debug = g_object_new (GS_TYPE_DEBUG, NULL);

	/* Strictly speaking these should be set before g_log_set_writer_func()
	 * is called, but threads probably haven’t been started at this point. */
	debug->domains = (domains != NULL && domains[0] != NULL) ? g_steal_pointer (&domains) : NULL;
	debug->verbose = verbose;
	debug->use_time = use_time;

	return g_steal_pointer (&debug);
}

/**
 * gs_debug_new_from_environment:
 *
 * Create a new #GsDebug with its configuration loaded from environment
 * variables.
 *
 * Returns: (transfer full): a new #GsDebug
 * Since: 40
 */
GsDebug *
gs_debug_new_from_environment (void)
{
	g_auto(GStrv) domains = NULL;
	gboolean verbose, use_time;

	if (g_getenv ("G_MESSAGES_DEBUG") != NULL) {
		domains = g_strsplit (g_getenv ("G_MESSAGES_DEBUG"), " ", -1);
		if (domains[0] == NULL)
			g_clear_pointer (&domains, g_strfreev);
	}

	verbose = (g_getenv ("GS_DEBUG") != NULL);
	use_time = (g_getenv ("GS_DEBUG_NO_TIME") == NULL);

	return gs_debug_new (g_steal_pointer (&domains), verbose, use_time);
}

/**
 * gs_debug_set_verbose:
 * @self: a #GsDebug
 * @verbose: whether to output log debug messages
 *
 * Enable or disable verbose logging mode.
 *
 * This can be called at any time, from any thread.
 *
 * Since: 40
 */
void
gs_debug_set_verbose (GsDebug  *self,
                      gboolean  verbose)
{
	g_return_if_fail (GS_IS_DEBUG (self));

	/* If we’re changing from !verbose → verbose, print OS information.
	 * This is helpful in verbose logs when people file bug reports. */
	if (g_atomic_int_compare_and_exchange (&self->verbose, !verbose, verbose) &&
	    verbose) {
		g_autoptr(GsOsRelease) os_release = NULL;
		g_autoptr(GError) error = NULL;

		g_debug (PACKAGE_NAME " " PACKAGE_VERSION);

		os_release = gs_os_release_new (&error);
		if (os_release) {
			g_debug ("OS: %s; %s",
				gs_os_release_get_name (os_release),
				gs_os_release_get_version (os_release));
		} else {
			g_debug ("Failed to get OS Release information: %s", error->message);
		}
	}
}
