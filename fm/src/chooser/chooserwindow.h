// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"

#include "base/signal.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Lingmo::ui {
class ConfirmDialog;
class DialogWindow;
class Window;
}

namespace fm {

class ChooserRoot;
class FileChooser;
class UnlockVolumeDialog;

// The places, the path bar, the list, the save
// and type fields and the buttons, in a fixed window over the application
// that asked, without a title bar.
class ChooserWindow
{
public:
    explicit ChooserWindow(FileChooser& chooser);
    ~ChooserWindow();

    void show();
    ui::Window* window() const { return m_window.get(); }

private:
    friend class ChooserRoot;

    void beginNewFolder();
    void retranslate();
    // The questions' title, text and buttons, in the language of now.
    void overwriteTexts();
    void deviceErrorTexts();

    FileChooser& m_chooser;
    std::unique_ptr<ui::Window> m_window;
    ChooserRoot* m_root = nullptr;
    std::unique_ptr<ui::ConfirmDialog> m_overwrite;
    std::string m_overwriteName;
    std::unique_ptr<ui::ConfirmDialog> m_deviceError;
    std::unique_ptr<UnlockVolumeDialog> m_unlock;
    std::unique_ptr<ui::DialogWindow> m_newFolder;
    std::function<void()> m_newFolderRetranslate;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace fm
