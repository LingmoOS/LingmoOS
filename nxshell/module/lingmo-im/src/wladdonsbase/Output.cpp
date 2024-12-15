// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Output.h"

#include "Server.h"

#include <time.h>

extern "C" {
#include <wlr/types/wlr_output.h>
#define static
#include <wlr/types/wlr_scene.h>
#undef static
#include <wlr/version.h>
}

WL_ADDONS_BASE_USE_NAMESPACE

Output::Output(Server *dimwl, struct wlr_output *output, wl_list *list)
    : server_(dimwl)
    , output_(output)
    , frame_(this)
    , destroy_(this)
{
    /* Configures the output created by the backend to use our allocator
     * and our renderer. Must be done once, before commiting the output */
    wlr_output_init_render(output_, server_->allocator(), server_->renderer());

    wlr_scene_output_create(server_->scene(), output_);

    wl_signal_add(&output_->events.destroy, destroy_);
    wl_signal_add(&output_->events.frame, frame_);

    if (list) {
        wl_list_insert(list, &link_);
    }
}

Output::~Output()
{
    wl_list_remove(&link_);
}

void Output::setSize(int width, int height)
{
    if (width == 0 || height == 0) {
        return;
    }

    wlr_output_state state;
    wlr_output_state_init(&state);
    wlr_output_state_set_enabled(&state, true);
    wlr_output_state_set_custom_mode(&state, width, height, 0);
    wlr_output_state_set_adaptive_sync_enabled(&state, true);
    wlr_output_commit_state(output_, &state);
    wlr_output_state_finish(&state);
}

void Output::frameNotify(void *data)
{
    /* This function is called every time an output is ready to display a frame,
     * generally at the output's refresh rate (e.g. 60Hz). */
    struct wlr_scene *scene = server_->scene();

    struct wlr_scene_output *scene_output = wlr_scene_get_scene_output(scene, output_);

    /* Render the scene if needed and commit the output */
#if WLR_VERSION_MINOR >= 17
    wlr_scene_output_commit(scene_output, nullptr);
#else
    wlr_scene_output_commit(scene_output);
#endif

    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    wlr_scene_output_send_frame_done(scene_output, &now);
}

void Output::destroyNotify(void *data)
{
    delete this;
}
