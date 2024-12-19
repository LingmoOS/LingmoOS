/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2008-2013 Richard Hughes <richard@hughsie.com>
 * Copyright (C) 2015 Kalev Lember <klember@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib-object.h>

G_BEGIN_DECLS

#define GS_TYPE_MARKDOWN (gs_markdown_get_type ())

G_DECLARE_FINAL_TYPE (GsMarkdown, gs_markdown, GS, MARKDOWN, GObject)

typedef enum {
	GS_MARKDOWN_OUTPUT_TEXT,
	GS_MARKDOWN_OUTPUT_PANGO,
	GS_MARKDOWN_OUTPUT_HTML,
	GS_MARKDOWN_OUTPUT_LAST
} GsMarkdownOutputKind;

GsMarkdown	*gs_markdown_new			(GsMarkdownOutputKind	 output);
void		 gs_markdown_set_max_lines		(GsMarkdown		*self,
							 gint			 max_lines);
void		 gs_markdown_set_smart_quoting		(GsMarkdown		*self,
							 gboolean		 smart_quoting);
void		 gs_markdown_set_escape			(GsMarkdown		*self,
							 gboolean		 escape);
void		 gs_markdown_set_autocode		(GsMarkdown		*self,
							 gboolean		 autocode);
void		 gs_markdown_set_autolinkify		(GsMarkdown		*self,
							 gboolean		 autolinkify);
gchar		*gs_markdown_parse			(GsMarkdown		*self,
							 const gchar		*text);

G_END_DECLS
