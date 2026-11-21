// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "wm/transient_manager.h"

#include <algorithm>

#include "wm/window.h"
#include "wm/window_manager.h"
#include "wm/window_observer.h"

namespace Lingmo::WM {

bool TransientManager::wouldCreateCycle(const Window *child, const Window *parent)
{
    for (const Window *ancestor = parent; ancestor; ancestor = ancestor->transientParent()) {
        if (ancestor == child)
            return true;
    }
    return false;
}

bool TransientManager::setTransientParent(Window *child, Window *parent)
{
    if (!child || child == parent)
        return false;
    if (parent && wouldCreateCycle(child, parent))
        return false;
    if (child->m_transientParent == parent)
        return true;

    Window *oldParent = child->m_transientParent;
    if (oldParent) {
        auto &siblings = oldParent->m_transientChildren;
        siblings.erase(std::remove(siblings.begin(), siblings.end(), child), siblings.end());
    }

    child->m_transientParent = parent;
    if (parent)
        parent->m_transientChildren.push_back(child);

    for (WindowObserver *observer : std::vector<WindowObserver *>(child->m_observers)) {
        if (child->hasObserver(observer))
            observer->onWindowTransientParentChanged(child, oldParent);
    }
    if (child->manager()->hasWindow(child)) {
        child->manager()->restackScene();
        child->manager()->onWindowModalityInputsChanged(child);
    }
    return true;
}

void TransientManager::windowDestroying(Window *window)
{
    if (!window)
        return;

    setTransientParent(window, nullptr);

    // Copy: each detach mutates the vector being walked.
    for (Window *child : std::vector<Window *>(window->m_transientChildren))
        setTransientParent(child, nullptr);
}

} // namespace Lingmo::WM
