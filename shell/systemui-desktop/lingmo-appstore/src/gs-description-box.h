/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2020 Red Hat <www.redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GS_TYPE_DESCRIPTION_BOX (gs_description_box_get_type ())

G_DECLARE_FINAL_TYPE (GsDescriptionBox, gs_description_box, GS, DESCRIPTION_BOX, GtkWidget)

GtkWidget	*gs_description_box_new		(void);
const gchar	*gs_description_box_get_text	(GsDescriptionBox *box);
void		 gs_description_box_set_text	(GsDescriptionBox *box,
						 const gchar *text);
gboolean	 gs_description_box_get_collapsed
						(GsDescriptionBox *box);
void		 gs_description_box_set_collapsed
						(GsDescriptionBox *box,
						 gboolean collapsed);
gboolean	gs_description_box_get_always_expanded
						(GsDescriptionBox *box);
void		gs_description_box_set_always_expanded
						(GsDescriptionBox *box,
						 gboolean always_expanded);

G_END_DECLS
