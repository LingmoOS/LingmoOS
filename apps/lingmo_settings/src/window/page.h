// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

#include "ui/scroll_view.h"

namespace Lingmo::ui {
class ColumnLayout;
}

namespace settings {

class SettingsWindow;

// A page of the window: a vertical scroll view whose content runs up under
// the floating window header, with the page margins and the rhythm between
// sections. The title is drawn by the window header.
class Page : public Lingmo::ui::ScrollView
{
public:
    Page(SettingsWindow& window, std::string title);
    ~Page() override;

    const std::string& title() const { return m_title; }
    SettingsWindow& settingsWindow() const { return m_settingsWindow; }

    // The column the page's sections go into.
    Lingmo::ui::ColumnLayout* content() const { return m_content; }

    // A wheel or touchpad scroll that landed on the header over the page.
    bool scrollFromHeader(const Lingmo::ui::ScrollEvent& event) { return scroll(event); }

protected:
    void paint(Lingmo::ui::Painter& painter) override;

private:
    SettingsWindow& m_settingsWindow;
    std::string m_title;
    Lingmo::ui::ColumnLayout* m_content;
};

// The page shown for an entry whose page is not ported yet.
class PendingPage : public Page
{
public:
    PendingPage(SettingsWindow& window, std::string title);
};

} // namespace settings
