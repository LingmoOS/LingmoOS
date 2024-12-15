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

Output::Output(Server *dimwl, struct wlr_output *output, wl_list *list)
    : server_(dimwl)
    , output_(output)
    , frame_(this)
    , destroy_(this)
{
    /* Configures the output created by the backend to use our allocator
     * and our renderer. Must be done once, before commiting the output */
    wlr_output_init_render(output_, server_->allocator(), server_->renderer());

    /* Some backends don't have modes. DRM+KMS does, and we need to set a mode
     * before we can use the output. The mode is a tuple of (width, height,
     * refresh rate), and each monitor supports only a specific set of modes. We
     * just pick the monitor's preferred mode, a more sophisticated compositor
     * would let the user configure it. */
    if (!wl_list_empty(&output_->modes)) {
        struct wlr_output_mode *mode = wlr_output_preferred_mode(output_);
        wlr_output_set_mode(output_, mode);
        wlr_output_enable(output_, true);
        if (!wlr_output_commit(output_)) {
            return;
        }
    }

    wl_signal_add(&output_->events.destroy, destroy_);
    wl_signal_add(&output_->events.frame, frame_);

    if (list) {
        wl_list_insert(list, &link_);
    }

    /* Adds this to the output layout. The add_auto function arranges outputs
     * from left-to-right in the order they appear. A more sophisticated
     * compositor would let the user configure the arrangement of outputs in the
     * layout.
     *
     * The output layout utility automatically adds a wl_output global to the
     * display, which Wayland clients can see to find out information about the
     * output (such as DPI, scale factor, manufacturer, etc).
     */
    wlr_output_layout_add_auto(server_->outputLayout(), output_);
}

Output::~Output()
{
    wl_list_remove(&link_);
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
