// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "ui/color_mixer.h"
#include "ui/geometry.h"

namespace Lingmo::ui {
class ColumnLayout;
class Label;
class RowLayout;
class Window;
}

namespace settings {

class FormRoot;
class SettingsWindow;

// Settings' own form dialogs (add a user, change a password, add a language,
// join a network, edit a shortcut): a modal window over the settings window
// with only a close button, its title in semibold on the button's line, and
// a column of content below. Its height follows the content unless fixed.
// A dialog is shown once: after it closes, make a new one.
class FormDialog
{
public:
    FormDialog(SettingsWindow& parent, std::string title, double width);
    virtual ~FormDialog();

    FormDialog(const FormDialog&) = delete;
    FormDialog& operator=(const FormDialog&) = delete;

    Lingmo::ui::Window* window() const { return m_window.get(); }
    // The dialog's ColumnLayout, inside its padding: the title first, then
    // what goes under it, with the dialog's spacing.
    Lingmo::ui::ColumnLayout* content() const;
    Lingmo::ui::Label* titleLabel() const;
    void setTitle(std::string title);
    // A RowLayout of buttons aligned to the right, as the last item of the
    // content.
    Lingmo::ui::RowLayout* addButtonRow();

    // A fixed height lets the content's filling items share the space.
    void setFixedHeight(double height);
    // Unset: the theme's second background colour.
    void setBackgroundColor(std::optional<Lingmo::ui::ColorRecipe> color);

    void show();
    // Asks canClose first, as the close button and Escape do.
    void requestClose();
    // Closes at once; the window goes once the event being handled is over.
    void close();
    bool isOpen() const { return m_window != nullptr; }

    // Returns false to keep the dialog open, e.g. while waiting for a reply.
    std::function<bool()> canClose;
    std::function<void()> onShown;
    std::function<void()> onClosed;

private:
    std::unique_ptr<Lingmo::ui::Window> m_window;
    FormRoot* m_root;
    double m_width;
    double m_fixedHeight = 0;
    std::optional<Lingmo::ui::ColorRecipe> m_background;
};

} // namespace settings
