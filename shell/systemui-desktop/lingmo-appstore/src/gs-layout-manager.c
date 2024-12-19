/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Endless OS Foundation LLC
 *
 * Author: Georges Basile Stavracas Neto <georges.stavracas@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include "gs-layout-manager.h"

/*
 * The GsLayoutManager is a copy of the GtkBoxLayout, only
 * declared as a derivable class, to avoid code duplication.
 */

G_DEFINE_TYPE (GsLayoutManager, gs_layout_manager, GTK_TYPE_LAYOUT_MANAGER)

static void
gs_layout_manager_measure (GtkLayoutManager *layout_manager,
			   GtkWidget        *widget,
			   GtkOrientation    orientation,
			   gint              for_size,
			   gint             *minimum,
			   gint             *natural,
			   gint             *minimum_baseline,
			   gint             *natural_baseline)
{
	GtkWidget *child;
	gint min = 0;
	gint nat = 0;

	for (child = gtk_widget_get_first_child (widget);
	     child != NULL;
	     child = gtk_widget_get_next_sibling (child)) {
		gint child_min_baseline = -1;
		gint child_nat_baseline = -1;
		gint child_min = 0;
		gint child_nat = 0;

		if (!gtk_widget_should_layout (child))
			continue;

		gtk_widget_measure (child, orientation,
				    for_size,
				    &child_min, &child_nat,
				    &child_min_baseline,
				    &child_nat_baseline);

		min = MAX (min, child_min);
		nat = MAX (nat, child_nat);

		if (child_min_baseline > -1)
			*minimum_baseline = MAX (*minimum_baseline, child_min_baseline);
		if (child_nat_baseline > -1)
		    *natural_baseline = MAX (*natural_baseline, child_nat_baseline);
	}

	*minimum = min;
	*natural = nat;
}

static void
gs_layout_manager_allocate (GtkLayoutManager *layout_manager,
			    GtkWidget        *widget,
			    gint              width,
			    gint              height,
			    gint              baseline)
{
	GtkWidget *child;

	for (child = gtk_widget_get_first_child (widget);
	     child != NULL;
	     child = gtk_widget_get_next_sibling (child)) {
		if (child && gtk_widget_should_layout (child))
			gtk_widget_allocate (child, width, height, baseline, NULL);
	}
}

static void
gs_layout_manager_class_init (GsLayoutManagerClass *klass)
{
	GtkLayoutManagerClass *layout_manager_class = GTK_LAYOUT_MANAGER_CLASS (klass);

	layout_manager_class->measure = gs_layout_manager_measure;
	layout_manager_class->allocate = gs_layout_manager_allocate;
}

static void
gs_layout_manager_init (GsLayoutManager *self)
{
}

/**
 * gs_layout_manager_new:
 *
 * Create a new #GsLayoutManager.
 *
 * Returns: (transfer full): a new #GsLayoutManager
 *
 * Since: 43
 **/
GtkLayoutManager *
gs_layout_manager_new (void)
{
	return g_object_new (GS_TYPE_LAYOUT_MANAGER, NULL);
}
