// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/thread_pool.h"
#include "shell/clipboard/clipboard_monitor.h"
#include "shell/clipboard/clipboard_store.h"
#include "shell/common/list_model.h"

namespace Lingmo::Clipboard {
class SelectionController;
} // namespace Lingmo::Clipboard

namespace Lingmo::Shell {

// The picker's rows: the history's entries a filter finds, newest first.
class ClipboardHistoryModel : public ListModelSignals
{
public:
    explicit ClipboardHistoryModel(ClipboardStore &store);

    int count() const { return int(m_rows.size()); }
    const ClipboardEntry &entry(int row) const;
    std::string keyAt(int row) const;

    void setFilterText(const std::string &text);
    bool historyEmpty() const { return m_store.entries().empty(); }

    // "text", "image" or "files".
    static std::string kind(const ClipboardEntry &entry);
    // One line: the text's start, "Image, <time>", a file's name or
    // "<n> Files".
    std::string title(int row) const;
    // The text the entry keeps, for the tooltip and the preview.
    std::string detail(int row) const;
    // The icon of the first file's type; empty for others, or until known.
    std::string iconName(int row) const;

private:
    void rebuild();
    void lookUpIcons();

    ClipboardStore &m_store;
    std::string m_filterText;
    // Indices into the store's entries.
    std::vector<size_t> m_rows;
    std::map<std::string, std::string> m_icons;
    base::Job m_iconJob;
    base::Subscription m_changed;
};

// The clipboard history component without its picker view: what is copied
// is recorded, an entry goes back on the clipboard and is pasted, and
// org.lingmo.ClipboardHistory at /ClipboardHistory toggles the picker on
// the compositor's shortcut.
class ClipboardHistory
{
public:
    ClipboardHistory(base::dbus::Connection *bus, clipboard::SelectionController &selection,
                     std::string directory = ClipboardStore::defaultDirectory(), size_t limit = 200);
    ~ClipboardHistory();

    ClipboardHistory(const ClipboardHistory &) = delete;
    ClipboardHistory &operator=(const ClipboardHistory &) = delete;

    bool registerService();

    ClipboardStore &store() { return m_store; }
    ClipboardHistoryModel &model() { return m_model; }

    // The view's.
    std::function<void()> togglePicker;
    std::function<void()> closePicker;
    // The window manager's sendPaste(), which waits for the keys the user
    // still holds.
    std::function<void()> sendPaste;

    // Puts the row's entry on the clipboard, closes the picker and pastes it
    // into the application in front; the body is read first, without
    // blocking.
    void paste(int row);
    // Puts it on the clipboard only.
    void copy(int row);
    void remove(int row);
    void clear();
    void toggle();

private:
    void select(int row, std::function<void()> then);

    base::dbus::Connection *m_bus;
    ClipboardStore m_store;
    ClipboardMonitor m_monitor;
    ClipboardHistoryModel m_model;
    std::unique_ptr<base::dbus::Object> m_object;
    std::vector<base::Subscription> m_subscriptions;
    // Selections being read; a later one supersedes an earlier.
    uint64_t m_selectSerial = 0;
};

} // namespace Lingmo::Shell

