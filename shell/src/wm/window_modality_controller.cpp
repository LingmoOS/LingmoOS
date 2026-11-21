// Copyright 2012 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/wm/core/window_modality_controller.{h,cc}
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include "wm/window_modality_controller.h"

#include "wm/activation.h"
#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_state.h"

namespace Lingmo::WM {

namespace {

const Window *modalTransientChild(const Window *activatable)
{
    for (const Window *transient : activatable->transientChildren()) {
        if (isModalInEffect(transient)) {
            if (transient->transientChildren().empty())
                return transient;

            const Window *modalChild = modalTransientChild(transient);
            return modalChild ? modalChild : transient;
        }
    }
    return nullptr;
}

} // namespace

bool isModalInEffect(const Window *window)
{
    return window && window->isModal() && window->targetVisibility()
            && !window->state()->isMinimized()
            && window->manager()->hasWindow(window->transientParent());
}

Window *modalTransient(Window *window)
{
    return const_cast<Window *>(modalTransient(const_cast<const Window *>(window)));
}

const Window *modalTransient(const Window *window)
{
    return window ? modalTransientChild(window) : nullptr;
}

WindowModalityController::WindowModalityController(WindowManager *manager)
    : m_manager(manager)
{
}

bool WindowModalityController::processLocatedEvent(Window *target, bool pressed)
{
    if (!modalTransient(target))
        return false;
    // Activation follows the modal transient, so this is what brings it back.
    if (pressed)
        m_manager->activation()->activate(target);
    return true;
}

void WindowModalityController::updateModality(Window *window, bool modalChanged)
{
    Activation *activation = m_manager->activation();
    if (modalChanged && isModalInEffect(window)) {
        activation->activate(window);
        return;
    }
    if (Window *active = activation->activeWindow(); active && modalTransient(active))
        activation->activate(active);
}

} // namespace Lingmo::WM
