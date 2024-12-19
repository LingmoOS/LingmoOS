/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2021 Red Hat <www.redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib-object.h>
#include <lingmo-appstore.h>
#include <packagekit-glib2/packagekit.h>

#include "gs-packagekit-helper.h"

G_BEGIN_DECLS

/**
 * GsPackagekitTaskQuestionType:
 * @GS_PACKAGEKIT_TASK_QUESTION_TYPE_NONE: No question should be asked.
 * @GS_PACKAGEKIT_TASK_QUESTION_TYPE_INSTALL: Question is about installing an app.
 * @GS_PACKAGEKIT_TASK_QUESTION_TYPE_DOWNLOAD: Question is about downloading an app.
 * @GS_PACKAGEKIT_TASK_QUESTION_TYPE_UPDATE: Question is about updating an app.
 *
 * The type of question the task should ask the user if there’s an untrusted
 * repo prompt from PackageKit. Most callers of #GsPackagekitTask should use
 * %GS_PACKAGEKIT_TASK_QUESTION_TYPE_NONE.
 *
 * Since: 44
 */
typedef enum {
	GS_PACKAGEKIT_TASK_QUESTION_TYPE_NONE,
	GS_PACKAGEKIT_TASK_QUESTION_TYPE_INSTALL,
	GS_PACKAGEKIT_TASK_QUESTION_TYPE_DOWNLOAD,
	GS_PACKAGEKIT_TASK_QUESTION_TYPE_UPDATE,
} GsPackagekitTaskQuestionType;

#define GS_TYPE_PACKAGEKIT_TASK (gs_packagekit_task_get_type ())

G_DECLARE_DERIVABLE_TYPE (GsPackagekitTask, gs_packagekit_task, GS, PACKAGEKIT_TASK, PkTask)

struct _GsPackagekitTaskClass
{
	PkTaskClass parent_class;
};

PkTask		*gs_packagekit_task_new		(GsPlugin		*plugin);
void		 gs_packagekit_task_setup	(GsPackagekitTask	*task,
						 GsPackagekitTaskQuestionType question_type,
						 gboolean		 interactive);
GsPackagekitTaskQuestionType gs_packagekit_task_get_question_type (GsPackagekitTask	*task);
void		 gs_packagekit_task_take_helper	(GsPackagekitTask	*task,
						 GsPackagekitHelper	*helper);
GsPackagekitHelper *
		 gs_packagekit_task_get_helper	(GsPackagekitTask	*task);

G_END_DECLS
