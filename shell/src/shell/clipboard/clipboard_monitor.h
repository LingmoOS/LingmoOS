// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <memory>
#include <string>
#include <vector>

#include "base/signal.h"
#include "base/timer.h"
#include "shell/clipboard/clipboard_store.h"

namespace Lingmo::Clipboard {
class DataSource;
class SelectionController;
} // namespace Lingmo::Clipboard

namespace Lingmo::Shell {

// Follows the session's clipboard, the seat's selection, and puts history
// entries back on it, through the selection itself rather than data
// control.
//
// Each new selection is read without blocking: the kept types one after the
// other through a pipe, within a deadline and a size cap. A newer selection
// cancels a read in progress, so a program that writes the clipboard twice
// in a row leaves one copy.
class ClipboardMonitor
{
public:
    // How long one copy may take to read before it is dropped.
    static constexpr std::chrono::milliseconds kReadDeadline{2000};
    // Carries the history entry's key, so the selection coming back is known.
    static constexpr const char *kEntryType = "application/x-lingmo-clipboard-history";
    // lingmo-services takes over every selection to keep it after its owner
    // quits, except one that carries this.
    static constexpr const char *kServicesType = "application/x-lingmo-clipboard";

    explicit ClipboardMonitor(clipboard::SelectionController &selection);
    ~ClipboardMonitor();

    ClipboardMonitor(const ClipboardMonitor &) = delete;
    ClipboardMonitor &operator=(const ClipboardMonitor &) = delete;

    // Offers the payload as the selection; `key` names the history entry,
    // so the selection coming back is recognised rather than read.
    bool setSelection(const ClipboardPayload &payload, const std::string &key);

    // Something new was copied and read in full.
    base::Signal<const ClipboardPayload &> copied;
    // The selection is the entry setSelection() put there.
    base::Signal<const std::string &> ownSelectionShown;

private:
    void onSelectionChanged();
    void readNext();
    void onReadable();
    void finishFormat();
    void cancelRead();

    clipboard::SelectionController &m_selection;
    base::Subscription m_changed;
    std::weak_ptr<clipboard::DataSource> m_source;
    std::vector<std::string> m_pendingFormats;
    std::string m_readingFormat;
    std::string m_buffer;
    ClipboardPayload m_payload;
    int64_t m_readSize = 0;
    int m_readFd = -1;
    int m_readWatch = 0;
    base::Timer m_deadline;
    std::string m_ownKey;
};

} // namespace Lingmo::Shell

