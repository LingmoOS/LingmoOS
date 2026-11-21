// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"

#include "base/dbus.h"
#include "base/signal.h"
#include "base/timer.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Lingmo::ui {
struct KeyEvent;
}

namespace fm {

class FolderModel;
class FolderView;

// A file view's side of Quick Look, which the
// org.lingmo.FilePreview service shows for the selection.
class PreviewClient
{
public:
    PreviewClient();
    ~PreviewClient();

    void attach(FolderModel* model, FolderView* view);
    // The desktop keeps the preview on its item when its empty space is clicked.
    void setFollowsEmptySelection(bool follows) { m_followsEmptySelection = follows; }
    bool previewVisible() const { return m_previewVisible; }
    void toggle();
    // The view's window became active, or the view the current one in it.
    void activate();
    // Keys the preview takes while it is shown: Escape, and Left/Right through several items.
    bool filterKey(const ui::KeyEvent& event);

    base::Signal<> previewVisibleChanged;

private:
    std::string snapshot() const;
    bool follows() const;
    bool viewIsActive() const;
    template<typename... Args>
    void send(const std::string& method, const Args&... args);
    void setVisible(bool visible);
    void queryVisibility();

    std::string m_id;
    base::dbus::Proxy m_service;
    FolderModel* m_model = nullptr;
    FolderView* m_view = nullptr;
    bool m_followsEmptySelection = true;
    bool m_previewVisible = false;
    bool m_navigating = false;
    bool m_serviceRunning = false;
    base::Timer m_updateTimer;
    std::vector<base::dbus::Match> m_matches;
    base::dbus::NameWatch m_nameWatch;
    std::vector<base::Subscription> m_subscriptions;
    std::map<int, base::dbus::PendingCall> m_calls;
    int m_nextCall = 0;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
};

} // namespace fm
