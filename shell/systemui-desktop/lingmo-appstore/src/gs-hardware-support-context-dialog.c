/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Endless OS Foundation LLC
 *
 * Author: Philip Withnall <pwithnall@endlessos.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/**
 * SECTION:gs-hardware-support-context-dialog
 * @short_description: A dialog showing hardware support information about an app
 *
 * #GsHardwareSupportContextDialog is a dialog which shows detailed information
 * about what hardware an app requires or recommends to be used when running it.
 * For example, what input devices it requires, and what display sizes it
 * supports. This information is derived from the `<requires>`,
 * `<recommends>` and `<supports>` elements in the app’s appdata.
 *
 * Currently, `<supports>` is treated as a synonym of `<recommends>` as it’s
 * only just been introduced into the appstream standard, and many apps which
 * should be using `<supports>` are still using `<recommends>`.
 *
 * It is designed to show a more detailed view of the information which the
 * app’s hardware support tile in #GsAppContextBar is derived from.
 *
 * The widget has no special appearance if the app is unset, so callers will
 * typically want to hide the dialog in that case.
 *
 * Since: 41
 */

#include "config.h"

#include <adwaita.h>
#include <glib.h>
#include <glib-object.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <locale.h>

#include "gs-app.h"
#include "gs-common.h"
#include "gs-context-dialog-row.h"
#include "gs-hardware-support-context-dialog.h"
#include "gs-lozenge.h"

struct _GsHardwareSupportContextDialog
{
	GsInfoWindow		 parent_instance;

	GsApp			*app;  /* (nullable) (owned) */
	gulong			 app_notify_handler_relations;
	gulong			 app_notify_handler_name;

	GtkWidget		*lozenge;
	GtkLabel		*title;
	GtkListBox		*relations_list;
};

G_DEFINE_TYPE (GsHardwareSupportContextDialog, gs_hardware_support_context_dialog, GS_TYPE_INFO_WINDOW)

typedef enum {
	PROP_APP = 1,
} GsHardwareSupportContextDialogProperty;

static GParamSpec *obj_props[PROP_APP + 1] = { NULL, };

typedef enum {
	MATCH_STATE_NO_MATCH = 0,
	MATCH_STATE_MATCH = 1,
	MATCH_STATE_UNKNOWN,
} MatchState;

/* The `icon_name_*`, `title_*` and `description_*` arguments are all nullable.
 * If a row would be added with %NULL values, it is not added. */
static void
add_relation_row (GtkListBox                   *list_box,
                  GsContextDialogRowImportance *chosen_rating,
                  AsRelationKind                control_relation_kind,
                  MatchState                    match_state,
                  gboolean                      any_control_relations_set,
                  const gchar                  *icon_name_required_matches,
                  const gchar                  *title_required_matches,
                  const gchar                  *description_required_matches,
                  const gchar                  *icon_name_no_relation,
                  const gchar                  *title_no_relation,
                  const gchar                  *description_no_relation,
                  const gchar                  *icon_name_required_no_match,
                  const gchar                  *title_required_no_match,
                  const gchar                  *description_required_no_match,
                  const gchar                  *icon_name_recommends,
                  const gchar                  *title_recommends,
                  const gchar                  *description_recommends,
                  const gchar                  *icon_name_unsupported,
                  const gchar                  *title_unsupported,
                  const gchar                  *description_unsupported)
{
	GtkListBoxRow *row;
	GsContextDialogRowImportance rating;
	const gchar *icon_name, *title, *description;

	g_assert (control_relation_kind == AS_RELATION_KIND_UNKNOWN || any_control_relations_set);

	switch (control_relation_kind) {
	case AS_RELATION_KIND_UNKNOWN:
		if (!any_control_relations_set) {
			rating = GS_CONTEXT_DIALOG_ROW_IMPORTANCE_NEUTRAL;
			icon_name = icon_name_no_relation;
			title = title_no_relation;
			description = description_no_relation;
		} else {
			rating = GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING;
			icon_name = icon_name_unsupported;
			title = title_unsupported;
			description = description_unsupported;
		}
		break;
	case AS_RELATION_KIND_REQUIRES:
		if (match_state == MATCH_STATE_MATCH) {
			rating = GS_CONTEXT_DIALOG_ROW_IMPORTANCE_UNIMPORTANT;
			icon_name = icon_name_required_matches;
			title = title_required_matches;
			description = description_required_matches;
		} else {
			rating = (match_state == MATCH_STATE_NO_MATCH) ? GS_CONTEXT_DIALOG_ROW_IMPORTANCE_IMPORTANT : GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING;
			icon_name = icon_name_required_no_match;
			title = title_required_no_match;
			description = description_required_no_match;
		}
		break;
	case AS_RELATION_KIND_RECOMMENDS:
#if AS_CHECK_VERSION(0, 15, 0)
	case AS_RELATION_KIND_SUPPORTS:
#endif
		rating = GS_CONTEXT_DIALOG_ROW_IMPORTANCE_UNIMPORTANT;
		icon_name = icon_name_recommends;
		title = title_recommends;
		description = description_recommends;
		break;
	default:
		g_assert_not_reached ();
	}

	if (icon_name == NULL)
		return;

	if (rating > *chosen_rating)
		*chosen_rating = rating;

	row = gs_context_dialog_row_new (icon_name, rating, title, description);
	gtk_list_box_append (list_box, GTK_WIDGET (row));
}

/**
 * gs_hardware_support_context_dialog_get_largest_monitor:
 * @display: a #GdkDisplay
 *
 * Get the largest monitor associated with @display, comparing the larger of the
 * monitor’s width and height, and breaking ties between equally-large monitors
 * using gdk_monitor_is_primary().
 *
 * Returns: (nullable) (transfer none): the largest monitor from @display, or
 *     %NULL if no monitor information is available
 * Since: 41
 */
GdkMonitor *
gs_hardware_support_context_dialog_get_largest_monitor (GdkDisplay *display)
{
	GListModel *monitors;  /* (unowned) */
	GdkMonitor *monitor;  /* (unowned) */
	int monitor_max_dimension;
	guint n_monitors;

	g_return_val_if_fail (GDK_IS_DISPLAY (display), NULL);

	monitors = gdk_display_get_monitors (display);
	n_monitors = g_list_model_get_n_items (monitors);
	monitor_max_dimension = 0;
	monitor = NULL;

	for (guint i = 0; i < n_monitors; i++) {
		g_autoptr(GdkMonitor) monitor2 = g_list_model_get_item (monitors, i);
		GdkRectangle monitor_geometry;
		int monitor2_max_dimension;

		if (monitor2 == NULL)
			continue;

		gdk_monitor_get_geometry (monitor2, &monitor_geometry);
		monitor2_max_dimension = MAX (monitor_geometry.width, monitor_geometry.height);

		if (monitor2_max_dimension > monitor_max_dimension) {
			monitor = monitor2;
			monitor_max_dimension = monitor2_max_dimension;
			continue;
		}
	}

	return monitor;
}

/* Unfortunately the integer values of #AsRelationKind don’t have the same order
 * as we want. */
static AsRelationKind
max_relation_kind (AsRelationKind kind1,
                   AsRelationKind kind2)
{
	/* cases are ordered from maximum to minimum */
	if (kind1 == AS_RELATION_KIND_REQUIRES || kind2 == AS_RELATION_KIND_REQUIRES)
		return AS_RELATION_KIND_REQUIRES;
	if (kind1 == AS_RELATION_KIND_RECOMMENDS || kind2 == AS_RELATION_KIND_RECOMMENDS)
		return AS_RELATION_KIND_RECOMMENDS;
#if AS_CHECK_VERSION(0, 15, 0)
	if (kind1 == AS_RELATION_KIND_SUPPORTS || kind2 == AS_RELATION_KIND_SUPPORTS)
		return AS_RELATION_KIND_SUPPORTS;
#endif
	return AS_RELATION_KIND_UNKNOWN;
}

typedef struct {
	guint min;
	guint max;
} Range;

/*
 * evaluate_display_comparison:
 * @comparand1:
 * @comparator:
 * @comparand2:
 *
 * Evaluate `comparand1 comparator comparand2` and return the result. For
 * example, `comparand1 EQ comparand2` or `comparand1 GT comparand2`.
 *
 * Comparisons are done as ranges, so depending on @comparator, sometimes the
 * #Range.min value of a comparand is compared, sometimes #Range.max, and
 * sometimes both. See the code for details.
 *
 * Returns: %TRUE if the comparison is true, %FALSE otherwise
 * Since: 41
 */
static gboolean
evaluate_display_comparison (Range             comparand1,
                             AsRelationCompare comparator,
                             Range             comparand2)
{
	switch (comparator) {
	case AS_RELATION_COMPARE_EQ:
		return (comparand1.min == comparand2.min &&
			comparand1.max == comparand2.max);
	case AS_RELATION_COMPARE_NE:
		return (comparand1.min != comparand2.min ||
			comparand1.max != comparand2.max);
	case AS_RELATION_COMPARE_LT:
		return (comparand1.max < comparand2.min);
	case AS_RELATION_COMPARE_GT:
		return (comparand1.min > comparand2.max);
	case AS_RELATION_COMPARE_LE:
		return (comparand1.max <= comparand2.max);
	case AS_RELATION_COMPARE_GE:
		return (comparand1.min >= comparand2.min);
	case AS_RELATION_COMPARE_UNKNOWN:
	case AS_RELATION_COMPARE_LAST:
	default:
		g_assert_not_reached ();
	}
}

/**
 * gs_hardware_support_context_dialog_get_control_support:
 * @display: a #GdkDisplay
 * @relations: (element-type AsRelation): relations retrieved from a #GsApp
 *     using gs_app_get_relations()
 * @any_control_relations_set_out: (out caller-allocates) (optional): return
 *     location for a boolean indicating whether any control relations are set
 *     in @relations
 * @control_relations: (out caller-allocates) (array length=AS_CONTROL_KIND_LAST):
 *     array mapping #AsControlKind to #AsRelationKind; must be at least
 *     %AS_CONTROL_KIND_LAST elements long, doesn’t need to be initialised
 * @has_touchscreen_out: (out caller-allocates) (optional): return location for
 *     a boolean indicating whether @display has a touchscreen
 * @has_keyboard_out: (out caller-allocates) (optional): return location for
 *     a boolean indicating whether @display has a keyboard
 * @has_mouse_out: (out caller-allocates) (optional): return location for
 *     a boolean indicating whether @display has a mouse
 *
 * Query @display and @relations and summarise the information in the output
 * arguments.
 *
 * Each element of @control_relations will be set to the highest type of
 * relation seen for that type of control. So if the appdata represented by
 * @relations contains `<requires><control>keyboard</control></requires>`,
 * `control_relations[AS_CONTROL_KIND_KEYBOARD]` will be set to
 * %AS_RELATION_KIND_REQUIRES. All elements of @control_relations are set to
 * %AS_RELATION_KIND_UNKNOWN by default.
 *
 * @any_control_relations_set_out is set to %TRUE if any elements of
 * @control_relations are changed from %AS_RELATION_KIND_UNKNOWN.
 *
 * @has_touchscreen_out, @has_keyboard_out and @has_mouse_out are set to %TRUE
 * if the default seat attached to @display has the relevant input device
 * (%GDK_SEAT_CAPABILITY_TOUCH, %GDK_SEAT_CAPABILITY_KEYBOARD,
 * %GDK_SEAT_CAPABILITY_POINTER respectively).
 *
 * Since: 41
 */
void
gs_hardware_support_context_dialog_get_control_support (GdkDisplay     *display,
                                                        GPtrArray      *relations,
                                                        gboolean       *any_control_relations_set_out,
                                                        AsRelationKind *control_relations,
                                                        gboolean       *has_touchscreen_out,
                                                        gboolean       *has_keyboard_out,
                                                        gboolean       *has_mouse_out)
{
	gboolean any_control_relations_set;
	gboolean has_touchscreen, has_keyboard, has_mouse;

	g_return_if_fail (display == NULL || GDK_IS_DISPLAY (display));
	g_return_if_fail (control_relations != NULL);

	any_control_relations_set = FALSE;
	has_touchscreen = FALSE;
	has_keyboard = FALSE;
	has_mouse = FALSE;

	/* Initialise @control_relations */
	for (gint i = 0; i < AS_CONTROL_KIND_LAST; i++)
		control_relations[i] = AS_RELATION_KIND_UNKNOWN;

	/* Set @control_relations to the maximum relation kind found for each control */
	for (guint i = 0; relations != NULL && i < relations->len; i++) {
		AsRelation *relation = AS_RELATION (g_ptr_array_index (relations, i));
		AsRelationKind kind = as_relation_get_kind (relation);

		if (as_relation_get_item_kind (relation) == AS_RELATION_ITEM_KIND_CONTROL) {
			AsControlKind control_kind = as_relation_get_value_control_kind (relation);
			control_relations[control_kind] = MAX (control_relations[control_kind], kind);

			if (kind == AS_RELATION_KIND_REQUIRES ||
#if AS_CHECK_VERSION(0, 15, 0)
			    kind == AS_RELATION_KIND_SUPPORTS ||
#endif
			    kind == AS_RELATION_KIND_RECOMMENDS)
				any_control_relations_set = TRUE;
		}
	}

	/* Work out what input devices are available. */
	if (display != NULL) {
		GdkSeat *seat = gdk_display_get_default_seat (display);
		GdkSeatCapabilities seat_capabilities = gdk_seat_get_capabilities (seat);

		has_touchscreen = (seat_capabilities & GDK_SEAT_CAPABILITY_TOUCH);
		has_keyboard = (seat_capabilities & GDK_SEAT_CAPABILITY_KEYBOARD);
		has_mouse = (seat_capabilities & GDK_SEAT_CAPABILITY_POINTER);
	}

	if (any_control_relations_set_out != NULL)
		*any_control_relations_set_out = any_control_relations_set;
	if (has_touchscreen_out != NULL)
		*has_touchscreen_out = has_touchscreen;
	if (has_keyboard_out != NULL)
		*has_keyboard_out = has_keyboard;
	if (has_mouse_out != NULL)
		*has_mouse_out = has_mouse;
}

/**
 * gs_hardware_support_context_dialog_get_display_support:
 * @monitor: the largest #GdkMonitor currently connected
 * @relations: (element-type AsRelation): (element-type AsRelation): relations retrieved from a #GsApp
 *     using gs_app_get_relations()
 * @any_display_relations_set_out: (out caller-allocates) (optional): return
 *     location for a boolean indicating whether any display relations are set
 *     in @relations
 * @desktop_match_out: (out caller-allocates) (not optional): return location
 *     for a boolean indicating whether @relations claims support for desktop
 *     displays
 * @desktop_relation_kind_out: (out caller-allocates) (not optional): return
 *     location for an #AsRelationKind indicating what kind of support the app
 *     has for desktop displays
 * @mobile_match_out: (out caller-allocates) (not optional): return location
 *     for a boolean indicating whether @relations claims support for mobile
 *     displays (phones)
 * @mobile_relation_kind_out: (out caller-allocates) (not optional): return
 *     location for an #AsRelationKind indicating what kind of support the app
 *     has for mobile displays
 * @current_match_out: (out caller-allocates) (not optional): return location
 *     for a boolean indicating whether @relations claims support for the
 *     currently connected @monitor
 * @current_relation_kind_out: (out caller-allocates) (not optional): return
 *     location for an #AsRelationKind indicating what kind of support the app
 *     has for the currently connected monitor
 *
 * Query @monitor and @relations and summarise the information in the output
 * arguments.
 *
 * @any_display_relations_set_out is set to %TRUE if any elements of @relations
 * have type %AS_RELATION_ITEM_KIND_DISPLAY_LENGTH, i.e. if the app has provided
 * any information about what displays it supports/requires.
 *
 * @desktop_match_out is set to %TRUE if the display relations in @relations
 * indicate that the app supports desktop displays (currently, larger than
 * 1024 pixels).
 *
 * @desktop_relation_kind_out is set to the type of support the app has for
 * desktop displays: whether they’re required (%AS_RELATION_KIND_REQUIRES),
 * supported but not required (%AS_RELATION_KIND_RECOMMENDS or
 * %AS_RELATION_KIND_SUPPORTS) or whether there’s no information
 * (%AS_RELATION_KIND_UNKNOWN).
 *
 * @mobile_match_out and @mobile_relation_kind_out behave similarly, but for
 * mobile displays (smaller than 768 pixels).
 *
 * @current_match_out and @current_relation_kind_out behave similarly, but for
 * the dimensions of @monitor.
 *
 * Since: 41
 */
void
gs_hardware_support_context_dialog_get_display_support (GdkMonitor     *monitor,
                                                        GPtrArray      *relations,
                                                        gboolean       *any_display_relations_set_out,
                                                        gboolean       *desktop_match_out,
                                                        AsRelationKind *desktop_relation_kind_out,
                                                        gboolean       *mobile_match_out,
                                                        AsRelationKind *mobile_relation_kind_out,
                                                        gboolean       *current_match_out,
                                                        AsRelationKind *current_relation_kind_out)
{
	GdkRectangle current_screen_size;
	gboolean any_display_relations_set;

	g_return_if_fail (GDK_IS_MONITOR (monitor));
	g_return_if_fail (desktop_match_out != NULL);
	g_return_if_fail (desktop_relation_kind_out != NULL);
	g_return_if_fail (mobile_match_out != NULL);
	g_return_if_fail (mobile_relation_kind_out != NULL);
	g_return_if_fail (current_match_out != NULL);
	g_return_if_fail (current_relation_kind_out != NULL);

	gdk_monitor_get_geometry (monitor, &current_screen_size);

	/* Set default output */
	any_display_relations_set = FALSE;
	*desktop_match_out = FALSE;
	*desktop_relation_kind_out = AS_RELATION_KIND_UNKNOWN;
	*mobile_match_out = FALSE;
	*mobile_relation_kind_out = AS_RELATION_KIND_UNKNOWN;
	*current_match_out = FALSE;
	*current_relation_kind_out = AS_RELATION_KIND_UNKNOWN;

	for (guint i = 0; relations != NULL && i < relations->len; i++) {
		AsRelation *relation = AS_RELATION (g_ptr_array_index (relations, i));

		/* All lengths here are in logical/app pixels,
		 * not device pixels. */
		if (as_relation_get_item_kind (relation) == AS_RELATION_ITEM_KIND_DISPLAY_LENGTH) {
			AsRelationCompare comparator = as_relation_get_compare (relation);
			Range current_display_comparand, relation_comparand = { 0, G_MAXUINT };

#if !AS_CHECK_VERSION(1, 0, 0)
			/* From https://www.freedesktop.org/software/appstream/docs/chap-Metadata.html#tag-requires-recommends-display_length */
			Range display_lengths[] = {
				[AS_DISPLAY_LENGTH_KIND_XSMALL] = { 0, 360 },
				[AS_DISPLAY_LENGTH_KIND_SMALL] = { 360, 768 },
				[AS_DISPLAY_LENGTH_KIND_MEDIUM] = { 768, 1024 },
				[AS_DISPLAY_LENGTH_KIND_LARGE] = { 1024, 3840 },
				[AS_DISPLAY_LENGTH_KIND_XLARGE] = { 3840, G_MAXUINT },
			};
#else
			enum {
				AS_DISPLAY_LENGTH_KIND_SMALL,
				AS_DISPLAY_LENGTH_KIND_LARGE,
			};
			Range display_lengths[] = {
				[AS_DISPLAY_LENGTH_KIND_SMALL] = { 360, 768 },
				[AS_DISPLAY_LENGTH_KIND_LARGE] = { 1024, 3840 },
			};
#endif

			any_display_relations_set = TRUE;

			switch (as_relation_get_display_side_kind (relation)) {
			case AS_DISPLAY_SIDE_KIND_SHORTEST:
				current_display_comparand.min = current_display_comparand.max = MIN (current_screen_size.width, current_screen_size.height);
				relation_comparand.min = relation_comparand.max = as_relation_get_value_px (relation);
				break;
			case AS_DISPLAY_SIDE_KIND_LONGEST:
				current_display_comparand.min = current_display_comparand.max = MAX (current_screen_size.width, current_screen_size.height);
				relation_comparand.min = relation_comparand.max = as_relation_get_value_px (relation);
				break;
			case AS_DISPLAY_SIDE_KIND_UNKNOWN:
			case AS_DISPLAY_SIDE_KIND_LAST:
			default:
				current_display_comparand.min = current_display_comparand.max = MAX (current_screen_size.width, current_screen_size.height);
#if !AS_CHECK_VERSION(1, 0, 0)
				relation_comparand.min = display_lengths[as_relation_get_value_display_length_kind (relation)].min;
				relation_comparand.max = display_lengths[as_relation_get_value_display_length_kind (relation)].max;
#endif
				break;
			}

			if (evaluate_display_comparison (display_lengths[AS_DISPLAY_LENGTH_KIND_SMALL], comparator, relation_comparand)) {
				*mobile_relation_kind_out = max_relation_kind (*mobile_relation_kind_out, as_relation_get_kind (relation));
				*mobile_match_out = TRUE;
			}

			if (evaluate_display_comparison (display_lengths[AS_DISPLAY_LENGTH_KIND_LARGE], comparator, relation_comparand)) {
				*desktop_relation_kind_out = max_relation_kind (*desktop_relation_kind_out, as_relation_get_kind (relation));
				*desktop_match_out = TRUE;
			}

			if (evaluate_display_comparison (current_display_comparand, comparator, relation_comparand)) {
				*current_relation_kind_out = max_relation_kind (*current_relation_kind_out, as_relation_get_kind (relation));
				*current_match_out = TRUE;
			}
		}
	}

	/* Output */
	if (any_display_relations_set_out != NULL)
		*any_display_relations_set_out = any_display_relations_set;
}

static void
update_relations_list (GsHardwareSupportContextDialog *self)
{
	const gchar *icon_name, *css_class;
	g_autofree gchar *title = NULL;
	g_autoptr(GPtrArray) relations = NULL;
	AsRelationKind control_relations[AS_CONTROL_KIND_LAST] = { AS_RELATION_KIND_UNKNOWN, };
	GdkDisplay *display;
	GdkMonitor *monitor = NULL;
	GdkRectangle current_screen_size;
	gboolean any_control_relations_set;
	gboolean has_touchscreen = FALSE, has_keyboard = FALSE, has_mouse = FALSE;
	GsContextDialogRowImportance chosen_rating;

	/* Treat everything as unknown to begin with, and downgrade its hardware
	 * support based on app properties. */
	chosen_rating = GS_CONTEXT_DIALOG_ROW_IMPORTANCE_NEUTRAL;

	gs_widget_remove_all (GTK_WIDGET (self->relations_list), (GsRemoveFunc) gtk_list_box_remove);

	/* UI state is undefined if app is not set. */
	if (self->app == NULL)
		return;

	relations = gs_app_get_relations (self->app);

	/* Extract the %AS_RELATION_ITEM_KIND_CONTROL relations and summarise
	 * them. */
	display = gtk_widget_get_display (GTK_WIDGET (self));
	gs_hardware_support_context_dialog_get_control_support (display, relations,
								&any_control_relations_set,
								control_relations,
								&has_touchscreen,
								&has_keyboard,
								&has_mouse);

	if (display != NULL)
		monitor = gs_hardware_support_context_dialog_get_largest_monitor (display);

	if (monitor != NULL)
		gdk_monitor_get_geometry (monitor, &current_screen_size);

	/* For each of the screen sizes we understand, add a row to the dialogue.
	 * In the unlikely case that (monitor == NULL), don’t bother providing
	 * fallback rows. */
	if (monitor != NULL) {
		AsRelationKind desktop_relation_kind, mobile_relation_kind, current_relation_kind;
		gboolean desktop_match, mobile_match, current_match;
		gboolean any_display_relations_set;

		gs_hardware_support_context_dialog_get_display_support (monitor, relations,
									&any_display_relations_set,
									&desktop_match, &desktop_relation_kind,
									&mobile_match, &mobile_relation_kind,
									&current_match, &current_relation_kind);

		add_relation_row (self->relations_list, &chosen_rating,
				  desktop_relation_kind,
				  desktop_match ? MATCH_STATE_MATCH : MATCH_STATE_NO_MATCH,
				  any_display_relations_set,
				  "desktop-symbolic",
				  _("Desktop Support"),
				  _("Supports being used on a large screen"),
				  "dialog-question-symbolic",
				  _("Desktop Support Unknown"),
				  _("Not enough information to know if large screens are supported"),
				  "desktop-symbolic",
				  _("Desktop Only"),
				  _("Requires a large screen"),
				  "desktop-symbolic",
				  _("Desktop Support"),
				  _("Supports being used on a large screen"),
				  "desktop-symbolic",
				  _("Desktop Not Supported"),
				  _("Cannot be used on a large screen"));

		add_relation_row (self->relations_list, &chosen_rating,
				  mobile_relation_kind,
				  mobile_match ? MATCH_STATE_MATCH : MATCH_STATE_NO_MATCH,
				  any_display_relations_set,
				  "phone-symbolic",
				  _("Mobile Support"),
				  _("Supports being used on a small screen"),
				  "dialog-question-symbolic",
				  _("Mobile Support Unknown"),
				  _("Not enough information to know if small screens are supported"),
				  "phone-symbolic",
				  _("Mobile Only"),
				  _("Requires a small screen"),
				  "phone-symbolic",
				  _("Mobile Support"),
				  _("Supports being used on a small screen"),
				  "phone-symbolic",
				  _("Mobile Not Supported"),
				  _("Cannot be used on a small screen"));

		/* Other display relations should only be listed if they are a
		 * requirement. They will typically be for special apps. */
		add_relation_row (self->relations_list, &chosen_rating,
				  current_relation_kind,
				  current_match ? MATCH_STATE_MATCH : MATCH_STATE_NO_MATCH,
				  any_display_relations_set,
				  NULL, NULL, NULL,
				  NULL, NULL, NULL,
				  "video-joined-displays-symbolic",
				  _("Screen Size Mismatch"),
				  _("Doesn’t support your current screen size"),
				  NULL, NULL, NULL,
				  NULL, NULL, NULL);
	}

	/* For each of the control devices we understand, add a row to the dialogue. */
	add_relation_row (self->relations_list, &chosen_rating,
			  control_relations[AS_CONTROL_KIND_KEYBOARD],
			  has_keyboard ? MATCH_STATE_MATCH : MATCH_STATE_NO_MATCH,
			  any_control_relations_set,
			  "input-keyboard-symbolic",
			  _("Keyboard Support"),
			  _("Requires a keyboard"),
			  "dialog-question-symbolic",
			  _("Keyboard Support Unknown"),
			  _("Not enough information to know if keyboards are supported"),
			  "input-keyboard-symbolic",
			  _("Keyboard Required"),
			  _("Requires a keyboard"),
			  "input-keyboard-symbolic",
			  _("Keyboard Support"),
			  _("Supports keyboards"),
			  "input-keyboard-symbolic",
			  _("Keyboard Not Supported"),
			  _("Cannot be used with a keyboard"));

	add_relation_row (self->relations_list, &chosen_rating,
			  control_relations[AS_CONTROL_KIND_POINTING],
			  has_mouse ? MATCH_STATE_MATCH : MATCH_STATE_NO_MATCH,
			  any_control_relations_set,
			  "input-mouse-symbolic",
			  _("Mouse Support"),
			  _("Requires a mouse or pointing device"),
			  "dialog-question-symbolic",
			  _("Mouse Support Unknown"),
			  _("Not enough information to know if mice or pointing devices are supported"),
			  "input-mouse-symbolic",
			  _("Mouse Required"),
			  _("Requires a mouse or pointing device"),
			  "input-mouse-symbolic",
			  _("Mouse Support"),
			  _("Supports mice and pointing devices"),
			  "input-mouse-symbolic",
			  _("Mouse Not Supported"),
			  _("Cannot be used with a mouse or pointing device"));

	add_relation_row (self->relations_list, &chosen_rating,
			  control_relations[AS_CONTROL_KIND_TOUCH],
			  has_touchscreen ? MATCH_STATE_MATCH : MATCH_STATE_NO_MATCH,
			  any_control_relations_set,
			  "phone-symbolic",
			  _("Touchscreen Support"),
			  _("Requires a touchscreen"),
			  "dialog-question-symbolic",
			  _("Touchscreen Support Unknown"),
			  _("Not enough information to know if touchscreens are supported"),
			  "phone-symbolic",
			  _("Touchscreen Required"),
			  _("Requires a touchscreen"),
			  "phone-symbolic",
			  _("Touchscreen Support"),
			  _("Supports touchscreens"),
			  "phone-symbolic",
			  _("Touchscreen Not Supported"),
			  _("Cannot be used with a touchscreen"));

	/* Gamepads are a little different; only show the row if the appdata
	 * explicitly mentions gamepads, and don’t vary the row based on whether
	 * a gamepad is plugged in, since users often leave their gamepads
	 * unplugged until they’re actually needed. */
	add_relation_row (self->relations_list, &chosen_rating,
			  control_relations[AS_CONTROL_KIND_GAMEPAD],
			  MATCH_STATE_UNKNOWN,
			  any_control_relations_set,
			  NULL, NULL, NULL,
			  NULL, NULL, NULL,
			  "input-gaming-symbolic",
			  _("Gamepad Required"),
			  _("Requires a gamepad"),
			  "input-gaming-symbolic",
			  _("Gamepad Support"),
			  _("Supports gamepads"),
			  NULL, NULL, NULL);

	/* Update the UI. */
	switch (chosen_rating) {
	case GS_CONTEXT_DIALOG_ROW_IMPORTANCE_NEUTRAL:
		icon_name = "desktop-symbolic";
		/* Translators: It’s unknown whether this app is supported on
		 * the current hardware. The placeholder is the app name. */
		title = g_strdup_printf (_("%s probably works on this device"), gs_app_get_name (self->app));
		css_class = "grey";
		break;
	case GS_CONTEXT_DIALOG_ROW_IMPORTANCE_UNIMPORTANT:
		icon_name = "app-installed-symbolic";
		/* Translators: The app will work on the current hardware.
		 * The placeholder is the app name. */
		title = g_strdup_printf (_("%s works on this device"), gs_app_get_name (self->app));
		css_class = "green";
		break;
	case GS_CONTEXT_DIALOG_ROW_IMPORTANCE_INFORMATION:
		icon_name = "app-installed-symbolic";
		/* Translators: The app will possbily work on the current hardware.
		 * The placeholder is the app name. */
		title = g_strdup_printf (_("%s possibly works on this device"), gs_app_get_name (self->app));
		css_class = "yellow";
		break;
	case GS_CONTEXT_DIALOG_ROW_IMPORTANCE_WARNING:
		icon_name = "dialog-question-symbolic";
		/* Translators: The app may not work fully on the current hardware.
		 * The placeholder is the app name. */
		title = g_strdup_printf (_("%s will not work properly on this device"), gs_app_get_name (self->app));
		css_class = "orange";
		break;
	case GS_CONTEXT_DIALOG_ROW_IMPORTANCE_IMPORTANT:
		icon_name = "dialog-warning-symbolic";
		/* Translators: The app will not work properly on the current hardware.
		 * The placeholder is the app name. */
		title = g_strdup_printf (_("%s will not work on this device"), gs_app_get_name (self->app));
		css_class = "red";
		break;
	default:
		g_assert_not_reached ();
	}

	gs_lozenge_set_icon_name (GS_LOZENGE (self->lozenge), icon_name);
	gtk_label_set_text (self->title, title);

	gtk_widget_remove_css_class (self->lozenge, "green");
	gtk_widget_remove_css_class (self->lozenge, "yellow");
	gtk_widget_remove_css_class (self->lozenge, "orange");
	gtk_widget_remove_css_class (self->lozenge, "red");
	gtk_widget_remove_css_class (self->lozenge, "grey");

	gtk_widget_add_css_class (self->lozenge, css_class);
}

static void
app_notify_cb (GObject    *obj,
               GParamSpec *pspec,
               gpointer    user_data)
{
	GsHardwareSupportContextDialog *self = GS_HARDWARE_SUPPORT_CONTEXT_DIALOG (user_data);

	update_relations_list (self);
}

static void
gs_hardware_support_context_dialog_init (GsHardwareSupportContextDialog *self)
{
	g_type_ensure (GS_TYPE_LOZENGE);

	gtk_widget_init_template (GTK_WIDGET (self));
}

static void
gs_hardware_support_context_dialog_get_property (GObject    *object,
                                                 guint       prop_id,
                                                 GValue     *value,
                                                 GParamSpec *pspec)
{
	GsHardwareSupportContextDialog *self = GS_HARDWARE_SUPPORT_CONTEXT_DIALOG (object);

	switch ((GsHardwareSupportContextDialogProperty) prop_id) {
	case PROP_APP:
		g_value_set_object (value, gs_hardware_support_context_dialog_get_app (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_hardware_support_context_dialog_set_property (GObject      *object,
                                                 guint         prop_id,
                                                 const GValue *value,
                                                 GParamSpec   *pspec)
{
	GsHardwareSupportContextDialog *self = GS_HARDWARE_SUPPORT_CONTEXT_DIALOG (object);

	switch ((GsHardwareSupportContextDialogProperty) prop_id) {
	case PROP_APP:
		gs_hardware_support_context_dialog_set_app (self, g_value_get_object (value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gs_hardware_support_context_dialog_dispose (GObject *object)
{
	GsHardwareSupportContextDialog *self = GS_HARDWARE_SUPPORT_CONTEXT_DIALOG (object);

	gs_hardware_support_context_dialog_set_app (self, NULL);

	G_OBJECT_CLASS (gs_hardware_support_context_dialog_parent_class)->dispose (object);
}

static void
gs_hardware_support_context_dialog_class_init (GsHardwareSupportContextDialogClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	object_class->get_property = gs_hardware_support_context_dialog_get_property;
	object_class->set_property = gs_hardware_support_context_dialog_set_property;
	object_class->dispose = gs_hardware_support_context_dialog_dispose;

	/**
	 * GsHardwareSupportContextDialog:app: (nullable)
	 *
	 * The app to display the hardware support context details for.
	 *
	 * This may be %NULL; if so, the content of the widget will be
	 * undefined.
	 *
	 * Since: 41
	 */
	obj_props[PROP_APP] =
		g_param_spec_object ("app", NULL, NULL,
				     GS_TYPE_APP,
				     G_PARAM_READWRITE | G_PARAM_EXPLICIT_NOTIFY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties (object_class, G_N_ELEMENTS (obj_props), obj_props);

	gtk_widget_class_set_template_from_resource (widget_class, "/org/lingmo/Appstore/gs-hardware-support-context-dialog.ui");

	gtk_widget_class_bind_template_child (widget_class, GsHardwareSupportContextDialog, lozenge);
	gtk_widget_class_bind_template_child (widget_class, GsHardwareSupportContextDialog, title);
	gtk_widget_class_bind_template_child (widget_class, GsHardwareSupportContextDialog, relations_list);
}

/**
 * gs_hardware_support_context_dialog_new:
 * @app: (nullable): the app to display hardware support context information for, or %NULL
 *
 * Create a new #GsHardwareSupportContextDialog and set its initial app to @app.
 *
 * Returns: (transfer full): a new #GsHardwareSupportContextDialog
 * Since: 41
 */
GsHardwareSupportContextDialog *
gs_hardware_support_context_dialog_new (GsApp *app)
{
	g_return_val_if_fail (app == NULL || GS_IS_APP (app), NULL);

	return g_object_new (GS_TYPE_HARDWARE_SUPPORT_CONTEXT_DIALOG,
			     "app", app,
			     NULL);
}

/**
 * gs_hardware_support_context_dialog_get_app:
 * @self: a #GsHardwareSupportContextDialog
 *
 * Gets the value of #GsHardwareSupportContextDialog:app.
 *
 * Returns: (nullable) (transfer none): app whose hardware support context information is
 *     being displayed, or %NULL if none is set
 * Since: 41
 */
GsApp *
gs_hardware_support_context_dialog_get_app (GsHardwareSupportContextDialog *self)
{
	g_return_val_if_fail (GS_IS_HARDWARE_SUPPORT_CONTEXT_DIALOG (self), NULL);

	return self->app;
}

/**
 * gs_hardware_support_context_dialog_set_app:
 * @self: a #GsHardwareSupportContextDialog
 * @app: (nullable) (transfer none): the app to display hardware support context
 *     information for, or %NULL for none
 *
 * Set the value of #GsHardwareSupportContextDialog:app.
 *
 * Since: 41
 */
void
gs_hardware_support_context_dialog_set_app (GsHardwareSupportContextDialog *self,
                                            GsApp                          *app)
{
	g_return_if_fail (GS_IS_HARDWARE_SUPPORT_CONTEXT_DIALOG (self));
	g_return_if_fail (app == NULL || GS_IS_APP (app));

	if (app == self->app)
		return;

	g_clear_signal_handler (&self->app_notify_handler_relations, self->app);
	g_clear_signal_handler (&self->app_notify_handler_name, self->app);

	g_set_object (&self->app, app);

	if (self->app != NULL) {
		self->app_notify_handler_relations = g_signal_connect (self->app, "notify::relations", G_CALLBACK (app_notify_cb), self);
		self->app_notify_handler_name = g_signal_connect (self->app, "notify::name", G_CALLBACK (app_notify_cb), self);
	}

	/* Update the UI. */
	update_relations_list (self);

	g_object_notify_by_pspec (G_OBJECT (self), obj_props[PROP_APP]);
}
