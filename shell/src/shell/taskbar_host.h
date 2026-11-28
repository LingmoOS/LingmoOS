// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

#include <functional>
#include <memory>

#include "base/geometry.h"

namespace Lingmo::ui {
class View;
}

namespace Lingmo::Shell {

class ShellWindow;

class TaskbarHost
{
public:
    virtual ~TaskbarHost() = default;

    virtual ShellWindow *window() const = 0;
    virtual base::Rect panelRect() const = 0;
    virtual ui::View *setTaskbarStatusArea(std::unique_ptr<ui::View> area,
                                           std::function<void()> onRemoved) = 0;
    virtual std::unique_ptr<ui::View> takeTaskbarStatusArea() = 0;
};

} // namespace Lingmo::Shell
