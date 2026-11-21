// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"
#include "model/pathbarmodel.h"

#include "ui/event.h"

#include <functional>
#include <memory>
#include <string>

namespace Lingmo::ui {
class TextField;
class Window;
}

namespace fm {

class CompletionList;

// The folders completing what is typed into a
// path editor, in a popup under it. The keys stay with the editor, which
// passes them on through handleKey().
class CompletionPopup
{
public:
    explicit CompletionPopup(ui::TextField* editor);
    ~CompletionPopup();

    // The user changed the text; active while it holds a path.
    void edited(bool active);
    // Any change of the text.
    void textChanged();
    void reset();
    void hide();
    // Whether the key was for the list.
    bool handleKey(const ui::KeyEvent& event, bool active);

    std::function<void(const std::string& path)> onActivated;

private:
    friend class CompletionList;

    void update();
    void move(int step);
    void complete();
    bool shown() const;

    ui::TextField* m_editor;
    PathCompletion m_model;
    bool m_active = false;
    bool m_edited = false;
    bool m_dismissed = false;
    int m_currentIndex = -1;
    std::unique_ptr<ui::Window> m_window;
    CompletionList* m_list = nullptr;
};

} // namespace fm
