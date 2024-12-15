// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WL_ADDONS_BASE_ZWPINPUTPANELV1_H
#define WL_ADDONS_BASE_ZWPINPUTPANELV1_H

#include "../Listener.h"
#include "Type.h"
#include "wayland-input-method-unstable-v1-server-protocol.h"
#include "ZwpInputPanelSurfaceV1.h"

extern "C" {
struct wlr_surface;
}

WL_ADDONS_BASE_BEGIN_NAMESPACE

class InputMethodV1;

class ZwpInputPanelV1 : public Type<ZwpInputPanelV1, zwp_input_panel_v1>
{
    friend class Type;

public:
    explicit ZwpInputPanelV1(InputMethodV1 *inputMethodV1);
    ~ZwpInputPanelV1() override;

    const auto inputPanelSurface() const { return inputPanelSurface_.get(); }

protected:
    void zwp_input_panel_v1_get_input_panel_surface(Resource *resource,
                                                    uint32_t id,
                                                    struct wl_resource *surface_resource);

private:
    void popupSurfaceCommitNotify(void *data);
    void popupSurfaceDestroyNotify(void *data);

private:
    static const struct zwp_input_panel_v1_interface impl;
    std::unique_ptr<ZwpInputPanelSurfaceV1> inputPanelSurface_;
    InputMethodV1 *inputMethodV1_ = nullptr;
    wlr_surface *surface_ = nullptr;

    Listener<&ZwpInputPanelV1::popupSurfaceCommitNotify> popup_surface_commit_;
    Listener<&ZwpInputPanelV1::popupSurfaceDestroyNotify> popup_surface_destroy_;
};

WL_ADDONS_BASE_END_NAMESPACE

#endif // !WL_ADDONS_BASE_ZWPINPUTPANELV1_H
