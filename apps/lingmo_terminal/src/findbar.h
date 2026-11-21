// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <string>

#include "base/signal.h"
#include "ui/view.h"

namespace Konsole {
class TerminalView;
}

namespace Lingmo::ui {
class HeaderButton;
}

class FindField;

// Floats over a terminal while searching its output and history. Enter goes to
// the next older match, Shift+Enter to the next newer one. Drawn in the
// terminal's colours; the field itself has no frame, only the bar does.
class FindBar : public Lingmo::ui::View
{
public:
    explicit FindBar(Konsole::TerminalView* terminal);
    ~FindBar() override;

    // The colour scheme's own colours.
    void setColors(Lingmo::ui::Color background, Lingmo::ui::Color text, bool darkMode,
                   Lingmo::ui::Color error, Lingmo::ui::Color accent);

    void open(const std::string& text);
    void close();
    void retranslate();

    std::function<void()> onClosed;

    Lingmo::ui::SizeF sizeHint() const override { return {320, 40}; }
    bool acceptsPointer() const override { return true; }

protected:
    void layout() override;
    void paint(Lingmo::ui::Painter& painter) override;

private:
    void search(bool forwards, bool fromSelection);
    void updateColors();

    Konsole::TerminalView* m_terminal;
    FindField* m_field;
    Lingmo::ui::HeaderButton* m_previous;
    Lingmo::ui::HeaderButton* m_next;
    Lingmo::ui::HeaderButton* m_close;
    Lingmo::base::Subscription m_findResult;
    bool m_notFound = false;
    Lingmo::ui::Color m_background = Lingmo::ui::Color::rgb(0x1A1B26);
    Lingmo::ui::Color m_text = Lingmo::ui::Color::rgb(0xC0CAF5);
    Lingmo::ui::Color m_error = Lingmo::ui::Color::rgb(0xF7768E);
    Lingmo::ui::Color m_accent = Lingmo::ui::Color::rgb(0x7AA2F7);
    bool m_darkMode = true;
};
