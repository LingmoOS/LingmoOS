// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "decoration/decoration_node.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <format>
#include <string>
#include <vector>

#include "decoration/caption_button.h"
#include "decoration/decoration.h"
#include "decoration/decoration_theme.h"
#include "decoration/text.h"
#include "wm/window.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Decoration {

namespace {

std::string colorKey(const render::Color& color)
{
    return std::format("{:.3f},{:.3f},{:.3f},{:.3f}", color.r, color.g, color.b, color.a);
}

const char* iconName(CaptionButtonType type, bool restore)
{
    switch (type) {
    case CaptionButtonType::Minimize:
        return "minimize";
    case CaptionButtonType::Maximize:
        return restore ? "restore" : "maximize";
    case CaptionButtonType::Close:
        return "close";
    }
    return "";
}

// The centre of the last pixels inside: an even width rounds toward the origin.
base::Point innerCenter(const base::Rect& rect)
{
    return {(2 * rect.x() + rect.width() - 1) / 2, (2 * rect.y() + rect.height() - 1) / 2};
}

// A rect of `size` whose innerCenter() is `center`.
base::Rect centeredAt(const base::Size& size, const base::Point& center)
{
    return base::Rect(center.x() - (size.width() - 1) / 2, center.y() - (size.height() - 1) / 2,
                      size.width(), size.height());
}

} // namespace

DecorationNode::DecorationNode(const Decoration* decoration, Theme* theme)
    : scene::Node("decoration")
    , m_decoration(decoration)
    , m_theme(theme)
{
}

DecorationNode::~DecorationNode() = default;

void DecorationNode::releaseRenderResources()
{
    m_captions.clear();
    scene::Node::releaseRenderResources();
}

void DecorationNode::paint(render::Renderer& renderer, const base::Rect& geometry) const
{
    if (!m_decoration || !m_decoration->window() || geometry.isEmpty()
        || !renderer.drawsInto(geometry))
        return;
    paintFrame(renderer, geometry);
    paintCaption(renderer, geometry);
    paintButtons(renderer, geometry);
}

void DecorationNode::paintFrame(render::Renderer& renderer, const base::Rect& bounds) const
{
    const auto insets = m_decoration->window()->frameInsets();
    const auto& color = m_theme->palette().titleBar;
    auto frameColor = m_decoration->isActive()
        ? m_theme->visualStyle().activeOutline : m_theme->visualStyle().inactiveOutline;
    frameColor.a = 1;
    const int top = std::min(bounds.height(), insets.top);
    const int bottom = std::min(bounds.height() - top, insets.bottom);
    renderer.fillRect(base::Rect(bounds.x(), bounds.y(), bounds.width(), top), color);
    renderer.fillRect(base::Rect(bounds.x(), bounds.y() + top, insets.left,
                                 bounds.height() - top - bottom), frameColor);
    renderer.fillRect(base::Rect(bounds.x() + bounds.width() - insets.right, bounds.y() + top,
                                 insets.right, bounds.height() - top - bottom), frameColor);
    renderer.fillRect(base::Rect(bounds.x(), bounds.y() + bounds.height() - bottom,
                                 bounds.width(), bottom), frameColor);
}

void DecorationNode::paintCaption(render::Renderer& renderer, const base::Rect& bounds) const
{
    const Metrics& metrics = m_theme->metrics();
    const base::Rect area = m_decoration->captionArea().translated(bounds.origin());
    const std::string title = m_decoration->title();
    if (area.width() <= 0 || title.empty())
        return;

    const render::Color color = m_theme->captionColor(m_decoration->isActive());
    const double scale = renderer.rasterScale();
    const std::string key = title + '\n' + colorKey(color)
        + std::format("\n{}:{}", metrics.textPixelSize, area.width());
    auto caption = std::find_if(m_captions.begin(), m_captions.end(), [&](const Caption& entry) {
        return entry.scale == scale;
    });
    if (caption == m_captions.end() || caption->key != key) {
        const std::string text =
                m_theme->textRenderer().elideText(title, metrics.textPixelSize, area.width());
        const Image image =
            m_theme->textRenderer().renderText(text, metrics.textPixelSize, color, scale);
        if (image.size().isEmpty())
            return;
        // Padded to the pixels its whole-unit target covers, so every texel
        // lands on one physical pixel instead of being resampled.
        const base::Size logical(int(std::ceil(image.size().width() / scale)),
                                 int(std::ceil(image.size().height() / scale)));
        const base::Size pixels(base::roundToInt(logical.width() * scale),
                                base::roundToInt(logical.height() * scale));
        std::vector<uint8_t> padded(size_t(pixels.width()) * pixels.height() * 4, 0);
        for (int row = 0; row < image.size().height(); ++row) {
            std::memcpy(padded.data() + size_t(row) * pixels.width() * 4,
                        image.bits() + size_t(row) * image.stride(), size_t(image.stride()));
        }
        auto texture = renderer.createTexture(pixels, padded.data(), pixels.width() * 4,
                                              render::PixelFormat::Rgba8888);
        if (!texture)
            return;
        if (caption == m_captions.end()) {
            // Two scales cover a mixed pair of outputs; a third is a scale change.
            if (m_captions.size() >= 2)
                m_captions.erase(m_captions.begin());
            caption = m_captions.insert(m_captions.end(), Caption {scale});
        }
        caption->key = key;
        caption->texture = std::move(texture);
    }
    render::Texture* texture = caption->texture.get();

    // Centred in the titlebar, and moved into the space beside the buttons when
    // centring would put it under them: the rule the Lingmo KWin decoration
    // used, which keeps the caption off the buttons without ever centring it in
    // a box that is not the window.
    const base::Size size(base::roundToInt(texture->size().width() / scale),
                          base::roundToInt(texture->size().height() / scale));
    base::Rect target(base::Point(0, 0), size);
    target.moveLeft(bounds.left() + (bounds.width() - size.width()) / 2);
    if (target.left() < area.left())
        target.moveLeft(area.left());
    else if (target.right() > area.right())
        target.moveRight(area.right());
    target.moveTop(bounds.top()
                   + (std::min(bounds.height(), metrics.titleBarHeight) - size.height()) / 2);
    // Mapped, so the texels land on whole physical pixels.
    renderer.drawTextureMapped(texture, target, 1.0f, base::AffineTransform());
}

void DecorationNode::paintButtons(render::Renderer& renderer, const base::Rect& bounds) const
{
    const int rasterDensity = int(std::ceil(renderer.rasterScale()));
    const Metrics& metrics = m_theme->metrics();
    const Palette& palette = m_theme->palette();
    const render::Color iconColor = palette.buttonIcon;

    for (const CaptionButton& button : m_decoration->buttons()) {
        const base::Rect geometry = button.geometry().translated(bounds.origin());
        if (!bounds.contains(innerCenter(geometry)))
            continue;

        if (button.isHovered() || button.isPressed()) {
            const render::Color wash =
                button.isPressed() ? palette.buttonPressed : palette.buttonHover;
            const std::string key = std::format("{}wash:{}:{}", rasterDensity,
                                                metrics.buttonSize, colorKey(wash));
            render::Texture* texture = m_theme->texture(renderer, key, [&] {
                return renderButtonHighlight(metrics.buttonSize * rasterDensity, wash);
            });
            if (texture) {
                const base::Size size = (base::SizeF(texture->size()) / rasterDensity).toSize();
                renderer.drawTexture(texture, centeredAt(size, innerCenter(geometry)));
            }
        }

        const bool restore = m_decoration->isRestoreButton();
        const std::string key =
            std::format("{}icon:{}:{}:{}", rasterDensity, iconName(button.type(), restore),
                        metrics.iconSize, colorKey(iconColor));
        render::Texture* texture = m_theme->texture(renderer, key, [&] {
            return renderCaptionIcon(button.type(), restore, metrics.iconSize * rasterDensity,
                                     iconColor);
        });
        if (!texture)
            continue;
        const base::Size size = (base::SizeF(texture->size()) / rasterDensity).toSize();
        renderer.drawTexture(texture, centeredAt(size, innerCenter(geometry)));
    }
}

} // namespace Lingmo::Decoration
