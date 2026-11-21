// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/language/preferredlanguagelist.h"

#include <algorithm>
#include <chrono>
#include <cmath>

#include "base/i18n.h"
#include "pages/language/language.h"
#include "ui/color_provider.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/menu.h"
#include "ui/cell_layout.h"
#include "ui/painter.h"
#include "ui/scroll_view.h"
#include "ui/theme.h"
#include "ui/window.h"
#include "widgets/colors.h"
#include "widgets/metrics.h"

using namespace Lingmo;
using namespace Lingmo::ui;
using Lingmo::i18n::tr;

namespace settings {

// How far the pointer moves before a press on a handle becomes a drag.
static constexpr double kStartDragDistance = 10;


// "System language" on the first row: a pill in the accent colour.
class SystemLanguageBadge : public View
{
public:
    SystemLanguageBadge()
    {
        m_label = addChild<Label>(tr("System language"));
        m_label->setWeight(500);
    }

    // Before the badge is measured: its size follows the font.
    void applyTheme()
    {
        const Theme& theme = Theme::current();
        m_label->setFontSize(std::max(1.0, theme.fontSize() - 2));
        m_label->setColor(theme.highlight);
    }

    SizeF sizeHint() const override
    {
        const SizeF text = m_label->sizeHint();
        return {std::ceil(text.width) + 16, std::round(m_label->tightBoundingRect().height) + 8};
    }

protected:
    void layout() override
    {
        applyTheme();
        const SizeF text = m_label->sizeHint();
        const double height = geometry().height;
        // Baselines that centre all the ink and just the capitals: the first
        // reads high once descenders count, the second low.
        const RectF ink = m_label->tightBoundingRect();
        const double inkBaseline = (height - ink.height) / 2 - ink.y;
        const double capBaseline = (height + m_label->capHeight()) / 2;
        const double y = std::round((inkBaseline + capBaseline) / 2 - m_label->baselineOffset());
        m_label->setGeometry({rounding::centeredStart(geometry().width, text.width), y, text.width, text.height});
    }

    void paint(Painter& painter) override
    {
        const RectF rect = localRect();
        painter.fillRoundedRect(rect, rect.height / 2, color(SettingsColor::PreferredLanguageTag));
    }

private:
    Label* m_label;
};

// The drag handle: three short lines, a hand cursor, and Up and Down to move
// the row without a pointer.
class ReorderHandle : public View
{
public:
    explicit ReorderHandle(PreferredLanguageList& list)
        : m_list(list)
    {
    }

    int index = -1;

    SizeF sizeHint() const override { return {32, 32}; }
    bool acceptsPointer() const override { return true; }
    bool acceptsFocus() const override { return true; }
    CursorShape cursorShape() const override { return m_held ? CursorShape::Grabbing : CursorShape::Grab; }

protected:
    void paint(Painter& painter) override
    {
        const Color color = Theme::current().disabledText;
        const double x = rounding::centeredStart(geometry().width, 14);
        const double scale = painter.scale();
        const int thickness = std::max(1, int(std::lround(1.2 * scale)));
        const double blockHeight = (std::lround(8 * scale) + thickness) / scale;
        const double y = (std::floor((geometry().height - blockHeight) / 2 * scale) + 0.25) / scale;
        for (int line = 0; line < 3; ++line)
            painter.drawHorizontalLine(x, y + line * 4, 14, color, thickness);
    }

    void pointerPress(const PointerEvent& event) override
    {
        if (event.button != MouseButton::Left)
            return;
        m_held = true;
        setFocus(FocusReason::Mouse);
        m_list.press(index, listPoint(event.position));
    }

    void pointerMove(const PointerEvent& event) override
    {
        if (m_held && m_list.pressedIndex() >= 0)
            m_list.updatePointer(listPoint(event.position));
    }

    void pointerRelease(const PointerEvent& event) override
    {
        if (event.button != MouseButton::Left || !m_held)
            return;
        m_held = false;
        m_list.release();
    }

    bool keyPress(const KeyEvent& event) override
    {
        if (event.key == key::Escape && m_list.isDragging()) {
            m_list.cancel();
            return true;
        }
        if (event.key == key::Up) {
            m_list.move(index, index - 1);
            return true;
        }
        if (event.key == key::Down) {
            m_list.move(index, index + 1);
            return true;
        }
        return false;
    }

private:
    PointF listPoint(PointF position) const { return m_list.mapFromWindow(mapToWindow(position)); }

    PreferredLanguageList& m_list;
    bool m_held = false;
};

// One preferred language: its own name, the badge on the first, its name in
// the interface's language below, the drag handle at the end, and a
// separator under every row but the last.
class PreferredLanguageRow : public View
{
public:
    PreferredLanguageRow(PreferredLanguageList& list, const LanguageEntry& entry, int index, bool last)
        : m_list(list)
        , m_index(index)
        , m_last(last)
    {
        m_row = addChild<RowLayout>();
        m_row->setSpacing(metrics::rowSpacing);
        ColumnLayout* names = m_row->addChild<ColumnLayout>();
        names->setSpacing(metrics::textSpacing);
        names->setLayoutHints({.fillWidth = true});
        RowLayout* top = names->addChild<RowLayout>();
        top->setSpacing(metrics::s);
        top->setLayoutHints({.fillWidth = true});
        m_name = top->addChild<Label>(entry.name);
        if (index == 0) {
            m_badge = top->addChild<SystemLanguageBadge>();
            m_badge->setLayoutHints({.verticalAlignment = Alignment::Center});
        }
        top->addChild<View>()->setLayoutHints({.fillWidth = true});
        m_description = names->addChild<Label>(entry.description);
        m_description->setLayoutHints({.fillWidth = true});
        m_handle = m_row->addChild<ReorderHandle>(list);
        // The lines, not the drag area around them, end where a switch does.
        m_handle->setLayoutHints({.margins = {0, 0, -(32 - 14) / 2.0, 0}});
        m_handle->index = index;
        setName("language:" + entry.code);
        m_handle->setName("languageDragHandle_" + std::to_string(index));
    }

    ReorderHandle* handle() const { return m_handle; }

    SizeF sizeHint() const override { return {0, PreferredLanguageList::kRowHeight}; }
    bool acceptsPointer() const override { return true; }

protected:
    void layout() override
    {
        const Theme& theme = Theme::current();
        m_name->setFontSize(theme.fontSize());
        m_description->setFontSize(std::max(1.0, theme.fontSize() - 1));
        m_description->setColor(theme.disabledText);
        if (m_badge)
            m_badge->applyTheme();
        // Shrinks to elide but never grows, so the badge stays next to the name.
        m_name->setLayoutHints({.fillWidth = true, .maximumWidth = std::ceil(m_name->sizeHint().width)});
        m_row->setGeometry(localRect());
    }

    void paint(Painter& painter) override
    {
        if (m_last)
            return;
        // One device pixel at any scale.
        const Color line = color(ColorId::Separator);
        painter.drawHorizontalLine(0, geometry().height - 1 / painter.scale(), geometry().width, line);
    }

    void pointerPress(const PointerEvent& event) override
    {
        if (event.button == MouseButton::Right)
            m_list.showActions(m_index, mapToWindow(event.position));
    }

private:
    PreferredLanguageList& m_list;
    int m_index;
    bool m_last;
    RowLayout* m_row;
    Label* m_name;
    SystemLanguageBadge* m_badge = nullptr;
    Label* m_description;
    ReorderHandle* m_handle;
};

// Where a dragged row would land: a line with a dot at its start.
class PreferredLanguageList::Indicator : public View
{
public:
    explicit Indicator(PreferredLanguageList& list)
        : m_list(list)
    {
    }

protected:
    void paint(Painter& painter) override
    {
        if (!m_list.isDragging() || !m_list.isInside() || m_list.targetIndex() == m_list.pressedIndex())
            return;
        const int slot = m_list.targetIndex() > m_list.pressedIndex() ? m_list.targetIndex() + 1
                                                                       : m_list.targetIndex();
        // The list clips, so the first and last slots move inwards to keep the dot whole.
        const double center = std::clamp(slot * kRowHeight - m_list.scrollY(), 3.0, geometry().height - 3);
        const Color color = Theme::current().highlight;
        painter.fillRoundedRect({3, center - 1, geometry().width - 3, 2}, 1, color);
        painter.fillRoundedRect({0, center - 3, 6, 6}, 3, color);
    }

private:
    PreferredLanguageList& m_list;
};

PreferredLanguageList::PreferredLanguageList(Language& language)
    : m_language(language)
{
    setName("preferredLanguageList");
    m_scroll = addChild<ScrollView>();
    m_scroll->setElastic(false);
    // The drag handles reach into the bar's strip and must take the press.
    m_scroll->setBarsBelowContent(true);
    m_scroll->setName("preferredLanguageView");
    m_rows = m_scroll->setContent<ColumnLayout>();
    m_rows->setSpacing(0);
    m_indicator = addChild<Indicator>(*this);
    setClipsChildren(true);
    setLayoutHints({.fillWidth = true});

    m_edgeTimer.setInterval(std::chrono::milliseconds(16));
    m_edgeTimer.onTimeout = [this] { scrollAtEdge(); };
    m_scroll->onScrolled = [this] { m_indicator->requestPaint(); };

    m_subscriptions.push_back(m_language.preferredLanguagesChanged.connect([this] {
        cancel();
        rebuild();
    }));
    m_subscriptions.push_back(m_language.savingChanged.connect([this] { updateStates(); }));
    rebuild();
}

PreferredLanguageList::~PreferredLanguageList() = default;

int PreferredLanguageList::count() const
{
    return int(m_language.preferredLanguages().size());
}

double PreferredLanguageList::scrollY() const
{
    return m_scroll->scrollPosition().y;
}

SizeF PreferredLanguageList::sizeHint() const
{
    return {0, heightForWidth(0)};
}

double PreferredLanguageList::heightForWidth(double) const
{
    return std::min(kRowHeight * kVisibleRows, kRowHeight * count());
}

void PreferredLanguageList::rebuild()
{
    while (!m_rows->children().empty())
        m_rows->takeChild(m_rows->children().front().get());
    const auto& languages = m_language.preferredLanguages();
    for (size_t i = 0; i < languages.size(); ++i) {
        PreferredLanguageRow* row = m_rows->addChild<PreferredLanguageRow>(*this, languages[i], int(i),
                                                                           i + 1 == languages.size());
        row->setLayoutHints({.fillWidth = true});
    }
    updateStates();
    requestLayout();
}

void PreferredLanguageList::updateStates()
{
    m_scroll->setEnabled(!m_language.isSaving());
    for (const auto& child : m_rows->children()) {
        auto* row = static_cast<PreferredLanguageRow*>(child.get());
        row->setOpacity(m_dragging && m_pressedIndex == row->handle()->index ? 0.45 : 1);
    }
    m_indicator->requestPaint();
}

void PreferredLanguageList::layout()
{
    m_scroll->setGeometry(localRect());
    m_indicator->setGeometry(localRect());
}

// Above or below the rows a drop goes first or last; only leaving the list
// sideways cancels it.
bool PreferredLanguageList::isInside() const
{
    return m_pointer.x >= 0 && m_pointer.x <= geometry().width;
}

void PreferredLanguageList::press(int index, PointF point)
{
    m_pointerHeld = true;
    m_pressedIndex = index;
    m_targetIndex = index;
    m_pointer = point;
    m_pressY = point.y;
    m_grabOffset = point.y + scrollY() - index * kRowHeight;
}

void PreferredLanguageList::updatePointer(PointF point)
{
    m_pointer = point;
    if (!m_dragging && count() > 1 && std::abs(m_pointer.y - m_pressY) >= kStartDragDistance) {
        m_dragging = true;
        m_edgeTimer.start();
        updateStates();
    }
    if (m_dragging)
        updateTarget();
}

void PreferredLanguageList::updateTarget()
{
    const double center = m_pointer.y - m_grabOffset + kRowHeight / 2 + scrollY();
    m_targetIndex = std::clamp(int(std::floor(center / kRowHeight)), 0, count() - 1);
    m_indicator->requestPaint();
}

void PreferredLanguageList::cancel()
{
    m_dragging = false;
    m_pressedIndex = -1;
    m_targetIndex = -1;
    m_edgeTimer.stop();
    updateStates();
}

void PreferredLanguageList::release()
{
    m_pointerHeld = false;
    const int from = m_pressedIndex;
    const int to = m_targetIndex;
    const bool move = m_dragging && isInside() && from != to;
    cancel();
    if (move)
        this->move(from, to);
}

void PreferredLanguageList::move(int from, int to)
{
    if (onMoveRequested)
        onMoveRequested(from, to);
    else
        m_language.moveLanguage(from, to);
}

void PreferredLanguageList::scrollAtEdge()
{
    if (!m_dragging || !isInside())
        return;
    const double edge = kRowHeight;
    const double height = geometry().height;
    const double y = std::clamp(m_pointer.y, 0.0, height);
    const double delta = y < edge            ? -8 * (1 - y / edge)
                         : y > height - edge ? 8 * (1 - (height - y) / edge)
                                             : 0;
    if (delta == 0)
        return;
    const double maximum = std::max(0.0, m_scroll->contentSize().height - height);
    m_scroll->setScrollPosition({0, std::clamp(scrollY() + delta, 0.0, maximum)});
    updateTarget();
}

void PreferredLanguageList::showActions(int index, PointF windowPosition)
{
    m_actions = std::make_unique<Menu>();
    const size_t remove =
            m_actions->addItem(tr("Remove language"), [this, index] { m_language.removeLanguage(index); });
    m_actions->setItemEnabled(remove, count() > 1);
    m_actions->popup(window(), windowPosition);
}

} // namespace settings
