/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2016 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * SECTION:gs-plugin-event
 * @title: GsPluginEvent
 * @include: lingmo-appstore.h
 * @stability: Unstable
 * @short_description: Information about a plugin event
 *
 * These functions provide a way for plugins to tell the UI layer about events
 * that may require displaying to the user. Plugins should not assume that a
 * specific event is actually shown to the user as it may be ignored
 * automatically.
 */

#include "config.h"

#include <glib.h>

#include "gs-enums.h"
#include "gs-plugin-private.h"
#include "gs-plugin-event.h"
#include "gs-plugin-job.h"
#include "gs-utils.h"

struct _GsPluginEvent
{
	GObject			 parent_instance;
	GsApp			*app;
	GsApp			*origin;
	GsPluginAction		 action;
	GsPluginJob		*job;  /* (owned) (nullable) */
	GError			*error;
	GsPluginEventFlag	 flags;
	gchar			*unique_id;
};

G_DEFINE_TYPE (GsPluginEvent, gs_plugin_event, G_TYPE_OBJECT)

typedef enum {
	PROP_APP = 1,
	PROP_ORIGIN,
	PROP_ACTION,
	PROP_JOB,
	PROP_ERROR,
} GsPluginEventProperty;

static GParamSpec *props[PROP_ERROR + 1] = { NULL, };

/**
 * gs_plugin_event_get_app:
 * @event: A #GsPluginEvent
 *
 * Gets an app that created the event.
 *
 * Returns: (transfer none): a #GsApp, or %NULL if unset
 *
 * Since: 3.22
 **/
GsApp *
gs_plugin_event_get_app (GsPluginEvent *event)
{
	g_return_val_if_fail (GS_IS_PLUGIN_EVENT (event), NULL);
	return event->app;
}

/**
 * gs_plugin_event_get_origin:
 * @event: A #GsPluginEvent
 *
 * Gets an origin that created the event.
 *
 * Returns: (transfer none): a #GsApp, or %NULL if unset
 *
 * Since: 3.22
 **/
GsApp *
gs_plugin_event_get_origin (GsPluginEvent *event)
{
	g_return_val_if_fail (GS_IS_PLUGIN_EVENT (event), NULL);
	return event->origin;
}

/**
 * gs_plugin_event_get_action:
 * @event: A #GsPluginEvent
 *
 * Gets an action that created the event.
 *
 * Returns: (transfer none): a #GsPluginAction, e.g. %GS_PLUGIN_ACTION_INSTALL
 *
 * Since: 3.22
 **/
GsPluginAction
gs_plugin_event_get_action (GsPluginEvent *event)
{
	g_return_val_if_fail (GS_IS_PLUGIN_EVENT (event), 0);
	return event->action;
}

/**
 * gs_plugin_event_get_job:
 * @event: A #GsPluginEvent
 *
 * Gets the job that created the event.
 *
 * Returns: (transfer none) (nullable): a #GsPluginJob
 *
 * Since: 42
 **/
GsPluginJob *
gs_plugin_event_get_job (GsPluginEvent *event)
{
	g_return_val_if_fail (GS_IS_PLUGIN_EVENT (event), NULL);
	return event->job;
}

/**
 * gs_plugin_event_get_unique_id:
 * @event: A #GsPluginEvent
 *
 * Gets the unique ID for the event. In most cases (if an app has been set)
 * this will just be the actual #GsApp unique-id. In the cases where only error
 * has been set a virtual (but plausible) ID will be generated.
 *
 * Returns: a string, or %NULL for invalid
 *
 * Since: 3.22
 **/
const gchar *
gs_plugin_event_get_unique_id (GsPluginEvent *event)
{
	/* just proxy */
	if (event->origin != NULL &&
	    gs_app_get_unique_id (event->origin) != NULL) {
		return gs_app_get_unique_id (event->origin);
	}
	if (event->app != NULL &&
	    gs_app_get_unique_id (event->app) != NULL) {
		return gs_app_get_unique_id (event->app);
	}

	/* generate from error */
	if (event->error != NULL) {
		if (event->unique_id == NULL) {
			g_autofree gchar *id = NULL;
			id = g_strdup_printf ("%s.error",
					      gs_plugin_error_to_string (event->error->code));
			event->unique_id = gs_utils_build_unique_id (AS_COMPONENT_SCOPE_UNKNOWN,
								     AS_BUNDLE_KIND_UNKNOWN,
								     NULL,
								     id,
								     NULL);
		}
		return event->unique_id;
	}

	/* failed */
	return NULL;
}

/**
 * gs_plugin_event_get_kind:
 * @event: A #GsPluginEvent
 * @flag: A #GsPluginEventFlag, e.g. %GS_PLUGIN_EVENT_FLAG_INVALID
 *
 * Adds a flag to the event.
 *
 * Since: 3.22
 **/
void
gs_plugin_event_add_flag (GsPluginEvent *event, GsPluginEventFlag flag)
{
	g_return_if_fail (GS_IS_PLUGIN_EVENT (event));
	event->flags |= flag;
}

/**
 * gs_plugin_event_set_kind:
 * @event: A #GsPluginEvent
 * @flag: A #GsPluginEventFlag, e.g. %GS_PLUGIN_EVENT_FLAG_INVALID
 *
 * Removes a flag from the event.
 *
 * Since: 3.22
 **/
void
gs_plugin_event_remove_flag (GsPluginEvent *event, GsPluginEventFlag flag)
{
	g_return_if_fail (GS_IS_PLUGIN_EVENT (event));
	event->flags &= ~flag;
}

/**
 * gs_plugin_event_has_flag:
 * @event: A #GsPluginEvent
 * @flag: A #GsPluginEventFlag, e.g. %GS_PLUGIN_EVENT_FLAG_INVALID
 *
 * Finds out if the event has a specific flag.
 *
 * Returns: %TRUE if the flag is set
 *
 * Since: 3.22
 **/
gboolean
gs_plugin_event_has_flag (GsPluginEvent *event, GsPluginEventFlag flag)
{
	g_return_val_if_fail (GS_IS_PLUGIN_EVENT (event), FALSE);
	return ((event->flags & flag) > 0);
}

/**
 * gs_plugin_event_get_error:
 * @event: A #GsPluginEvent
 *
 * Gets the event error.
 *
 * Returns: a #GError, or %NULL for unset
 *
 * Since: 3.22
 **/
const GError *
gs_plugin_event_get_error (GsPluginEvent *event)
{
	return event->error;
}

static void
gs_plugin_event_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
	GsPluginEvent *self = GS_PLUGIN_EVENT (object);

	switch ((GsPluginEventProperty) prop_id) {
	case PROP_APP:
		g_value_set_object (value, self->app);
		break;
	case PROP_ORIGIN:
		g_value_set_object (value, self->origin);
		break;
	case PROP_ACTION:
		g_value_set_enum (value, self->action);
		break;
	case PROP_JOB:
		g_value_set_object (value, self->job);
		break;
	case PROP_ERROR:
		g_value_set_boxed (value, self->error);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_plugin_event_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
	GsPluginEvent *self = GS_PLUGIN_EVENT (object);

	switch ((GsPluginEventProperty) prop_id) {
	case PROP_APP:
		/* Construct only. */
		g_assert (self->app == NULL);
		self->app = g_value_dup_object (value);
		g_object_notify_by_pspec (object, props[prop_id]);
		break;
	case PROP_ORIGIN:
		/* Construct only. */
		g_assert (self->origin == NULL);
		self->origin = g_value_dup_object (value);
		g_object_notify_by_pspec (object, props[prop_id]);
		break;
	case PROP_ACTION:
		/* Construct only. */
		g_assert (self->action == GS_PLUGIN_ACTION_UNKNOWN);
		self->action = g_value_get_enum (value);
		g_object_notify_by_pspec (object, props[prop_id]);
		break;
	case PROP_JOB:
		/* Construct only. */
		g_assert (self->job == NULL);
		self->job = g_value_dup_object (value);
		g_object_notify_by_pspec (object, props[prop_id]);
		break;
	case PROP_ERROR:
		/* Construct only. */
		g_assert (self->error == NULL);
		self->error = g_value_dup_boxed (value);
		if (self->error) {
			/* Just in case the caller left there any D-Bus remote error notes */
			g_dbus_error_strip_remote_error (self->error);
		}
		g_object_notify_by_pspec (object, props[prop_id]);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_plugin_event_dispose (GObject *object)
{
	GsPluginEvent *event = GS_PLUGIN_EVENT (object);

	g_clear_object (&event->app);
	g_clear_object (&event->origin);
	g_clear_object (&event->job);

	G_OBJECT_CLASS (gs_plugin_event_parent_class)->dispose (object);
}

static void
gs_plugin_event_finalize (GObject *object)
{
	GsPluginEvent *event = GS_PLUGIN_EVENT (object);

	g_clear_error (&event->error);
	g_free (event->unique_id);

	G_OBJECT_CLASS (gs_plugin_event_parent_class)->finalize (object);
}

static void
gs_plugin_event_class_init (GsPluginEventClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gs_plugin_event_get_property;
	object_class->set_property = gs_plugin_event_set_property;
	object_class->dispose = gs_plugin_event_dispose;
	object_class->finalize = gs_plugin_event_finalize;

	/**
	 * GsPluginEvent:app: (nullable)
	 *
	 * The application (or source, or whatever component) that caused the
	 * event to be created.
	 *
	 * Since: 42
	 */
	props[PROP_APP] =
		g_param_spec_object ("app", "App",
				     "The application (or source, or whatever component) that caused the event to be created.",
				     GS_TYPE_APP,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsPluginEvent:origin: (nullable)
	 *
	 * The origin that caused the event to be created.
	 *
	 * Since: 42
	 */
	props[PROP_ORIGIN] =
		g_param_spec_object ("origin", "Origin",
				     "The origin that caused the event to be created.",
				     GS_TYPE_APP,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsPluginEvent:action:
	 *
	 * The action that caused the event to be created.
	 *
	 * Since: 42
	 */
	props[PROP_ACTION] =
		g_param_spec_enum ("action", "Action",
				   "The action that caused the event to be created.",
				   GS_TYPE_PLUGIN_ACTION, GS_PLUGIN_ACTION_UNKNOWN,
				   G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				   G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsPluginEvent:job: (nullable)
	 *
	 * The job that caused the event to be created.
	 *
	 * Since: 42
	 */
	props[PROP_JOB] =
		g_param_spec_object ("job", "Job",
				     "The job that caused the event to be created.",
				     GS_TYPE_PLUGIN_JOB,
				     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				     G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	/**
	 * GsPluginEvent:error: (nullable)
	 *
	 * The error the event is reporting.
	 *
	 * Since: 42
	 */
	props[PROP_ERROR] =
		g_param_spec_boxed ("error", "Error",
				    "The error the event is reporting.",
				    G_TYPE_ERROR,
				    G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
				    G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (props), props);
}

static void
gs_plugin_event_init (GsPluginEvent *event)
{
}

/**
 * gs_plugin_event_new:
 * @first_property_name: the name of the first property
 * @...: the value of the first property, followed by zero or more pairs of
 *   property name/value pairs, then %NULL
 *
 * Creates a new event.
 *
 * The arguments are as for g_object_new(): property name/value pairs to set
 * the properties of the event.
 *
 * Returns: (transfer full): A newly allocated #GsPluginEvent
 *
 * Since: 42
 **/
GsPluginEvent *
gs_plugin_event_new (const gchar *first_property_name,
		     ...)
{
	GsPluginEvent *event;
	va_list args;

	va_start (args, first_property_name);
	event = GS_PLUGIN_EVENT (g_object_new_valist (GS_TYPE_PLUGIN_EVENT, first_property_name, args));
	va_end (args);

	return GS_PLUGIN_EVENT (event);
}
