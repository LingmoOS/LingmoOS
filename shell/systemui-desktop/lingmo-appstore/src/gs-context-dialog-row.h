/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Endless OS Foundation LLC
 *
 * Author: Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <adwaita.h>
#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

/**
 * GsContextDialogRowImportance:
 * @GS_CONTEXT_DIALOG_ROW_IMPORTANCE_NEUTRAL: neutral or unknown importance
 * @GS_CONTEXT_DIALOG_ROW_IMPORTANCE_UNIMPORTANT: unimportant
 * @GS_CONTEXT_DIALOG_ROW_IMPORTANCE_INFORMATION: a notice-like importance (Since: 45)
 * @GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING: a bit important
 * @GS_CONTEXT_DIALOG_ROW_IMPORTANCE_IMPORTANT: definitely important
 *
 * The importance of the information in a #GsContextDialogRow. The values
 * increase from less important to more important.
 *
 * Since: 41
 */
typedef enum
{
	/* The code in this file relies on the fact that these enum values
	 * numerically increase as they get more important. */
	GS_CONTEXT_DIALOG_ROW_IMPORTANCE_NEUTRAL,
	GS_CONTEXT_DIALOG_ROW_IMPORTANCE_UNIMPORTANT,
	GS_CONTEXT_DIALOG_ROW_IMPORTANCE_INFORMATION,
	GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING,
	GS_CONTEXT_DIALOG_ROW_IMPORTANCE_IMPORTANT,
} GsContextDialogRowImportance;

#define GS_TYPE_CONTEXT_DIALOG_ROW (gs_context_dialog_row_get_type ())

G_DECLARE_FINAL_TYPE (GsContextDialogRow, gs_context_dialog_row, GS, CONTEXT_DIALOG_ROW, AdwActionRow)

GtkListBoxRow	*gs_context_dialog_row_new	(const gchar			*icon_name,
						 GsContextDialogRowImportance	 importance,
						 const gchar			*title,
						 const gchar			*description);
GtkListBoxRow	*gs_context_dialog_row_new_text	(const gchar			*content,
						 GsContextDialogRowImportance	 importance,
						 const gchar			*title,
						 const gchar			*description);

const gchar			*gs_context_dialog_row_get_icon_name	(GsContextDialogRow	*self);
const gchar			*gs_context_dialog_row_get_content	(GsContextDialogRow	*self);
GsContextDialogRowImportance	 gs_context_dialog_row_get_importance	(GsContextDialogRow	*self);
gboolean			 gs_context_dialog_row_get_content_is_markup
									(GsContextDialogRow	*self);
void				 gs_context_dialog_row_set_content_markup
									(GsContextDialogRow	*self,
									 const gchar		*markup);

void				 gs_context_dialog_row_set_size_groups	(GsContextDialogRow	*self,
									 GtkSizeGroup		*lozenge,
									 GtkSizeGroup		*title,
									 GtkSizeGroup		*description);

G_END_DECLS
