/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 * vi:set noexpandtab tabstop=8 shiftwidth=8:
 *
 * Copyright (C) 2016 Richard Hughes <richard@hughsie.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#ifndef I_KNOW_THE_LINGMO_SOFTWARE_API_IS_SUBJECT_TO_CHANGE
#error You have to define I_KNOW_THE_LINGMO_SOFTWARE_API_IS_SUBJECT_TO_CHANGE
#endif

#include <gs-app.h>
#include <gs-app-list.h>
#include <gs-app-collation.h>
#include <gs-app-permissions.h>
#include <gs-app-query.h>
#include <gs-category.h>
#include <gs-category-manager.h>
#include <gs-desktop-data.h>
#include <gs-download-utils.h>
#include <gs-enums.h>
#include <gs-icon.h>
#include <gs-icon-downloader.h>
#include <gs-metered.h>
#include <gs-odrs-provider.h>
#include <gs-os-release.h>
#include <gs-plugin.h>
#include <gs-plugin-helpers.h>
#include <gs-plugin-job.h>
#include <gs-plugin-job-list-apps.h>
#include <gs-plugin-job-list-categories.h>
#include <gs-plugin-job-list-distro-upgrades.h>
#include <gs-plugin-job-manage-repository.h>
#include <gs-plugin-job-refine.h>
#include <gs-plugin-job-refresh-metadata.h>
#include <gs-plugin-job-update-apps.h>
#include <gs-plugin-vfuncs.h>
#include <gs-remote-icon.h>
#include <gs-rewrite-resources.h>
#include <gs-utils.h>
#include <gs-worker-thread.h>
