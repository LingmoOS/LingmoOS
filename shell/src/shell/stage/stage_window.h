// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once

#include <memory>
#include <map>
#include <set>
#include <vector>

#include "base/signal.h"
#include "shell/shell.h"
#include "wm/stage/stage_manager.h"
#include "shell/screenshot/screen_grabber.h"

namespace Lingmo::ui {
class Button;
class Animation;
class ThemeIcon;
}

namespace Lingmo::Shell {

class ShellWindow;

// The compositor-owned recent-groups strip. It deliberately has no Desk
// controls: Desk remains a separate workspace feature.
class StageWindow final : public Component
{
public:
    StageWindow(Shell *shell, std::unique_ptr<ScreenGrabber> grabber);
    ~StageWindow() override;

    bool isShown() const;
    void toggle();
    ShellWindow *window() const { return m_window.get(); }

private:
    class View;

    void sync(const wm::StageManager::Snapshot &snapshot);
    void place();
    void updateVisibility();
    void refreshThumbnails();

    Shell *m_shell;
    std::unique_ptr<ShellWindow> m_window;
    View *m_view = nullptr;
    std::vector<base::Subscription> m_subscriptions;
    wm::StageManager::Snapshot m_snapshot;
    bool m_userVisible = true;
    std::unique_ptr<ScreenGrabber> m_grabber;
    std::unique_ptr<ui::Animation> m_visibilityAnimation;
    double m_visibilityProgress = 0;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
    struct Thumbnail {
        uint64_t revision = 0;
        ui::Bitmap bitmap;
    };
    std::map<wm::StageManager::Id, uint64_t> m_captured;
    std::map<wm::StageManager::Id, Thumbnail> m_thumbnails;
    bool m_capturePending = false;
};

} // namespace Lingmo::Shell
