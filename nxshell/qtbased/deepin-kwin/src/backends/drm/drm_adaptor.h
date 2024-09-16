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

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#if HAVE_SYS_SYSCTL_H
#ifdef __FreeBSD__
#include <sys/types.h>
#endif
#include <sys/sysctl.h>
#endif
#include <stdio.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libdrm/drm_mode.h>

#ifndef SUPPORT_DRM_ITERATOR

#define DRM_MODE_CONNECTOR_WRITEBACK 18
#define DRM_MODE_CONNECTOR_SPI 19
#define DRM_MODE_CONNECTOR_USB 20

/* Vendor Ids: */
#define DRM_FORMAT_MOD_NONE           0
#define DRM_FORMAT_MOD_VENDOR_NONE    0
#define DRM_FORMAT_MOD_VENDOR_INTEL   0x01
#define DRM_FORMAT_MOD_VENDOR_AMD     0x02
#define DRM_FORMAT_MOD_VENDOR_NV      0x03
#define DRM_FORMAT_MOD_VENDOR_SAMSUNG 0x04
#define DRM_FORMAT_MOD_VENDOR_QCOM    0x05
#define DRM_FORMAT_MOD_VENDOR_VIVANTE 0x06
#define DRM_FORMAT_MOD_VENDOR_BROADCOM 0x07
/* add more to the end as needed */

#define DRM_FORMAT_RESERVED ((1ULL << 56) - 1)

#define fourcc_mod_code(vendor, val) \
    ((((__u64)DRM_FORMAT_MOD_VENDOR_##vendor) << 56) | (val & 0x00ffffffffffffffULL))

/*
 * Invalid Modifier
 *
 * This modifier can be used as a sentinel to terminate the format modifiers
 * list, or to initialize a variable with an invalid modifier. It might also be
 * used to report an error back to userspace for certain APIs.
 */
#define DRM_FORMAT_MOD_INVALID fourcc_mod_code(NONE, DRM_FORMAT_RESERVED)

typedef struct _drmModeFormatModifierIterator
{
    uint32_t fmt_idx, mod_idx;
    uint32_t fmt;
    uint64_t mod;
} drmModeFormatModifierIterator;

bool drmModeFormatModifierBlobIterNext(const drmModePropertyBlobRes *blob,
                                       drmModeFormatModifierIterator *iter);

#endif

#ifndef DSUPPORT_DRM_TYPENAME
const char *drmModeGetConnectorTypeName(uint32_t connector_type);
#endif

#if defined(__cplusplus)
}
#endif