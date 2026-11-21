// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_TESTS_SHELL_SHELL_TEST_UTIL_H_
#define LINGMO_TESTS_SHELL_SHELL_TEST_UTIL_H_

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/strings.h"
#include "shell/common/applications.h"
#include "shell/common/task_windows.h"
#include "ui/bitmap.h"

namespace Lingmo::Test {

// Installed applications of a test's own, from desktop file text.
class FakeApplications : public shell::Applications
{
public:
    void add(const std::string &id, const std::string &contents)
    {
        auto entry = base::DesktopEntry::parse(id, "/usr/share/applications/" + id + ".desktop", contents, "C");
        if (!entry)
            return;
        std::erase_if(m_entries, [&id](const shell::DesktopEntryPtr &e) { return e->id == id; });
        m_entries.push_back(std::make_shared<const base::DesktopEntry>(*entry));
        rebuild();
    }

    void addSimple(const std::string &id, const std::string &name, const std::string &icon = {},
                   const std::string &extra = {})
    {
        add(id, "[Desktop Entry]\nType=Application\nName=" + name + "\nExec=" + id + "\nIcon=" + icon + "\n"
                        + extra);
    }

    void remove(const std::string &id)
    {
        std::erase_if(m_entries, [&id](const shell::DesktopEntryPtr &e) { return e->id == id; });
        rebuild();
    }

    // Announces a finished scan.
    void scan()
    {
        m_loading = false;
        changed();
    }

    void setLoading(bool loading) { m_loading = loading; }

    bool isLoading() const override { return m_loading; }

    shell::DesktopEntryPtr byId(const std::string &id) const override
    {
        for (const auto &entry : m_entries) {
            if (entry->id == id)
                return entry;
        }
        for (const auto &entry : m_entries) {
            if (base::strings::equalsIgnoreAsciiCase(entry->id, id))
                return entry;
        }
        return nullptr;
    }

    shell::DesktopEntryPtr byPath(const std::string &path) const override
    {
        for (const auto &entry : m_entries) {
            if (entry->path == path)
                return entry;
        }
        return nullptr;
    }

    shell::DesktopEntryPtr resolveWindow(const std::string &appId, const std::string &windowClass,
                                         const std::string &, const std::string &executableName) const override
    {
        for (const auto &entry : m_entries) {
            if (!appId.empty() && base::strings::equalsIgnoreAsciiCase(entry->id, appId))
                return entry;
            if (!windowClass.empty() && base::strings::equalsIgnoreAsciiCase(entry->startupWMClass, windowClass))
                return entry;
            const auto command = entry->command();
            if (!executableName.empty() && !command.empty() && command.front() == executableName)
                return entry;
        }
        return nullptr;
    }

    const std::vector<shell::DesktopEntryPtr> &applications() const override { return m_applications; }

private:
    void rebuild()
    {
        m_applications.clear();
        for (const auto &entry : m_entries) {
            if (entry->shouldShow({"Lingmo"}))
                m_applications.push_back(entry);
        }
        std::sort(m_applications.begin(), m_applications.end(),
                  [](const auto &a, const auto &b) { return a->name < b->name; });
    }

    std::vector<shell::DesktopEntryPtr> m_entries;
    std::vector<shell::DesktopEntryPtr> m_applications;
    bool m_loading = true;
};

// Application windows of a test's own; requests are recorded.
class FakeTaskWindows : public shell::TaskWindows
{
public:
    uint64_t add(shell::TaskWindow window)
    {
        if (!window.id)
            window.id = m_nextId++;
        const uint64_t id = window.id;
        m_windows[id] = window;
        m_order.push_back(id);
        windowAdded(id);
        return id;
    }

    void change(uint64_t id, const std::function<void(shell::TaskWindow &)> &edit, uint32_t changes)
    {
        edit(m_windows[id]);
        windowChanged(id, changes);
    }

    void remove(uint64_t id)
    {
        m_windows.erase(id);
        std::erase(m_order, id);
        windowRemoved(id);
        if (m_active == id)
            setActive(0);
    }

    void setActive(uint64_t id)
    {
        m_active = id;
        activeWindowChanged(id);
    }

    std::vector<uint64_t> windows() const override { return m_order; }
    const shell::TaskWindow *window(uint64_t id) const override
    {
        const auto it = m_windows.find(id);
        return it == m_windows.end() ? nullptr : &it->second;
    }
    uint64_t activeWindow() const override { return m_active; }
    const ui::Bitmap *minimizedSnapshot(uint64_t id) const override
    {
        const auto it = m_snapshots.find(id);
        return it == m_snapshots.end() ? nullptr : &it->second;
    }

    void activate(uint64_t id) override { requests.push_back("activate " + std::to_string(id)); }
    void minimize(uint64_t id) override { requests.push_back("minimize " + std::to_string(id)); }
    void setHidden(uint64_t id, bool hidden) override
    {
        requests.push_back((hidden ? "hide " : "unhide ") + std::to_string(id));
    }
    void close(uint64_t id) override { requests.push_back("close " + std::to_string(id)); }
    void quitApplication(uint64_t id) override { requests.push_back("quit " + std::to_string(id)); }
    void hideApplication(uint64_t id) override { requests.push_back("hideApplication " + std::to_string(id)); }
    void hideOtherApplications(uint64_t keep) override
    {
        requests.push_back("hideOthers " + std::to_string(keep));
    }

    std::map<uint64_t, ui::Bitmap> m_snapshots;
    std::vector<std::string> requests;

private:
    std::map<uint64_t, shell::TaskWindow> m_windows;
    std::vector<uint64_t> m_order;
    uint64_t m_active = 0;
    uint64_t m_nextId = 1;
};

inline std::string fromHex(std::string_view hex)
{
    return base::strings::fromHex(hex).value_or(std::string());
}

} // namespace Lingmo::Test

#endif // LINGMO_TESTS_SHELL_SHELL_TEST_UTIL_H_
