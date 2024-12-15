// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ZwpInputPanelSurfaceV1.h"

#include "../InputMethodV1.h"

WL_ADDONS_BASE_USE_NAMESPACE

const struct zwp_input_panel_surface_v1_interface ZwpInputPanelSurfaceV1::impl = {
    ResourceCallbackWrapper<&ZwpInputPanelSurfaceV1::zwp_input_panel_surface_v1_set_toplevel>::func,
    ResourceCallbackWrapper<
        &ZwpInputPanelSurfaceV1::zwp_input_panel_surface_v1_set_overlay_panel>::func,
};

ZwpInputPanelSurfaceV1::ZwpInputPanelSurfaceV1()
    : Type()
{
}

ZwpInputPanelSurfaceV1::~ZwpInputPanelSurfaceV1() = default;

void ZwpInputPanelSurfaceV1::zwp_input_panel_surface_v1_set_toplevel(Resource *resource,
                                                                     struct wl_resource *output,
                                                                     uint32_t position)
{
}

void ZwpInputPanelSurfaceV1::zwp_input_panel_surface_v1_set_overlay_panel(Resource *resource) { }
