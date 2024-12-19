/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2022 Red Hat <www.redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include "gs-plugin-job.h"

G_BEGIN_DECLS

#define GS_TYPE_PLUGIN_JOB_MANAGE_REPOSITORY (gs_plugin_job_manage_repository_get_type ())

G_DECLARE_FINAL_TYPE (GsPluginJobManageRepository, gs_plugin_job_manage_repository, GS, PLUGIN_JOB_MANAGE_REPOSITORY, GsPluginJob)

GsPluginJob	*gs_plugin_job_manage_repository_new	(GsApp				 *repository,
							 GsPluginManageRepositoryFlags	  flags);

G_END_DECLS
