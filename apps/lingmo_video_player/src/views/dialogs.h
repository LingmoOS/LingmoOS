// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "mpvnode.h"

#include <memory>
#include <string>

class MainWindow;

namespace Lingmo::ui {
class DialogWindow;
class FileChooser;
class Window;
}

// The player's dialogs, each a window of its own made on first use:
// Open URL, Media Info, Settings, Keyboard Shortcuts and About.
class Dialogs
{
public:
    explicit Dialogs(MainWindow& owner);
    ~Dialogs();

    void openUrl();
    void mediaInfo(const MpvNode::Map& info, const std::string& name);
    void preferences();
    void shortcuts();
    void about();
    void retranslate();

private:
    class OpenUrl;
    class About;

    MainWindow& m_owner;
    std::unique_ptr<OpenUrl> m_openUrl;
    std::unique_ptr<Lingmo::ui::DialogWindow> m_mediaInfo;
    std::unique_ptr<Lingmo::ui::DialogWindow> m_preferences;
    std::unique_ptr<Lingmo::ui::DialogWindow> m_shortcuts;
    std::unique_ptr<About> m_about;
    std::unique_ptr<Lingmo::ui::FileChooser> m_folderChooser;
};
