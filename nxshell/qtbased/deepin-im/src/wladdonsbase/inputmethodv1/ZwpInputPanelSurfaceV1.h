// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WL_ADDONS_BASE_ZWPINPUTPANELSURFACEV1_H
#define WL_ADDONS_BASE_ZWPINPUTPANELSURFACEV1_H

#include "Type.h"
#include "wayland-input-method-unstable-v1-server-protocol.h"

WL_ADDONS_BASE_BEGIN_NAMESPACE

class InputMethodV1;

class ZwpInputPanelSurfaceV1 : public Type<ZwpInputPanelSurfaceV1, zwp_input_panel_surface_v1>
{
    friend class Type;

public:
    explicit ZwpInputPanelSurfaceV1();
    ~ZwpInputPanelSurfaceV1() override;

protected:
    void zwp_input_panel_surface_v1_set_toplevel(Resource *resource,
                                                 struct wl_resource *output,
                                                 uint32_t position);
    void zwp_input_panel_surface_v1_set_overlay_panel(Resource *resource);

private:
    static const struct zwp_input_panel_surface_v1_interface impl;
};

WL_ADDONS_BASE_END_NAMESPACE

#endif // !WL_ADDONS_BASE_ZWPINPUTPANELSURFACEV1_H
