// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ZwpInputPanelV1.h"

#include "../InputMethodV1.h"
#include "../Output.h"
#include "../Server.h"

extern "C" {
#include <wlr/types/wlr_compositor.h>
}

WL_ADDONS_BASE_USE_NAMESPACE

const struct zwp_input_panel_v1_interface ZwpInputPanelV1::impl = {
    ResourceCallbackWrapper<&ZwpInputPanelV1::zwp_input_panel_v1_get_input_panel_surface>::func,
};

ZwpInputPanelV1::ZwpInputPanelV1(InputMethodV1 *inputMethodV1)
    : Type()
    , inputMethodV1_(inputMethodV1)
    , popup_surface_commit_(this)
    , popup_surface_destroy_(this)
{
}

ZwpInputPanelV1::~ZwpInputPanelV1() = default;

void ZwpInputPanelV1::zwp_input_panel_v1_get_input_panel_surface(
    Resource *resource, uint32_t id, struct wl_resource *surface_resource)
{
    inputPanelSurface_.reset(new ZwpInputPanelSurfaceV1);
    inputPanelSurface_->init(resource->client(), id);

    surface_ = wlr_surface_from_resource(surface_resource);

    if (surface_) {
        wl_signal_add(&surface_->events.commit, popup_surface_commit_);
        wl_signal_add(&surface_->events.destroy, popup_surface_destroy_);
    }

    if (inputMethodV1_->inputPanelV1CreateCallback_) {
        inputMethodV1_->inputPanelV1CreateCallback_();
    }
}

void ZwpInputPanelV1::popupSurfaceCommitNotify(void *data)
{
    if (!surface_) {
        return;
    }

    auto &state = surface_->current;
    inputMethodV1_->server()->output()->setSize(state.width, state.height);
}

void ZwpInputPanelV1::popupSurfaceDestroyNotify(void *data)
{
    if (inputMethodV1_->inputPanelV1DestoryCallback_) {
        inputMethodV1_->inputPanelV1DestoryCallback_();
    }
}
