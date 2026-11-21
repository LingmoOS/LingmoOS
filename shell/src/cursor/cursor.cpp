// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cursor/cursor.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <map>
#include <string>
#include <utility>

#include "base/timer.h"
#include "input/seat.h"
#include "compositor/compositor.h"
#include "input/types.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Cursor {

class Cursor::ImageNode final : public scene::Node
{
public:
    explicit ImageNode(int size)
        : scene::Node("system-cursor")
        , m_size(size)
    {
        m_timer.setSingleShot(true);
        m_timer.onTimeout = [this] {
            m_frame = (m_frame + 1) % m_frames.size();
            if (onFrameChanged)
                onFrameChanged();
            else
                notifyChanged();
            startTimer();
        };
    }

    void setVisualScale(double scale)
    {
        if (scale == m_visualScale) return;
        m_visualScale = scale;
        if (!m_frames.empty())
            setGeometry(base::Rect(position(), (base::SizeF(m_frames.front().image.size()) * (scale / m_density)).toSize()));
        notifyChanged();
    }

    void setDensity(int density)
    {
        if (density == m_density)
            return;
        m_density = density;
        m_timer.stop();
        m_frames.clear();
    }

    int density() const { return m_density; }

    bool setTheme(const std::string &theme, int size)
    {
        if (m_theme == theme && m_size == size)
            return false;
        m_theme = theme;
        m_size = size;
        m_timer.stop();
        m_frames.clear();
        m_loaded.clear();
        return true;
    }

    void setPointerPosition(const base::PointF &position)
    {
        const base::Point integral = position.toPoint();
        const base::PointF remainder = position - base::PointF(integral);
        if (!input::fuzzyEqual(remainder, m_fractionalOffset)) {
            m_fractionalOffset = remainder;
            notifyChanged();
        }
        setPosition(integral - hotspot());
    }

    void setShape(input::CursorShape shape, bool visible)
    {
        if (shape != m_shape || m_frames.empty()) {
            m_timer.stop();
            m_shape = shape;
            m_frames = frames(shape);
            m_frame = 0;
            if (!m_frames.empty())
                setGeometry(base::Rect(
                        position(),
                        (base::SizeF(m_frames.front().image.size()) * (m_visualScale / m_density)).toSize()));
            notifyChanged();
        }
        m_wantsDisplay = visible;
        setVisible(visible && !m_frames.empty());
        if (!isVisible())
            m_timer.stop();
        else if (m_frames.size() > 1 && !m_timer.isActive())
            startTimer();
    }

    base::Point hotspot() const
    {
        return m_frames.empty() ? base::Point()
                                : (base::PointF(m_frames.front().hotspot) * (m_visualScale / m_density)).toPoint();
    }

    const Frame *currentFrame() const
    {
        return m_frame < m_frames.size() ? &m_frames[m_frame] : nullptr;
    }

    // Replaces the scene repaint when the image lives on a hardware plane.
    std::function<void()> onFrameChanged;

    // Whether the cursor would be drawn if it were not on a hardware plane.
    bool wantsDisplay() const { return m_wantsDisplay && !m_frames.empty(); }

    Content content() const override
    {
        return m_frames.empty() ? Content::None : Content::Translucent;
    }

protected:
    void paint(render::Renderer &renderer, const base::Rect &geometry) const override
    {
        if (m_frames.empty())
            return;
        const std::string key = "cursor:" + m_theme + ":" + resourceName(m_shape) + ":"
                + std::to_string(m_size * m_density);
        const auto &textures = renderer.cachedTextureSet(key, [&] {
            render::Renderer::TextureSet result;
            for (const auto &frame : m_frames) {
                const auto image = frame.image.converted(ui::Bitmap::Format::Rgba8888Premultiplied);
                result.push_back(renderer.createTexture(image.size(),
                                                        image.constBits(), int(image.stride()),
                                                        render::PixelFormat::Rgba8888));
            }
            return result;
        });
        if (m_frame < textures.size() && textures[m_frame])
            renderer.drawTextureF(textures[m_frame].get(),
                                  base::RectF(geometry).translated(m_fractionalOffset));
    }

private:
    void startTimer() { m_timer.start(std::chrono::milliseconds(m_frames[m_frame].delayMs)); }

    // A theme is read from disk once per shape and pixel size, as
    // wm::CursorLoader keeps image_cursors_ until the theme or size changes.
    std::vector<Frame> frames(input::CursorShape shape)
    {
        const int pixels = m_size * m_density;
        auto &loaded = m_loaded[{shape, pixels}];
        if (loaded.empty())
            loaded = load(shape, pixels, m_theme);
        return loaded;
    }

    double m_visualScale = 1;
    int m_size;
    std::string m_theme = "default";
    std::map<std::pair<input::CursorShape, int>, std::vector<Frame>> m_loaded;
    int m_density = 1;
    base::PointF m_fractionalOffset;
    input::CursorShape m_shape = input::CursorShape::Hidden;
    std::vector<Frame> m_frames;
    size_t m_frame = 0;
    bool m_wantsDisplay = false;
    base::Timer m_timer;
};

Cursor::Cursor(input::Seat &seat, scene::Node &container, int size)
    : m_seat(seat)
    , m_container(container)
    , m_node(static_cast<ImageNode *>(container.addChild(std::make_unique<ImageNode>(size))))
{
    m_locatorTimer.setInterval(std::chrono::milliseconds(16));
    m_locatorTimer.onTimeout = [this] { animateLocator(); };
    m_seat.addObserver(this);
    m_node->onFrameChanged = [this] {
        if (!m_hardware || !pushToSink())
            updatePresentation();
    };
    onCursorChanged();
}

Cursor::~Cursor()
{
    m_locatorTimer.stop();
    m_seat.removeObserver(this);
    m_container.removeChild(m_node);
}

base::Point Cursor::deviceHotspot() const
{
    const Frame *frame = m_node->currentFrame();
    if (!frame)
        return {};
    const double ratio = m_scale / m_node->density();
    return base::Point(int(std::floor(frame->hotspot.x() * ratio)),
                       int(std::floor(frame->hotspot.y() * ratio)));
}

void Cursor::setHeld(bool held)
{
    if (m_held == held)
        return;
    m_held = held;
    if (held) resetLocator();
    // The sink learns the position before the image, which it then puts on the
    // output under the pointer and nowhere else first.
    if (!held && m_sink)
        m_sink->moveCursor(m_seat.cursorPosition());
    updatePresentation();
    onCursorPositionChanged();
}

base::RectF Cursor::captureBounds() const
{
    if (!m_node->wantsDisplay())
        return {};
    const auto *frame = m_node->currentFrame();
    return base::RectF(m_seat.cursorPosition() - base::PointF(frame->hotspot) * (m_locatorScale / m_node->density()),
                       base::SizeF(frame->image.size()) * (m_locatorScale / m_node->density()));
}

void Cursor::paintForCapture(render::Renderer &renderer) const
{
    if (!m_node->wantsDisplay())
        return;
    const auto *frame = m_node->currentFrame();
    const auto image = frame->image.converted(ui::Bitmap::Format::Rgba8888Premultiplied);
    auto texture = renderer.createTexture(image.size(), image.constBits(),
                                          int(image.stride()), render::PixelFormat::Rgba8888);
    if (texture)
        renderer.drawTextureF(
                texture.get(),
                base::RectF(m_seat.cursorPosition() - base::PointF(frame->hotspot) * (m_locatorScale / m_node->density()),
                            base::SizeF(image.size()) * (m_locatorScale / m_node->density())));
}

void Cursor::setScale(double scale)
{
    if (m_scale == scale)
        return;
    m_scale = scale;
    m_node->setDensity(int(std::ceil(scale)));
    onCursorChanged();
}

void Cursor::setTheme(const std::string &theme, int size)
{
    // Preferences reloads on any config write; reprogramming the plane blinks the cursor.
    if (m_node->setTheme(validThemeName(theme) ? theme : "default", std::clamp(size, 16, 64)))
        onCursorChanged();
}

void Cursor::setSink(render::CursorSink *sink)
{
    if (m_sink == sink)
        return;
    if (m_sink && m_hardware)
        m_sink->hideCursor();
    m_sink = sink;
    m_hardware = false;
    updatePresentation();
    onCursorPositionChanged();
}

bool Cursor::pushToSink()
{
    const Frame *frame = m_node->currentFrame();
    if (!m_sink || !frame)
        return false;

    // A fractional scale gets the plane too, as ash scales the next larger
    // artwork down (wm::ScaleAndRotateCursorBitmapAndHotpoint, floored). The
    // hotspot then lands within half a device pixel, and the cursor moves on
    // the plane instead of costing a composited frame per motion.
    ui::Bitmap image = frame->image.converted(ui::Bitmap::Format::Argb32Premultiplied);
    const double ratio = m_scale / m_node->density();
    const base::Size deviceSize(int(std::floor(image.width() * ratio)),
                                int(std::floor(image.height() * ratio)));
    if (deviceSize.isEmpty())
        return false;
    if (deviceSize != image.size())
        image = image.scaled(deviceSize);
    render::CursorImage cursorImage;
    cursorImage.pixels = image.constBits();
    cursorImage.size = image.size();
    cursorImage.stride = static_cast<int>(image.stride());
    cursorImage.hotspot = deviceHotspot();
    return m_sink->setCursorImage(cursorImage);
}

// Decides between the cursor plane and the scene, and keeps exactly one of them
// showing the cursor.
void Cursor::updatePresentation()
{
    if (m_held) {
        m_hardware = false;
        m_node->setVisible(false);
        return;
    }
    // A client cursor surface composites through the seat's container, so the
    // plane has to give the cursor back rather than draw a second one.
    const bool wantsHardware = m_locatorScale == 1 && m_sink && m_node->wantsDisplay();
    m_hardware = wantsHardware && pushToSink();

    if (m_sink && !m_hardware)
        m_sink->hideCursor();
    m_node->setVisible(m_node->wantsDisplay() && !m_hardware);
}

void Cursor::refresh()
{
    // Drop the plane first: pushToSink() is skipped while the image is
    // unchanged, and the point here is to reprogram it regardless.
    if (m_sink && !m_held)
        m_sink->hideCursor();
    m_hardware = false;
    onCursorChanged();
}

void Cursor::onCursorChanged()
{
    const auto override = m_seat.cursorOverride();
    const auto client = m_seat.clientCursorShape();
    // A touch hides every cursor, the client's own surface too, until the
    // mouse moves.
    const bool touched = m_seat.cursorHiddenByTouch();
    // A client-owned bitmap has no theme shape. During locating, use the
    // system pointer as a temporary locator, then restore the client image.
    // An explicit hidden cursor is never revealed by the locator.
    if (touched) resetLocator();
    const bool locatingClient = m_locatorScale > 1 && m_seat.hasClientCursor();
    const bool visible = !touched && (override || client || locatingClient || !m_seat.hasClientCursorRequest());
    if (auto *container = m_seat.cursorContainer())
        container->setVisible(!override && !touched && !locatingClient);
    m_node->setVisualScale(m_locatorScale);
    m_node->setShape(override.value_or(client.value_or(locatingClient ? input::CursorShape::Default : m_seat.compositorCursorShape())), visible);
    updatePresentation();
    onCursorPositionChanged();
}

void Cursor::setShakeToLocate(bool enabled)
{
    if (m_shakeEnabled == enabled) return;
    m_shakeEnabled = enabled;
    if (!enabled) { resetLocator(); onCursorChanged(); }
}

void Cursor::resetLocator()
{
    m_shake.reset(); m_shakeAnimation.reset(); m_locatorTimer.stop();
    m_locatorScale = 1;
    m_node->setVisualScale(1);
}

void Cursor::animateLocator()
{
    if (m_held || m_seat.cursorHiddenByTouch()) { resetLocator(); onCursorChanged(); return; }
    const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    m_locatorScale = m_shakeAnimation.advance(now);
    // Do not change input or the seat override: only image presentation.
    onCursorChanged();
    if (!m_shakeAnimation.active()) m_locatorTimer.stop();
    if (auto *compositor = m_seat.compositor()) compositor->scheduleRepaint();
}

void Cursor::onCursorPositionChanged()
{
    if (m_shakeEnabled && !m_held && !m_seat.cursorHiddenByTouch() && m_seat.pressedButtons() == input::NoButton) {
        const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        if (m_shake.move(m_seat.cursorPosition(), now)) {
            m_shakeAnimation.trigger(now);
            if (!m_locatorTimer.isActive()) m_locatorTimer.start();
        }
    } else m_shake.reset();
    if (m_hardware) {
        // The whole point of the plane: motion costs one ioctl, not a frame.
        // The sink maps the logical position onto each output itself.
        m_sink->moveCursor(m_seat.cursorPosition());
        return;
    }
    m_node->setPointerPosition(m_seat.cursorPosition());
}

} // namespace Lingmo::Cursor
