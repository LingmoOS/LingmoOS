// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "base/signal.h"
#include "pages/shortcuts/shortcutsmodel.h"
#include "widgets/formdialog.h"

namespace Lingmo::ui {
class ColumnLayout;
class DialogButton;
class Label;
struct KeyEvent;
}

namespace settings {

class ShortcutRecorder;

// Edits the shortcuts of one action: remove any of them, add another by
// pressing it, or go back to the defaults. Adding is confirm-by-repeat, as in
// the ChromeOS Shortcuts app: a conflict or a warning is shown once, and
// pressing the same combination again applies it anyway.
class ShortcutEditDialog : public FormDialog
{
public:
    ShortcutEditDialog(SettingsWindow& parent, ShortcutsModel& model, std::string action);
    ~ShortcutEditDialog() override;

    // What the recorder does with a key, from the keyboard or a test.
    void keyPressed(const Lingmo::ui::KeyEvent& event);
    void keyReleased(const Lingmo::ui::KeyEvent& event);
    void startRecording(std::optional<ShortcutsModel::Accelerator> replacing);
    void stopRecording();
    bool isRecording() const { return m_recording; }
    const std::string& message() const { return m_message; }

    // After the dialog closed.
    std::function<void()> onFinished;

private:
    void refresh();
    void update();
    void submit(uint32_t keycode, uint32_t keysym, uint32_t modifiers, std::vector<std::string> parts);

    ShortcutsModel& m_model;
    std::string m_action;
    Lingmo::base::Subscription m_modelChanged;
    std::vector<ShortcutsModel::Accelerator> m_accelerators;
    bool m_isDefault = true;
    bool m_recording = false;
    // The shortcut being replaced while recording; empty when adding.
    std::optional<ShortcutsModel::Accelerator> m_replacing;
    std::vector<std::string> m_pendingParts;
    std::string m_message;
    bool m_messageIsWarning = false;
    // Meta went down alone and nothing else has been pressed since: releasing
    // it records the Meta tap, which is how the launcher key is bound.
    bool m_metaTapCandidate = false;

    Lingmo::ui::Label* m_noShortcut;
    Lingmo::ui::ColumnLayout* m_list;
    ShortcutRecorder* m_recorder;
    Lingmo::ui::Label* m_messageLabel;
    Lingmo::ui::DialogButton* m_add;
    Lingmo::ui::DialogButton* m_restore;
};

} // namespace settings
