// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <memory>
#include <vector>

#include "base/geometry.h"
#include "base/signal.h"
#include "shell/shell.h"

namespace Lingmo::Shell {

class ClipboardHistory;
class ClipboardPickerView;
class ShellWindow;

// The clipboard history's picker: a 720 x 440 glass card centred in the primary
// output's work area, opened by org.lingmo.ClipboardHistory.toggle on the
// compositor's shortcut. As a Picker window it only borrows the keyboard:
// the application in front stays active, gets the keyboard back when the
// card goes, and receives the paste. Losing the keyboard while shown (a
// press elsewhere, another window activated) closes it.
//
// Ash's equivalent is ClipboardHistoryControllerImpl with its menu; the card
// and its keys are Lingmo's own.
class ClipboardPicker : public Component
{
public:
    ClipboardPicker(Shell *shell, std::unique_ptr<ClipboardHistory> history);
    ~ClipboardPicker() override;

    ClipboardHistory &history() const { return *m_history; }
    ShellWindow *window() const { return m_window.get(); }
    ClipboardPickerView *view() const { return m_view; }

    bool isOpen() const;
    void open();
    void close();
    void toggle();

    // Where the card goes: centred in the primary output's work area.
    base::Rect geometry() const;

private:
    void place();
    void applyGlass();

    Shell *m_shell;
    std::unique_ptr<ClipboardHistory> m_history;
    std::unique_ptr<ShellWindow> m_window;
    ClipboardPickerView *m_view = nullptr;
    // The keyboard arrives after the window maps; only losing it counts.
    bool m_hadKeyboard = false;
    int m_glassDark = -1;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace Lingmo::Shell

