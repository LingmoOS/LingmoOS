// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <utility>

#include "ui/layout.h"
#include "ui/cell_layout.h"

namespace Lingmo::ui {
class Animation;
class Icon;
class Label;
class ListView;
class Switch;
}

namespace settings {

// A view at the end of a row whose box reaches past the row's content edge,
// so its drawing, not its box, ends where a switch does.
class TrailingOverhang
{
public:
    virtual ~TrailingOverhang() = default;
    virtual double trailingOverhang() const = 0;
};

// One row of a list card. The press background fills the whole row and only
// rounds the corners it shares with the card, which the row finds out from
// its visible sibling rows; a separator runs between rows. Content goes into
// the row after the label, a leading view before it, and details unfold
// below the row while it is expanded.
class ListRow : public Lingmo::ui::View
{
public:
    explicit ListRow(std::string text = {});
    ~ListRow() override;

    void setText(std::string text);
    const std::string& text() const { return m_text; }

    template<typename T, typename... Args>
    T* addContent(Args&&... args)
    {
        return m_row->addChild<T>(std::forward<Args>(args)...);
    }
    Lingmo::ui::RowLayout* contentRow() const { return m_row; }
    void setContentSpacing(double spacing);

    // Goes before the label: an icon, a colour swatch.
    template<typename T, typename... Args>
    T* setLeading(Args&&... args)
    {
        auto view = std::make_unique<T>(std::forward<Args>(args)...);
        T* leading = view.get();
        setLeadingView(std::move(view));
        return leading;
    }
    void setLeadingView(std::unique_ptr<Lingmo::ui::View> view);

    // A panel that unfolds below the row while the row is expanded.
    Lingmo::ui::ColumnLayout* details();
    void setExpanded(bool expanded);
    bool isExpanded() const { return m_expanded; }

    // A row that only holds a control, such as a switch, neither highlights
    // nor reacts to clicks.
    void setInteractive(bool interactive);
    bool isInteractive() const { return m_interactive; }
    // Off for a row whose actions are buttons inside it.
    void setHighlightOnPress(bool highlight) { m_highlightOnPress = highlight; }
    void setSeparatorVisible(bool visible);
    void setMinimumRowHeight(double height);
    // Lifts the label by half this, onto a control with captions hanging below it.
    void setLabelBottomMargin(double margin);

    // For a row a ListView made: the view reuses rows, so they are not
    // siblings in row order, and the row's neighbours come from the view.
    // Call from bindItem with the row's index; a null view drops it.
    void setViewIndex(Lingmo::ui::ListView* view, int index);

    bool isHighlighted() const;
    // The row itself, without its details.
    double rowHeight() const;

    Lingmo::ui::SizeF sizeHint() const override;
    double heightForWidth(double width) const override;
    bool acceptsPointer() const override { return m_interactive; }
    bool acceptsFocus() const override { return m_interactive; }

    std::function<void()> onClicked;
    // isHovered() changed: the pointer is over the row, whatever is under it
    // inside the row, as a HoverHandler on it has it.
    std::function<void()> onHoveredChanged;

protected:
    void layout() override;
    void paint(Lingmo::ui::Painter& painter) override;
    void enabledChanged() override;
    void hoverChanged() override;
    void pointerPress(const Lingmo::ui::PointerEvent& event) override;
    void pointerRelease(const Lingmo::ui::PointerEvent& event) override;
    bool keyPress(const Lingmo::ui::KeyEvent& event) override;

    Lingmo::ui::Label* label() const { return m_label; }

private:
    // The visible list rows among the parent's children, around this one.
    struct Neighbours
    {
        bool first = true;
        bool last = true;
        ListRow* previous = nullptr;
        ListRow* next = nullptr;
    };
    Neighbours neighbours() const;
    double rowHeightForWidth(double width) const;
    double detailsHeight(double width) const;
    void setPressed(bool pressed);

    void updateLabelHints();

    std::string m_text;
    Lingmo::ui::RowLayout* m_row;
    // QML's Row of leading items, shown while it holds one.
    Lingmo::ui::Row* m_leadingBox;
    Lingmo::ui::View* m_leading = nullptr;
    Lingmo::ui::Label* m_label;
    Lingmo::ui::View* m_detailsClip = nullptr;
    Lingmo::ui::ColumnLayout* m_details = nullptr;
    std::unique_ptr<Lingmo::ui::Animation> m_detailsAnimation;
    // 0 folded, 1 unfolded; animated between.
    double m_unfolded = 0;
    double m_unfoldedFrom = 0;
    bool m_expanded = false;
    bool m_interactive = true;
    bool m_highlightOnPress = true;
    bool m_separatorVisible = true;
    bool m_pressed = false;
    double m_minimumRowHeight;
    double m_labelBottomMargin = 0;
    Lingmo::ui::ListView* m_view = nullptr;
    int m_viewIndex = -1;
};

// FishUI's switch with `rightPadding: 0` at the end of a row: its 8 px left
// padding, then the indicator flush with the content edge.
void setTrailingSwitchPadding(Lingmo::ui::Switch* toggle);

// A row holding only a switch at its end, flush with the content edge; the
// row itself does not react to clicks.
class SwitchRow : public ListRow
{
public:
    explicit SwitchRow(std::string text = {});

    Lingmo::ui::Switch* control() const { return m_switch; }
    void setChecked(bool checked);
    bool isChecked() const;

    // After the user flipped the switch.
    std::function<void(bool checked)> onToggled;

private:
    Lingmo::ui::Switch* m_switch;
};

// A read-only row: the label, then its value in secondary text. Hidden while
// there is no value.
class InfoRow : public ListRow
{
public:
    explicit InfoRow(std::string text = {}, std::string value = {});

    void setValue(std::string value);

protected:
    void layout() override;

private:
    Lingmo::ui::Label* m_value;
};

// One of FishUI's glyphs at the end of a row.
class TrailingIcon : public Lingmo::ui::View, public TrailingOverhang
{
public:
    explicit TrailingIcon(std::string name, double size = 16);

    Lingmo::ui::Icon* icon() const { return m_icon; }

    Lingmo::ui::SizeF sizeHint() const override { return {m_size, m_size}; }
    double trailingOverhang() const override { return m_overhang; }

protected:
    void layout() override;

private:
    Lingmo::ui::Icon* m_icon;
    double m_size;
    double m_overhang;
};

// A row that leads to another page.
class NavigationItem : public ListRow
{
public:
    explicit NavigationItem(std::string text = {});

    // The current choice, shown before the chevron.
    void setValue(std::string value);

protected:
    void layout() override;

private:
    Lingmo::ui::Label* m_value;
};

} // namespace settings
