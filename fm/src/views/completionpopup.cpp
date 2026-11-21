// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/completionpopup.h"
#include "views/style.h"

#include "ui/color_provider.h"
#include "ui/icon.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/cell_layout.h"
#include "ui/scroll_bar.h"
#include "ui/scroll_view.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "ui/window.h"

#include <algorithm>
#include <cmath>

namespace fm {

static constexpr int kMaximumRows = 8;
static constexpr double kRowHeight = 30;
static constexpr double kSpacing = 1;
static constexpr double kPadding = 4;
// Above and below the rows: with the highlight's 1 px it leaves the same 7 px gap to the card as at the sides.
static constexpr double kVerticalPadding = 6;
// FishUI.MenuItem's row: padding 8, label padding 4, a 22 px icon, the highlight 3 px in and 1 px down;
// its radius is concentric with the card's, the card radius less the 7 px between them.
static constexpr double kRowPadding = 8 + 4;
static constexpr double kIconSize = 22;
static constexpr double kHighlightInset = 3;

// The rows, as menu items with a folder icon; a lit row follows the keys only.
class CompletionList : public ui::View
{
public:
    explicit CompletionList(CompletionPopup* popup)
        : m_popup(popup)
    {
        m_bar = addChild<ui::ScrollBar>(ui::ScrollBar::Orientation::Vertical);
        // Always drawn on a list that scrolls, as in the ComboBox drop-down: the only sign there is more of it.
        m_bar->setAlwaysShown(true);
        m_bar->onPositionRequested = [this](double position) { scrollBy(position - m_scroll); };
    }

    bool acceptsPointer() const override { return true; }

    void rebuild()
    {
        const int count = m_popup->m_model.count();
        while (int(m_rows.size()) > count) {
            std::shared_ptr<View> label = takeChild(m_rows.back().first);
            std::shared_ptr<View> icon = takeChild(m_rows.back().second);
            m_rows.pop_back();
        }
        while (int(m_rows.size()) < count) {
            auto* icon = addChild<ui::ThemeIcon>("folder", int(kIconSize));
            auto* label = addChild<ui::Label>();
            label->setElide(ui::Label::Elide::Right);
            m_rows.push_back({label, icon});
        }
        for (int i = 0; i < count; ++i)
            m_rows[size_t(i)].first->setText(m_popup->m_model.name(i));
        raiseChild(m_bar);
        requestLayout();
        requestPaint();
    }

    double contentHeight() const
    {
        const int count = m_popup->m_model.count();
        return count * (kRowHeight + kSpacing) - (count > 0 ? kSpacing : 0);
    }

protected:
    void layout() override
    {
        const ui::Theme& theme = ui::Theme::current();
        const double width = geometry().width;
        const double scroll = m_scroll;
        for (size_t i = 0; i < m_rows.size(); ++i) {
            const double y = double(i) * (kRowHeight + kSpacing) - scroll;
            const bool lit = int(i) == m_popup->m_currentIndex;
            auto [label, icon] = m_rows[i];
            icon->setGeometry({kRowPadding, y + std::round((kRowHeight - kIconSize) / 2), kIconSize, kIconSize});
            icon->setColor(lit ? std::optional(theme.highlightedText) : std::nullopt);
            const double x = kRowPadding + kIconSize + theme.mediumSpacing;
            const ui::SizeF hint = label->sizeHint();
            label->setColor(lit ? theme.highlightedText : theme.text);
            label->setGeometry({x, y + ui::rounding::centeredStart(kRowHeight, hint.height),
                                std::max(0.0, width - x - 8), hint.height});
        }
        m_bar->setGeometry({width - ui::ScrollBar::thickness, 0, ui::ScrollBar::thickness, geometry().height});
        m_bar->setRange(geometry().height, contentHeight(), m_scroll);
    }

    void paint(ui::Painter& painter) override
    {
        const int current = m_popup->m_currentIndex;
        if (current < 0)
            return;
        const ui::Theme& theme = ui::Theme::current();
        const double y = current * (kRowHeight + kSpacing) - m_scroll;
        painter.drawRoundedRect({kHighlightInset, y + 1, geometry().width - kHighlightInset * 2, kRowHeight - 2},
                                {theme.mediumRadius - kPadding - kHighlightInset, 0, ui::Corners::Circular},
                                ui::color(ui::ColorId::MenuHighlight));
    }

    bool scroll(const ui::ScrollEvent& event) override
    {
        scrollBy(event.deltaY);
        return true;
    }

    void pointerRelease(const ui::PointerEvent& event) override
    {
        const int row = int(std::floor((event.position.y + m_scroll) / (kRowHeight + kSpacing)));
        if (row >= 0 && row < m_popup->m_model.count() && m_popup->onActivated)
            m_popup->onActivated(m_popup->m_model.path(row));
    }

public:
    void scrollBy(double delta)
    {
        const double maximum = std::max(0.0, contentHeight() - geometry().height);
        m_scroll = std::clamp(m_scroll + delta, 0.0, maximum);
        requestLayout();
        requestPaint();
    }

    void reveal(int row)
    {
        const double top = row * (kRowHeight + kSpacing);
        if (top < m_scroll)
            m_scroll = top;
        else if (top + kRowHeight > m_scroll + geometry().height)
            m_scroll = top + kRowHeight - geometry().height;
        requestLayout();
        requestPaint();
    }

    void resetScroll() { m_scroll = 0; }

private:
    CompletionPopup* m_popup;
    std::vector<std::pair<ui::Label*, ui::ThemeIcon*>> m_rows;
    ui::ScrollBar* m_bar = nullptr;
    double m_scroll = 0;
};

// The popup's body: the rounded card and the list in it. The compositor shadows and outlines the card,
// declared as the window's blur body; a self-drawn shadow would leave the margin to its popup frame.
class CompletionCard : public ui::View
{
public:
    explicit CompletionCard(CompletionList* list)
        : m_list(list)
    {
    }

protected:
    void layout() override
    {
        m_list->setGeometry(localRect().adjusted(kPadding, kVerticalPadding, -kPadding, -kVerticalPadding));
    }

    void paint(ui::Painter& painter) override
    {
        const ui::Theme& theme = ui::Theme::current();
        painter.drawRoundedRect(localRect(), {theme.mediumRadius, 0, ui::Corners::Circular}, theme.secondBackground);
    }

private:
    CompletionList* m_list;
};

CompletionPopup::CompletionPopup(ui::TextField* editor)
    : m_editor(editor)
{
}

CompletionPopup::~CompletionPopup() = default;

bool CompletionPopup::shown() const
{
    return m_active && m_editor->isVisible() && m_editor->hasFocus() && m_edited && !m_dismissed
            && m_model.count() > 0;
}

void CompletionPopup::edited(bool active)
{
    m_active = active;
    m_edited = true;
    m_dismissed = false;
    update();
}

void CompletionPopup::textChanged()
{
    m_currentIndex = -1;
    update();
}

void CompletionPopup::reset()
{
    m_edited = false;
    m_dismissed = false;
    m_currentIndex = -1;
    update();
}

void CompletionPopup::hide()
{
    m_window.reset();
    m_list = nullptr;
}

void CompletionPopup::update()
{
    // Empty while not editing, so the next edit lists the folders afresh.
    m_model.setText(m_edited && m_active ? m_editor->text() : std::string());
    if (!shown()) {
        hide();
        return;
    }

    ui::Window* parent = m_editor->window();
    if (!parent)
        return;
    const double width = m_editor->geometry().width;
    const double listHeight = std::min(m_model.count(), kMaximumRows) * (kRowHeight + kSpacing) - kSpacing;
    const ui::SizeF size {width, listHeight + kVerticalPadding * 2};
    if (!m_window) {
        ui::PopupPlacement placement;
        const ui::PointF origin = m_editor->mapToWindow({0, 0});
        placement.anchorRect = {origin.x, origin.y + m_editor->geometry().height + 3, 1, 1};
        placement.anchor = ui::PopupAnchor::TopLeft;
        placement.gravity = ui::PopupAnchor::BottomRight;
        placement.flipY = false;
        m_window = ui::Window::createPopup(parent, placement, false);
        auto list = std::make_unique<CompletionList>(this);
        // Rows scrolled past the list's ends stay out of the padding and the corners.
        list->setClipsChildren(true);
        m_list = list.get();
        auto card = std::make_unique<CompletionCard>(m_list);
        card->addChild(std::move(list));
        m_window->setContentView(std::move(card));
        m_window->setBackgroundColor(ui::Color {0, 0, 0, 0});
        m_window->resize(size);
        m_window->setBackgroundBlur({0, 0, size.width, size.height}, ui::Theme::current().mediumRadius);
        m_list->rebuild();
        m_window->show();
    } else {
        m_window->resize(size);
        m_window->setBackgroundBlur({0, 0, size.width, size.height}, ui::Theme::current().mediumRadius);
        m_list->resetScroll();
        m_list->rebuild();
    }
}

void CompletionPopup::move(int step)
{
    const int count = m_model.count();
    const int index = m_currentIndex < 0 ? (step > 0 ? 0 : count - 1) : m_currentIndex + step;
    m_currentIndex = std::clamp(index, 0, count - 1);
    if (m_list) {
        m_list->reveal(m_currentIndex);
        m_list->requestLayout();
    }
}

// Tab takes the lit folder, or as much as all the folders share; a single
// folder gets its slash so the next level lists at once.
void CompletionPopup::complete()
{
    const bool single = m_currentIndex >= 0 || m_model.count() == 1;
    std::string text = m_currentIndex >= 0 ? m_model.completion(m_currentIndex) : m_model.commonCompletion();
    if (single)
        text += "/";
    m_editor->setText(text);
    m_editor->setCursorPosition(text.size());
    m_edited = true;
    m_dismissed = false;
    update();
}

bool CompletionPopup::handleKey(const ui::KeyEvent& event, bool active)
{
    m_active = active;
    if (!m_active || !m_edited || m_model.count() == 0)
        return false;

    switch (event.key) {
    case ui::key::Tab:
        complete();
        return true;
    case ui::key::Down:
        if (m_dismissed) {
            m_dismissed = false;
            update();
            return true;
        }
        move(1);
        return true;
    case ui::key::Up:
        if (!shown())
            return false;
        move(-1);
        return true;
    case ui::key::PageDown:
    case ui::key::PageUp:
        if (!shown())
            return false;
        move(event.key == ui::key::PageDown ? kMaximumRows - 1 : -(kMaximumRows - 1));
        return true;
    case ui::key::Return:
    case ui::key::KeypadEnter:
        if (!shown() || m_currentIndex < 0)
            return false;
        if (onActivated)
            onActivated(m_model.path(m_currentIndex));
        return true;
    case ui::key::Escape:
        if (!shown())
            return false;
        m_dismissed = true;
        hide();
        return true;
    default:
        return false;
    }
}

} // namespace fm
