// Copyright 2018 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wl_output.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#pragma once

#include <string>
#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include "compositor/capture_source.h"
#include "render/presentation.h"

struct wl_resource;
struct wl_client;
struct wl_display;
struct wl_global;

namespace Lingmo::Compositor {

struct Output;
class Surface;
}

namespace Lingmo::Wayland {

void forEachOutputBinding(wl_client *client,
                          const std::shared_ptr<render::PresentationOutput> &output,
                          const std::function<void(wl_resource *)> &callback);

wl_global *createOutputGlobal(wl_display *display, const compositor::Output *output);
void updateOutputGlobal(wl_global *global, const compositor::Output *output);
void updateOutputMembership(wl_global *global);
// Enter and leave for `surfaces` alone, the ones a scene change moved.
void updateOutputMembership(wl_global *global,
                            const std::vector<std::shared_ptr<compositor::Surface *>> &surfaces);
void removeOutputGlobal(wl_global *global);
std::shared_ptr<compositor::CaptureSource> outputCaptureSource(wl_resource *resource);
std::optional<std::string> outputNameFromResource(wl_resource *resource);
wl_global *createXdgOutputManagerGlobal(wl_display *display);
} // namespace Lingmo::Wayland
