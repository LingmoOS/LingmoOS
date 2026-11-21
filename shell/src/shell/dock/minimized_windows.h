// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "base/timer.h"
#include "shell/common/list_model.h"

namespace Lingmo::ui {
class Bitmap;
} // namespace Lingmo::UI

namespace Lingmo::Shell {

class DockModel;
class DockSettings;
class TaskWindows;

// The minimized windows the dock keeps beside the trash, in the order they
// were minimized, each shown by the image the window manager took of it as
// it went. Empty while the minimized area is off.
class MinimizedWindows : public ListModelSignals
{
public:
    // A minimize never reported landed, for example into a hidden dock,
    // still shows its thumbnail after this.
    static constexpr std::chrono::milliseconds kLandingFallback{600};

    struct Entry {
        uint64_t id = 0;
        // The window arrived in its slot; the thumbnail stays hidden while
        // the window manager still draws the window flying in.
        bool landed = false;
        // It flew into its slot with a minimize animation; one that did not,
        // such as a hidden window shown again, is revealed there.
        bool flownIn = false;
    };

    MinimizedWindows(TaskWindows &windows, DockModel &dock, DockSettings &settings);
    ~MinimizedWindows();

    MinimizedWindows(const MinimizedWindows &) = delete;
    MinimizedWindows &operator=(const MinimizedWindows &) = delete;

    const std::vector<Entry> &entries() const { return m_entries; }
    int count() const { return int(m_entries.size()); }
    bool contains(uint64_t id) const { return indexOf(id) >= 0; }

    // The window's title, else its app id.
    std::string title(int row) const;
    std::string iconName(int row) const;
    // Null until the window manager's image arrives; the icon stands in.
    const ui::Bitmap *thumbnail(int row) const;
    // Width over height of the window, which the thumbnail is fitted by.
    double aspect(int row) const;

    void restore(uint64_t id);
    void close(uint64_t id);
    void move(int from, int to);

    std::function<void()> hideLauncher;

private:
    int indexOf(uint64_t id) const;
    void update(uint64_t id);
    void append(uint64_t id, bool landed);
    void remove(uint64_t id, bool closed);
    void setLanded(uint64_t id);
    void onMinimizedAreaChanged();

    TaskWindows &m_windows;
    DockModel &m_dock;
    DockSettings &m_settings;
    std::vector<Entry> m_entries;
    // Minimized windows the user has hidden. They come back to the dock
    // without a minimize animation, so nothing lands on them.
    std::set<uint64_t> m_hiddenMinimized;
    std::map<uint64_t, std::unique_ptr<base::Timer>> m_landingTimers;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace Lingmo::Shell

