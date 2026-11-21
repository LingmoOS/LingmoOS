// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/statusbar/bar_item.h"

#include <cmath>

#include "ui/icon.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/window.h"

namespace Lingmo::Shell {

ui::Color BarContext::textColor() const
{
    return lightContents() ? ui::Color::rgb(0xFFFFFF) : ui::Color {0, 0, 0, 217};
}

ui::Color BarContext::menuTextColor() const
{
    return lightContents() ? ui::Color::rgb(0xFFFFFF) : ui::Color::rgb(0x000000);
}

// The centre of a centred item aligned to whole pixels: an odd whole width
// puts the centre on the pixel after the middle.
static double anchorCentre(double length)
{
    const int whole = int(length);
    if (whole % 2)
        return (length + 1) / 2;
    return length / 2;
}

BarItem::BarItem(BarContext *bar)
    : m_bar(bar)
{
}

void BarItem::setInsets(double left, double right)
{
    m_leftInset = left;
    m_rightInset = right;
    requestLayout();
    requestPaint();
}

void BarItem::setGlyphWidth(double width)
{
    m_glyphWidth = width;
    requestPaint();
}

void BarItem::setChecked(bool checked)
{
    if (checked == m_checked)
        return;
    m_checked = checked;
    requestPaint();
}

void BarItem::setContextMenuPressed(bool pressed)
{
    if (pressed == m_contextMenuPressed)
        return;
    m_contextMenuPressed = pressed;
    requestPaint();
}

ui::RectF BarItem::centred(const ui::SizeF &size) const
{
    const ui::RectF bounds = localRect();
    return {anchorCentre(bounds.width) - anchorCentre(size.width) + centerOffset(),
            anchorCentre(bounds.height) - anchorCentre(size.height), size.width, size.height};
}

double BarItem::plateWidth(double scale) const
{
    // The plate takes the glyph's device-pixel parity, so both snap alike
    // and the glyph stays centred.
    double pixels = std::round((geometry().width - m_leftInset - m_rightInset) * scale);
    if (m_glyphWidth > 0 && std::fmod(pixels - std::round(m_glyphWidth * scale), 2.0) != 0)
        pixels -= 1;
    return pixels / scale;
}

bool BarItem::isPlateShown() const
{
    return m_checked || m_contextMenuPressed;
}

ui::RectF BarItem::plateRect() const
{
    if (!isPlateShown())
        return {};
    const ui::Window *w = window();
    const double scale = w ? w->scale() : 1;
    const double width = plateWidth(scale);
    return {m_leftInset + (geometry().width - m_leftInset - m_rightInset - width) / 2, 4, width,
            geometry().height - 8};
}

ui::Color BarItem::plateColor() const
{
    // The verdict on what is behind the bar, as the contents' colour.
    return m_bar->lightContents() ? ui::Color {255, 255, 255, 51} : ui::Color {0, 0, 0, 26};
}

void BarItem::paint(ui::Painter &painter)
{
    m_bar->platePainted();
    const ui::RectF plate = plateRect();
    if (plate.width <= 0 || plate.height <= 0)
        return;
    painter.drawRoundedRect(plate, {ui::Theme::current().smallRadius, 0, ui::Corners::Circular}, plateColor());
}

void BarItem::pointerEnter()
{
    if (onEntered)
        onEntered();
}

void BarItem::pointerPress(const ui::PointerEvent &event)
{
    m_pressed = true;
    m_pressedButton = event.button;
    m_pressPosition = event.position;
    m_doubleClicked = false;
    if (event.clickCount == 2 && onDoubleClicked) {
        m_doubleClicked = true;
        onDoubleClicked(event);
        return;
    }
    if (onPressed)
        onPressed(event);
}

void BarItem::pointerMove(const ui::PointerEvent &event)
{
    if (onPositionChanged)
        onPositionChanged(event);
}

void BarItem::cancelPress()
{
    m_pressed = false;
    m_doubleClicked = false;
}

void BarItem::pointerRelease(const ui::PointerEvent &event)
{
    const bool click = m_pressed && localRect().contains(event.position) && !m_doubleClicked;
    m_pressed = false;
    m_doubleClicked = false;
    if (click && onClicked) {
        ui::PointerEvent clicked = event;
        clicked.button = m_pressedButton;
        onClicked(clicked);
    }
}

// The shadow's box: the content plus 16 each way, centred, 1 down.
static ui::RectF shadowBox(const ui::RectF &content)
{
    return {anchorCentre(content.width) - anchorCentre(content.width + 16),
            anchorCentre(content.height) - anchorCentre(content.height + 16) + 1, content.width + 16,
            content.height + 16};
}

// What the holder centres inside itself.
class ShadowContent final : public ui::View
{
protected:
    void layout() override
    {
        const ui::RectF box = localRect();
        for (const auto &child : children()) {
            const ui::SizeF size = m_size;
            child->setGeometry({anchorCentre(box.width) - anchorCentre(size.width),
                                anchorCentre(box.height) - anchorCentre(size.height), size.width, size.height});
        }
    }

public:
    ui::SizeF m_size;
};

// The icon itself: a file drawn on whole device pixels as IconItem puts
// its pixmap, or a FishUI glyph.
class BarIcon::Face final : public ui::View
{
public:
    void set(const std::string &file, bool tinted, const std::string &glyph, ui::Color color)
    {
        m_file = file;
        m_tinted = tinted;
        m_color = color;
        if (!glyph.empty() && !m_glyph)
            m_glyph = addChild<ui::Icon>(glyph, 16);
        if (m_glyph) {
            m_glyph->setIconName(glyph);
            m_glyph->setColor(color);
            m_glyph->setVisible(!glyph.empty());
        }
        requestLayout();
        requestPaint();
    }

protected:
    void layout() override
    {
        const ui::RectF bounds = localRect();
        if (m_glyph) {
            m_glyph->setSize(bounds.height);
            m_glyph->setGeometry(bounds);
        }
    }

    void paint(ui::Painter &painter) override
    {
        if (m_file.empty())
            return;
        const ui::Rect device = painter.snappedRect(localRect());
        const double scale = painter.scale();
        const ui::PointF offset = painter.offset();
        painter.save();
        // Artwork drawn in its own colours takes the contents' alpha.
        if (!m_tinted)
            painter.multiplyOpacity(m_color.a / 255.0);
        painter.drawSvg(m_file,
                        {device.x / scale - offset.x, device.y / scale - offset.y, device.width / scale,
                         device.height / scale},
                        m_tinted ? std::optional<ui::Color>(m_color) : std::nullopt);
        painter.restore();
    }

private:
    std::string m_file;
    bool m_tinted = false;
    ui::Color m_color;
    ui::Icon *m_glyph = nullptr;
};

BarIcon::BarIcon()
{
    // First, so it is drawn under the icon.
    m_shadow = addChild<ui::BlurredCopy>(6, 0.5);
    m_shadow->setOpacity(0.25);
    m_shadow->setVisible(false);
    auto *content = m_shadow->setContent<ShadowContent>();
    m_shadowFace = content->addChild<Face>();
    m_face = addChild<Face>();
}

void BarIcon::update()
{
    m_face->set(m_file, m_monochrome || !m_glyph.empty(), m_glyph, m_color);
    m_shadow->setVisible(m_dropShadow);
    if (m_dropShadow) {
        // The copy is black wherever the icon has coverage.
        m_shadowFace->set(m_file, true, m_glyph, ui::Color {0, 0, 0, 255});
        m_shadow->contentChanged();
    }
}

void BarIcon::setSource(std::string file, bool monochrome)
{
    if (file == m_file && monochrome == m_monochrome && m_glyph.empty())
        return;
    m_file = std::move(file);
    m_monochrome = monochrome;
    m_glyph.clear();
    update();
}

void BarIcon::setGlyph(std::string name)
{
    if (name == m_glyph && m_file.empty())
        return;
    m_file.clear();
    m_glyph = std::move(name);
    update();
}

void BarIcon::setColor(ui::Color color)
{
    if (color == m_color)
        return;
    m_color = color;
    update();
}

void BarIcon::setDropShadow(bool shadow)
{
    if (shadow == m_dropShadow)
        return;
    m_dropShadow = shadow;
    update();
}

void BarIcon::layout()
{
    const ui::RectF bounds = localRect();
    m_face->setGeometry(bounds);
    const ui::RectF box = shadowBox(bounds);
    if (box != m_shadow->geometry()) {
        m_shadow->setGeometry(box);
        static_cast<ShadowContent *>(m_shadow->content())->m_size = {bounds.width, bounds.height};
        m_shadow->contentChanged();
    }
}

BarLabel::BarLabel()
{
    m_shadow = addChild<ui::BlurredCopy>(6, 0.5);
    m_shadow->setOpacity(0.3);
    m_shadow->setVisible(false);
    auto *content = m_shadow->setContent<ShadowContent>();
    m_shadowText = content->addChild<ui::Label>();
    m_shadowText->setColor(ui::Color {0, 0, 0, 255});
    m_label = addChild<ui::StyledLabel>();
    for (ui::Label *label : {m_shadowText, static_cast<ui::Label *>(m_label)}) {
        label->setFontSize(BarContext::kFontPointSize * 4 / 3);
        label->setElide(ui::Label::Elide::None);
    }
    m_label->setSubpixelAntialiasing(false);
}

void BarLabel::setText(std::string text)
{
    if (text == m_label->text())
        return;
    m_label->setText(text);
    m_shadowText->setText(std::move(text));
    requestLayout();
    m_shadow->contentChanged();
}

const std::string &BarLabel::text() const
{
    return m_label->text();
}

void BarLabel::setWeight(int weight)
{
    if (weight == m_weight)
        return;
    m_weight = weight;
    m_label->setWeight(weight);
    m_shadowText->setWeight(weight);
    requestLayout();
    m_shadow->contentChanged();
}

void BarLabel::setDropShadow(bool shadow)
{
    // The faint edge hides the glyphs' subpixel fringes on the wallpaper.
    m_label->setStyle(shadow ? ui::StyledLabel::Style::Raised : ui::StyledLabel::Style::Normal,
                      ui::Color {0, 0, 0, 36});
    m_shadow->setVisible(shadow);
}

void BarLabel::setTextColor(ui::Color color)
{
    m_label->setColor(color);
}

ui::SizeF BarLabel::sizeHint() const
{
    return m_label->sizeHint();
}

void BarLabel::layout()
{
    const ui::RectF bounds = localRect();
    m_label->setGeometry(bounds);
    const ui::RectF box = shadowBox(bounds);
    if (box != m_shadow->geometry()) {
        m_shadow->setGeometry(box);
        static_cast<ShadowContent *>(m_shadow->content())->m_size = {bounds.width, bounds.height};
        m_shadow->contentChanged();
    }
}

} // namespace Lingmo::Shell
