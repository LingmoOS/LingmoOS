/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2023 jccKevin <luochaojiang@uniontech.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "drm_adaptor.h"

#ifndef SUPPORT_DRM_ITERATOR

static inline const uint32_t *
get_formats_ptr(const struct drm_format_modifier_blob *blob)
{
    return (const uint32_t *)(((uint8_t *)blob) + blob->formats_offset);
}

static inline const struct drm_format_modifier *
get_modifiers_ptr(const struct drm_format_modifier_blob *blob)
{
    return (const struct drm_format_modifier *)(((uint8_t *)blob) + blob->modifiers_offset);
}

static bool _drmModeFormatModifierGetNext(const drmModePropertyBlobRes *blob,
                                          drmModeFormatModifierIterator *iter)
{
    const struct drm_format_modifier *blob_modifiers, *mod;
    const struct drm_format_modifier_blob *fmt_mod_blob;
    const uint32_t *blob_formats;

    assert(blob && iter);

    fmt_mod_blob = static_cast<struct drm_format_modifier_blob *>(blob->data);
    blob_modifiers = get_modifiers_ptr(fmt_mod_blob);
    blob_formats = get_formats_ptr(fmt_mod_blob);

    /* fmt_idx and mod_idx designate the number of processed formats
     * and modifiers.
     */
    if (iter->fmt_idx >= fmt_mod_blob->count_formats || iter->mod_idx >= fmt_mod_blob->count_modifiers)
        return false;

    iter->fmt = blob_formats[iter->fmt_idx];
    iter->mod = DRM_FORMAT_MOD_INVALID;

    /* From the latest valid found, get the next valid modifier */
    while (iter->mod_idx < fmt_mod_blob->count_modifiers) {
        mod = &blob_modifiers[iter->mod_idx++];

        /* Check if the format that fmt_idx designates, belongs to
         * this modifier 64-bit window selected via mod->offset.
         */
        if (iter->fmt_idx < mod->offset || iter->fmt_idx >= mod->offset + 64)
            continue;
        if (!(mod->formats & (1 << (iter->fmt_idx - mod->offset))))
            continue;

        iter->mod = mod->modifier;
        break;
    }

    if (iter->mod_idx == fmt_mod_blob->count_modifiers) {
        iter->mod_idx = 0;
        iter->fmt_idx++;
    }

    /* Since mod_idx reset, in order for the caller to iterate over
     * the last modifier of the last format, always return true here
     * and early return from the next call.
     */
    return true;
}

bool drmModeFormatModifierBlobIterNext(const drmModePropertyBlobRes *blob,
                                       drmModeFormatModifierIterator *iter)
{
    drmModeFormatModifierIterator tmp;
    bool has_fmt;

    if (!blob || !iter)
        return false;

    tmp.fmt_idx = iter->fmt_idx;
    tmp.mod_idx = iter->mod_idx;

    /* With the current state of things, DRM/KMS drivers are allowed to
     * construct blobs having formats and no modifiers. Userspace can't
     * legitimately abort in such cases.
     *
     * While waiting for the kernel to perhaps disallow formats with no
     * modifiers in IN_FORMATS blobs, skip the format altogether.
     */
    do {
        has_fmt = _drmModeFormatModifierGetNext(blob, &tmp);
        if (has_fmt && tmp.mod != DRM_FORMAT_MOD_INVALID)
            *iter = tmp;

    } while (has_fmt && tmp.mod == DRM_FORMAT_MOD_INVALID);

    return has_fmt;
}

#endif

#ifndef DSUPPORT_DRM_TYPENAME

const char *drmModeGetConnectorTypeName(uint32_t connector_type)
{
    switch (connector_type) {
    case DRM_MODE_CONNECTOR_Unknown:
        return "Unknown";
    case DRM_MODE_CONNECTOR_VGA:
        return "VGA";
    case DRM_MODE_CONNECTOR_DVII:
        return "DVI-I";
    case DRM_MODE_CONNECTOR_DVID:
        return "DVI-D";
    case DRM_MODE_CONNECTOR_DVIA:
        return "DVI-A";
    case DRM_MODE_CONNECTOR_Composite:
        return "Composite";
    case DRM_MODE_CONNECTOR_SVIDEO:
        return "SVIDEO";
    case DRM_MODE_CONNECTOR_LVDS:
        return "LVDS";
    case DRM_MODE_CONNECTOR_Component:
        return "Component";
    case DRM_MODE_CONNECTOR_9PinDIN:
        return "DIN";
    case DRM_MODE_CONNECTOR_DisplayPort:
        return "DP";
    case DRM_MODE_CONNECTOR_HDMIA:
        return "HDMI-A";
    case DRM_MODE_CONNECTOR_HDMIB:
        return "HDMI-B";
    case DRM_MODE_CONNECTOR_TV:
        return "TV";
    case DRM_MODE_CONNECTOR_eDP:
        return "eDP";
    case DRM_MODE_CONNECTOR_VIRTUAL:
        return "Virtual";
    case DRM_MODE_CONNECTOR_DSI:
        return "DSI";
    case DRM_MODE_CONNECTOR_DPI:
        return "DPI";
    case DRM_MODE_CONNECTOR_WRITEBACK:
        return "Writeback";
    case DRM_MODE_CONNECTOR_SPI:
        return "SPI";
    case DRM_MODE_CONNECTOR_USB:
        return "USB";
    default:
        return NULL;
    }
}

#endif