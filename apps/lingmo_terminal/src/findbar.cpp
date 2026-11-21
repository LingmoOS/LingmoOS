// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "findbar.h"

#include <algorithm>

#include "base/i18n.h"
#include "base/strings.h"
#include "ui/header_button.h"
#include "ui/keys.h"
#include "ui/painter.h"
#include "ui/cell_layout.h"
#include "ui/text_field.h"
#include "ui/theme.h"

#include "TerminalView.h"

using namespace Lingmo::ui;
using Lingmo::i18n::tr;

// The field's keys: Enter searches, Shift+Enter searches the other way, Escape closes.
class FindField : public TextField
{
public:
    std::function<void(bool shift)> onReturn;
    std::function<void()> onEscape;

protected:
    bool keyPress(const KeyEvent& event) override
    {
        if ((event.key == key::Return || event.key == key::KeypadEnter) && onReturn) {
            onReturn(event.modifiers & ShiftModifier);
            return true;
        }
        if (event.key == key::Escape && onEscape) {
            onEscape();
            return true;
        }
        return TextField::keyPress(event);
    }
};

// The overlay's colour over the base, mixed by the overlay's alpha.
static Color tint(Color base, Color overlay)
{
    const double a = overlay.a / 255.0;
    auto channel = [a](uint8_t b, uint8_t o) { return uint8_t(std::lround(b * (1 - a) + o * a)); };
    return Color{channel(base.r, overlay.r), channel(base.g, overlay.g), channel(base.b, overlay.b), base.a};
}

FindBar::FindBar(Konsole::TerminalView* terminal)
    : m_terminal(terminal)
    , m_field(addChild<FindField>())
    , m_previous(addChild<HeaderButton>(std::string(), 28))
    , m_next(addChild<HeaderButton>(std::string(), 28))
    , m_close(addChild<HeaderButton>("close", 28))
{
    setName("findBar");
    setVisible(false);

    m_field->setName("findField");
    m_field->setFrameVisible(false);
    m_field->setTextMargins(0, 0);
    m_previous->setGlyph("chevron-up", 16);
    m_next->setGlyph("chevron-down", 16);

    m_field->onTextChanged = [this] { search(false, false); };
    m_field->onReturn = [this](bool shift) { search(shift, true); };
    m_field->onEscape = [this] { close(); };
    m_previous->onClicked = [this] { search(false, true); };
    m_next->onClicked = [this] { search(true, true); };
    m_close->onClicked = [this] { close(); };

    m_findResult = m_terminal->findResult.connect([this](bool found) {
        m_notFound = !found;
        updateColors();
    });
    retranslate();
    updateColors();
}

FindBar::~FindBar() = default;

void FindBar::setColors(Color background, Color text, bool darkMode, Color error, Color accent)
{
    m_background = background;
    m_text = text;
    m_darkMode = darkMode;
    m_error = error;
    m_accent = accent;
    updateColors();
}

void FindBar::updateColors()
{
    m_field->setTextColor(m_notFound ? m_error : m_text);
    m_field->setPlaceholderColor(m_text.withAlpha(0.45));
    m_field->setSelectionColors(m_accent.withAlpha(0.4), m_text);
    // The tint the hover and press plates are drawn in: white on dark, #323238 on light.
    m_previous->setPlateAlphas({0.11, 0.21}, {0.12, 0.06});
    m_next->setPlateAlphas({0.11, 0.21}, {0.12, 0.06});
    m_close->setPlateAlphas({0.11, 0.21}, {0.12, 0.06});
    // The chevrons in the scheme foreground; the close artwork and the plates follow the header's dark mode.
    m_previous->setGlyphColor(m_text);
    m_next->setGlyphColor(m_text);
    requestPaint();
}

void FindBar::open(const std::string& text)
{
    if (!text.empty())
        m_field->setText(text);

    setVisible(true);
    m_field->selectAll();
    m_field->setFocus();
}

void FindBar::close()
{
    setVisible(false);
    m_notFound = false;
    updateColors();
    m_terminal->clearFind();
    if (onClosed)
        onClosed();
}

void FindBar::retranslate()
{
    m_field->setPlaceholder(tr("Find"));
}

void FindBar::search(bool forwards, bool fromSelection)
{
    const std::string text = m_field->text();
    if (text.empty())
        m_notFound = false;

    // Smart case: a capital letter in the query makes it case-sensitive.
    const bool caseSensitive = text != Lingmo::base::strings::toAsciiLower(text);
    m_terminal->find(text, forwards, fromSelection, caseSensitive);
}

// A row, large spacing from the left, 4 from the right, 2 between: the field
// fills, then three 28 px buttons.
void FindBar::layout()
{
    const Theme& theme = Theme::current();
    const RectF bounds = localRect();
    const double buttonSize = 28;
    const double spacing = 2;
    const double right = bounds.width - 4;
    const double buttonsWidth = buttonSize * 3 + spacing * 2;
    const double fieldWidth = std::max(0.0, right - buttonsWidth - spacing - theme.largeSpacing);
    // Layouts place their children on whole logical pixels, centred.
    const double buttonY = std::round((bounds.height - buttonSize) / 2);

    m_field->setGeometry({theme.largeSpacing, 0, std::round(fieldWidth), bounds.height});
    double x = std::round(theme.largeSpacing + fieldWidth + spacing);
    for (HeaderButton* button : {m_previous, m_next, m_close}) {
        button->setGeometry({x, buttonY, buttonSize, buttonSize});
        x += buttonSize + spacing;
    }
}

void FindBar::paint(Painter& painter)
{
    const Theme& theme = Theme::current();
    const Color fill = tint(m_background, m_text.withAlpha(m_darkMode ? 0.1 : 0.06));
    // A rounded rectangle with a 0.5 px border, not pixel aligned.
    painter.drawSmoothRectangle(localRect(), theme.bigRadius, fill, 0.5, m_text.withAlpha(0.12));
}
