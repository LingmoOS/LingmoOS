/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2013 Matthias Clasen <mclasen@redhat.com>
 * Copyright (C) 2019 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include "gs-app-tile.h"
#include "gs-common.h"

typedef struct {
	GsApp				*app;
	guint				 app_notify_idle_id;
} GsAppTilePrivate;

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (GsAppTile, gs_app_tile, GTK_TYPE_FLOW_BOX_CHILD)

typedef enum {
	PROP_APP = 1,
} GsAppTileProperty;

static GParamSpec *obj_props[PROP_APP + 1] = { NULL, };

static void
gs_app_tile_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	GsAppTile *self = GS_APP_TILE (object);
	GsAppTilePrivate *priv = gs_app_tile_get_instance_private (self);

	switch ((GsAppTileProperty) prop_id) {
	case PROP_APP:
		g_value_set_object (value, priv->app);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_app_tile_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	GsAppTile *self = GS_APP_TILE (object);

	switch ((GsAppTileProperty) prop_id) {
	case PROP_APP:
		gs_app_tile_set_app (self, g_value_get_object (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_app_tile_dispose (GObject *object)
{
	GsAppTile *self = GS_APP_TILE (object);

	gs_app_tile_set_app (self, NULL);

	G_OBJECT_CLASS (gs_app_tile_parent_class)->dispose (object);
}

/**
 * gs_app_tile_get_app:
 * @self: a #GsAppTile
 *
 * Get the value of #GsAppTile:app.
 *
 * Returns: (nullable) (transfer none): the #GsAppTile:app property
 */
GsApp *
gs_app_tile_get_app (GsAppTile *self)
{
	GsAppTilePrivate *priv = gs_app_tile_get_instance_private (self);
	g_return_val_if_fail (GS_IS_APP_TILE (self), NULL);
	return priv->app;
}

static gboolean
gs_app_tile_app_notify_idle_cb (gpointer user_data)
{
	GsAppTile *self = GS_APP_TILE (user_data);
	GsAppTileClass *klass = GS_APP_TILE_GET_CLASS (self);
	GsAppTilePrivate *priv = gs_app_tile_get_instance_private (self);

	priv->app_notify_idle_id = 0;
	klass->refresh (self);

	return G_SOURCE_REMOVE;
}

static void
gs_app_tile_app_notify_cb (GsApp *app, GParamSpec *pspec, GsAppTile *self)
{
	GsAppTilePrivate *priv = gs_app_tile_get_instance_private (self);

	/* Already pending */
	if (priv->app_notify_idle_id != 0)
		return;

	priv->app_notify_idle_id = g_idle_add (gs_app_tile_app_notify_idle_cb, self);
}

/**
 * gs_app_tile_set_app:
 * @self: a #GsAppTile
 * @app: (transfer none) (nullable): the new value for #GsAppTile:app
 *
 * Set the value of #GsAppTile:app.
 */
void
gs_app_tile_set_app (GsAppTile *self, GsApp *app)
{
	GsAppTileClass *klass = GS_APP_TILE_GET_CLASS (self);
	GsAppTilePrivate *priv = gs_app_tile_get_instance_private (self);

	g_return_if_fail (GS_IS_APP_TILE (self));
	g_return_if_fail (!app || GS_IS_APP (app));

	/* cancel pending refresh */
	g_clear_handle_id (&priv->app_notify_idle_id, g_source_remove);

	/* disconnect old app */
	if (priv->app != NULL)
		g_signal_handlers_disconnect_by_func (priv->app, gs_app_tile_app_notify_cb, self);
	g_set_object (&priv->app, app);

	/* optional refresh */
	if (klass->refresh != NULL && priv->app != NULL) {
		g_signal_connect (app, "notify",
				  G_CALLBACK (gs_app_tile_app_notify_cb), self);
		klass->refresh (self);
	}

	if (app)
		gtk_widget_add_css_class (GTK_WIDGET (self), "activatable");
	else
		gtk_widget_remove_css_class (GTK_WIDGET (self), "activatable");

	g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_APP]);
}

void
gs_app_tile_class_init (GsAppTileClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->get_property = gs_app_tile_get_property;
	object_class->set_property = gs_app_tile_set_property;
	object_class->dispose = gs_app_tile_dispose;

	/**
	 * GsAppTile:app: (nullable)
	 *
	 * The app to display in this tile.
	 *
	 * Set this to %NULL to display a loading/empty tile.
	 *
	 * Since: 41
	 */
	obj_props[PROP_APP] =
		g_param_spec_object ("app", "App",
				     "The app to display in this tile.",
				     GS_TYPE_APP,
				     G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);
}

void
gs_app_tile_init (GsAppTile *self)
{
	GsAppTilePrivate *priv = gs_app_tile_get_instance_private (self);
	priv->app_notify_idle_id = 0;

	gtk_widget_add_css_class (GTK_WIDGET (self), "card");
}
