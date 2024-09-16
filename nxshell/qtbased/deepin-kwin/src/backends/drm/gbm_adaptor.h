/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2023 jccKevin <luochaojiang@uniontech.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

#include <gbm.h>

#ifndef SUPPORT_FULL_GBM
uint32_t gbm_format_canonicalize(uint32_t gbm_format);

char *gbm_format_get_name(uint32_t gbm_format, struct gbm_format_name_desc *desc);
#endif

#if defined(__cplusplus)
}
#endif