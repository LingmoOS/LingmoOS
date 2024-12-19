/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Red Hat <www.redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include <glib/gi18n-lib.h>

#include "gs-packagekit-task.h"

/**
 * SECTION:gs-packagekit-task
 * @short_description: PkTask subclass which implements vfuncs for user interaction during a task
 *
 * #GsPackagekitTask is a subclass of #PkTask which represents a single
 * operation on PackageKit.
 *
 * By subclassing #PkTask, it can implement vfuncs which allow decisions
 * to be made about the task while it’s running. For example, to decide
 * what to do if an untrusted package needs to be installed.
 *
 * Since: 42
 */

typedef struct {
	GWeakRef plugin_weakref; /* GsPlugin * */
	GsPackagekitTaskQuestionType question_type;
	GsPackagekitHelper *helper;
} GsPackagekitTaskPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (GsPackagekitTask, gs_packagekit_task, PK_TYPE_TASK)

static gboolean
gs_packagekit_task_user_accepted (PkTask *task,
				  const gchar *title,
				  const gchar *msg,
				  const gchar *details,
				  const gchar *accept_label)
{
	GsPackagekitTask *gs_task = GS_PACKAGEKIT_TASK (task);
	GsPackagekitTaskPrivate *priv = gs_packagekit_task_get_instance_private (gs_task);
	g_autoptr(GsPlugin) plugin = NULL;
	gboolean accepts = FALSE;

	plugin = g_weak_ref_get (&priv->plugin_weakref);
	if (plugin)
		accepts = gs_plugin_ask_untrusted (plugin, title, msg, details, accept_label);

	return accepts;
}

typedef struct {
	GWeakRef task_weakref;
	guint request;
	gchar *title;
	gchar *msg;
	gchar *details;
	gchar *accept_label;
} QuestionData;

static QuestionData *
question_data_new (GsPackagekitTask *task,
		   guint request,
		   const gchar *title,
		   const gchar *msg,
		   const gchar *details,
		   const gchar *accept_label)
{
	QuestionData *qd;

	qd = g_slice_new0 (QuestionData);
	g_weak_ref_init (&qd->task_weakref, task);
	qd->request = request;
	qd->title = g_strdup (title);
	qd->msg = g_strdup (msg);
	qd->details = g_strdup (details);
	qd->accept_label = g_strdup (accept_label);

	return qd;
}

static void
question_data_free (gpointer ptr)
{
	QuestionData *qd = ptr;
	g_weak_ref_clear (&qd->task_weakref);
	g_free (qd->title);
	g_free (qd->msg);
	g_free (qd->details);
	g_free (qd->accept_label);
	g_slice_free (QuestionData, qd);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC (QuestionData, question_data_free)

static gboolean
gs_packagekit_task_question_idle_cb (gpointer user_data)
{
	QuestionData *qd = user_data;
	g_autoptr(PkTask) task = NULL;

	task = g_weak_ref_get (&qd->task_weakref);
	if (task) {
		if (gs_packagekit_task_user_accepted (task, qd->title, qd->msg, qd->details, qd->accept_label))
			pk_task_user_accepted (task, qd->request);
		else
			pk_task_user_declined (task, qd->request);
	}

	return G_SOURCE_REMOVE;
}

static void
gs_packagekit_task_schedule_question (GsPackagekitTask *task,
				      guint request,
				      const gchar *title,
				      const gchar *msg,
				      const gchar *details,
				      const gchar *accept_label)
{
	g_autoptr(QuestionData) qd = NULL;

	qd = question_data_new (task, request, title, msg, details, accept_label);
	g_idle_add_full (G_PRIORITY_HIGH_IDLE, gs_packagekit_task_question_idle_cb, g_steal_pointer (&qd), question_data_free);
}

/* This may be called in a PackageKit worker thread. */
static void
gs_packagekit_task_untrusted_question (PkTask *task,
				       guint request,
				       PkResults *results)
{
	GsPackagekitTask *gs_task = GS_PACKAGEKIT_TASK (task);
	GsPackagekitTaskPrivate *priv = gs_packagekit_task_get_instance_private (gs_task);
	g_autoptr(PkError) error = NULL;
	const gchar *title;
	const gchar *msg;
	const gchar *details;
	const gchar *accept_label;

	switch (priv->question_type) {
	case GS_PACKAGEKIT_TASK_QUESTION_TYPE_INSTALL:
		title = _("Install Unsigned Appstore?");
		msg = _("Appstore that is to be installed is not signed. It will not be possible to verify the origin of updates to this software, or whether updates have been tampered with.");
		accept_label = _("_Install");
		break;
	case GS_PACKAGEKIT_TASK_QUESTION_TYPE_DOWNLOAD:
		title = _("Download Unsigned Appstore?");
		msg = _("Unsigned updates are available. Without a signature, it is not possible to verify the origin of the update, or whether it has been tampered with.");
		accept_label = _("_Download");
		break;
	case GS_PACKAGEKIT_TASK_QUESTION_TYPE_UPDATE:
		title = _("Update Unsigned Appstore?");
		msg = _("Unsigned updates are available. Without a signature, it is not possible to verify the origin of the update, or whether it has been tampered with. Appstore updates will be disabled until unsigned updates are either removed or updated.");
		accept_label = _("_Update");
		break;
	default:
		pk_task_user_declined (task, request);
		return;
	}

	error = pk_results_get_error_code (results);
	if (error)
		details = pk_error_get_details (error);
	else
		details = NULL;

	gs_packagekit_task_schedule_question (gs_task, request, title, msg, details, accept_label);
}

static void
gs_packagekit_task_finalize (GObject *object)
{
	GsPackagekitTask *task = GS_PACKAGEKIT_TASK (object);
	GsPackagekitTaskPrivate *priv = gs_packagekit_task_get_instance_private (task);

	g_weak_ref_clear (&priv->plugin_weakref);
	g_clear_object (&priv->helper);

	G_OBJECT_CLASS (gs_packagekit_task_parent_class)->finalize (object);
}

static void
gs_packagekit_task_class_init (GsPackagekitTaskClass *klass)
{
	GObjectClass *object_class;
	PkTaskClass *task_class;

	task_class = PK_TASK_CLASS (klass);
	task_class->untrusted_question = gs_packagekit_task_untrusted_question;

	object_class = G_OBJECT_CLASS (klass);
	object_class->finalize = gs_packagekit_task_finalize;
}

static void
gs_packagekit_task_init (GsPackagekitTask *task)
{
	GsPackagekitTaskPrivate *priv = gs_packagekit_task_get_instance_private (task);

	g_weak_ref_init (&priv->plugin_weakref, NULL);
}

PkTask *
gs_packagekit_task_new (GsPlugin *plugin)
{
	GsPackagekitTask *task;
	GsPackagekitTaskPrivate *priv;

	g_return_val_if_fail (GS_IS_PLUGIN (plugin), NULL);

	task = g_object_new (GS_TYPE_PACKAGEKIT_TASK, NULL);
	priv = gs_packagekit_task_get_instance_private (task);

	g_weak_ref_set (&priv->plugin_weakref, plugin);

	return PK_TASK (task);
}

void
gs_packagekit_task_setup (GsPackagekitTask *task,
			  GsPackagekitTaskQuestionType question_type,
			  gboolean interactive)
{
	GsPackagekitTaskPrivate *priv = gs_packagekit_task_get_instance_private (task);

	g_return_if_fail (GS_IS_PACKAGEKIT_TASK (task));

	priv->question_type = question_type;

	/* The :interactive and :background properties have slightly different
	 * purposes:
	 *  - :interactive controls whether the task can create interactive
	 *    authentication (polkit) prompts
	 *  - :background controls the scheduling of the task relative to other
	 *    PackageKit tasks from this client and other clients
	 * However, we always want to set them both based on the same
	 * conditions. */
	pk_client_set_interactive (PK_CLIENT (task), interactive);
	pk_client_set_background (PK_CLIENT (task), !interactive);
}

GsPackagekitTaskQuestionType
gs_packagekit_task_get_question_type (GsPackagekitTask *task)
{
	GsPackagekitTaskPrivate *priv = gs_packagekit_task_get_instance_private (task);

	g_return_val_if_fail (GS_IS_PACKAGEKIT_TASK (task), GS_PACKAGEKIT_TASK_QUESTION_TYPE_NONE);

	return priv->question_type;
}

void
gs_packagekit_task_take_helper (GsPackagekitTask *task,
				GsPackagekitHelper *helper)
{
	GsPackagekitTaskPrivate *priv = gs_packagekit_task_get_instance_private (task);

	g_return_if_fail (GS_IS_PACKAGEKIT_TASK (task));

	if (priv->helper != helper) {
		g_clear_object (&priv->helper);
		priv->helper = helper;
	}
}

GsPackagekitHelper *
gs_packagekit_task_get_helper (GsPackagekitTask *task)
{
	GsPackagekitTaskPrivate *priv = gs_packagekit_task_get_instance_private (task);

	g_return_val_if_fail (GS_IS_PACKAGEKIT_TASK (task), NULL);

	return priv->helper;
}
