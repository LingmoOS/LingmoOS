// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#pragma once


#include <functional>
#include <optional>
#include <string>

#include "ui/geometry.h"
#include "ui/blurred_copy.h"
#include "ui/styled_label.h"
#include "ui/view.h"

namespace Lingmo::ui {
class Icon;
} // namespace Lingmo::UI

namespace Lingmo::Shell {

// The bar's look, shared by every item.
class BarContext
{
public:
    static constexpr double kIconSize = 16;
    static constexpr double kFontPointSize = 10;
    static constexpr double kEdgeMargin = 12;
    static constexpr double kItemGap = 12;
    static constexpr double kHighlightGap = 6;
    static constexpr double kSmallSpacing = 6;

    virtual ~BarContext() = default;

    // What is behind the bar is dark, so its contents are light and cast a
    // shadow.
    virtual bool lightContents() const = 0;
    virtual bool lightContentsAt(const ui::RectF&) const { return lightContents(); }
    // #FFFFFF on a dark backdrop, black at 85% otherwise.
    ui::Color textColor() const;
    // The menu bar's: #FFFFFF on a dark backdrop, black otherwise.
    ui::Color menuTextColor() const;
    // An item painted, with its plate or without; the bar puts its blur
    // behind the plate shown, in the frame that shows it.
    virtual void platePainted() {}
};

// A clickable slot of the bar with the rounded plate behind it,
// inset so that neighbouring plates keep their gap. Takes left, right and
// middle presses; the wheel passes through.
class BarItem : public ui::View
{
public:
    explicit BarItem(BarContext *bar);

    void setInsets(double left, double right);
    double leftInset() const { return m_leftInset; }
    double rightInset() const { return m_rightInset; }
    // Keeps content centred over the plate rather than the whole item.
    double centerOffset() const { return (m_leftInset - m_rightInset) / 2; }
    // A lone glyph's width: the plate takes its device-pixel parity.
    void setGlyphWidth(double width);

    void setChecked(bool checked);
    bool isChecked() const { return m_checked; }
    void setContextMenuPressed(bool pressed);

    bool isPressed() const { return m_pressed; }
    // Forgets the held press, which a drag took over.
    void cancelPress();
    // Where the held press went down, in item coordinates.
    ui::PointF pressPosition() const { return m_pressPosition; }
    // The plate as drawn, in item coordinates; empty while hidden.
    ui::RectF plateRect() const;
    bool isPlateShown() const;
    ui::Color plateColor() const;

    // MouseArea's: clicked on a release inside after a press, doubleClicked
    // on a second press (which then gives no click) when set.
    std::function<void(const ui::PointerEvent &)> onClicked;
    std::function<void(const ui::PointerEvent &)> onPressed;
    std::function<void(const ui::PointerEvent &)> onDoubleClicked;
    std::function<void(const ui::PointerEvent &)> onPositionChanged;
    std::function<void()> onEntered;

    // Repaints after the bar's look or panels changed.
    void barChanged() { requestPaint(); }

    // Centred with the item's centerOffset, at whole logical pixels.
    ui::RectF centred(const ui::SizeF &size) const;

    bool acceptsPointer() const override { return true; }

protected:
    void pointerEnter() override;
    void pointerPress(const ui::PointerEvent &event) override;
    void pointerMove(const ui::PointerEvent &event) override;
    void pointerRelease(const ui::PointerEvent &event) override;
    void paint(ui::Painter &painter) override;

    BarContext *bar() const { return m_bar; }

private:
    double plateWidth(double scale) const;

    BarContext *m_bar;
    double m_leftInset = 0;
    double m_rightInset = 0;
    double m_glyphWidth = 0;
    bool m_checked = false;
    bool m_contextMenuPressed = false;
    bool m_pressed = false;
    bool m_doubleClicked = false;
    ui::MouseButton m_pressedButton = ui::MouseButton::Left;
    ui::PointF m_pressPosition;
};

// An SVG file in its own colours, or tinted, or one of FishUI's
// glyphs in a colour, snapped to device pixels as IconItem; a soft shadow
// when the bar's contents are light.
class BarIcon : public ui::View
{
public:
    BarIcon();

    // A file; empty draws nothing.
    void setSource(std::string file, bool monochrome = false);
    // A FishUI glyph, in place of the file.
    void setGlyph(std::string name);
    void setColor(ui::Color color);
    void setDropShadow(bool shadow);

protected:
    void layout() override;

private:
    class Face;

    void update();

    std::string m_file;
    bool m_monochrome = false;
    std::string m_glyph;
    ui::Color m_color;
    bool m_dropShadow = false;
    // The black copy, blurred, under the icon (MultiEffect, blurMax 6, blur 0.5).
    ui::BlurredCopy *m_shadow = nullptr;
    Face *m_shadowFace = nullptr;
    Face *m_face = nullptr;
};

// 10 pt text in the bar's colour; with the shadow, the 1 px
// Raised edge of rgba(0, 0, 0, 0.14) over a blurred black copy.
class BarLabel : public ui::View
{
public:
    BarLabel();

    void setText(std::string text);
    const std::string &text() const;
    void setWeight(int weight);
    void setDropShadow(bool shadow);
    void setTextColor(ui::Color color);

    // The text's implicit size.
    ui::SizeF sizeHint() const override;

protected:
    void layout() override;

private:
    ui::BlurredCopy *m_shadow;
    ui::Label *m_shadowText;
    ui::StyledLabel *m_label;
    int m_weight = 400;
};

} // namespace Lingmo::Shell

