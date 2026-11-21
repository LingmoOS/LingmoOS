// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"

#include "ui/view.h"
#include "ui/window.h"

#include <functional>
#include <memory>
#include <string>

namespace Lingmo::ui {
class HeaderButton;
}

namespace fm {

// The file manager's own dialog windows (I/O progress, conflicts): a
// FishUI.Window without a title bar, in the second background colour, as
// tall as its content at a fixed width, moved by dragging anywhere and
// closed by Escape.
class FramelessDialog
{
public:
    // closeButton: FishUI's close button at 8, 8 from the top right.
    FramelessDialog(std::string title, bool closeButton);
    virtual ~FramelessDialog();

    FramelessDialog(const FramelessDialog&) = delete;
    FramelessDialog& operator=(const FramelessDialog&) = delete;

    // Null while hidden: a hidden dialog has no window, since a Wayland
    // window cannot be unmapped and shown again.
    ui::Window* window() const { return m_window.get(); }
    // Makes the content each time the dialog shows, laid out at the width
    // less the padding on every side.
    void setContentFactory(std::function<std::unique_ptr<ui::View>()> factory, double width, double padding);
    void setWidth(double width);
    // The window it belongs to, from the next show; null for none.
    void setParentWindow(ui::Window* parent, bool modal = false)
    {
        m_parent = parent;
        m_modal = modal;
    }
    ui::Window* parentWindow() const { return m_parent; }
    // A header above the content, as FishUI.Window's, its close button this
    // far from the top and the end; 0 for none (the content starts at the
    // padding and the button at 8, 8).
    void setHeader(double height, double buttonMargin);
    // FishUI.DialogWindow's title on the close button's line, 14 px, 16 in.
    void setHeaderTitle(std::string title);
    // Paddings other than the one on every side: left/right and top/bottom.
    void setPadding(double horizontal, double top, double bottom);
    // After the content's size may have changed.
    void fit();
    void show();
    void hide();
    bool isShown() const { return m_window != nullptr; }
    void setTitle(std::string title);
    // Makes the content of a shown dialog again, in place.
    void rebuildContent();
    // After the session language changed, for a dialog to take its text again.
    virtual void retranslate() {}

    // Escape, the close button or the compositor's close.
    std::function<void()> onCloseRequested;

private:
    class Root;

    std::string m_title;
    bool m_closeButton;
    std::function<std::unique_ptr<ui::View>()> m_factory;
    double m_width = 480;
    double m_padding = 16;
    double m_horizontal = -1;
    double m_top = -1;
    double m_bottom = -1;
    double m_header = 0;
    double m_buttonMargin = 8;
    std::string m_headerTitle;
    ui::Window* m_parent = nullptr;
    bool m_modal = false;
    std::unique_ptr<ui::Window> m_window;
    Root* m_root = nullptr;
};

} // namespace fm
