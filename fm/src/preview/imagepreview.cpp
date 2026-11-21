// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include "preview/imagepreview.h"

#include "base/url.h"
#include "ui/animated_image.h"
#include "ui/animation.h"
#include "ui/image_view.h"
#include "ui/window.h"

#include <algorithm>
#include <cmath>

namespace fm {

// The largest texture to ask for; a 50 MP photo stays inside common GPU limits.
static ui::SizeF cappedSize(double width, double height)
{
    const double scale = std::min(1.0, 8192 / std::max(width, height));
    return {std::round(width * scale), std::round(height * scale)};
}

// The flickable's content: the frame, centred while smaller than the view,
// and its two layers.
class ImageCanvas : public ui::View
{
public:
    explicit ImageCanvas(ImagePreview* preview)
        : m_preview(preview)
    {
        fit = addChild<ui::ImageView>();
        fit->setFillMode(ui::ImageView::FillMode::PreserveAspectFit);
        fit->setAutoTransform(true);
        fit->onStatusChanged = [this](ui::ImageView::Status) {
            if (m_preview->onStatusChanged)
                m_preview->onStatusChanged();
        };
        detail = addChild<ui::ImageView>();
        detail->setFillMode(ui::ImageView::FillMode::PreserveAspectFit);
        detail->setAutoTransform(true);
        detail->setVisible(false);
        detail->onStatusChanged = [this](ui::ImageView::Status) { requestLayout(); };
    }

    ui::SizeF frameSize() const
    {
        return {m_preview->naturalWidth() * m_preview->m_zoom, m_preview->naturalHeight() * m_preview->m_zoom};
    }

    ui::SizeF sizeHint() const override
    {
        const ui::SizeF frame = frameSize();
        const ui::RectF view = m_preview->geometry();
        return {std::max(view.width, frame.width), std::max(view.height, frame.height)};
    }

    bool acceptsPointer() const override { return true; }

    ui::ImageView* fit;
    ui::ImageView* detail;

    // GIF, APNG and animated WebP play as an AnimatedImage instead.
    void setAnimated(const std::string& path)
    {
        m_animatedPath = path;
        m_animated.reset();
        loadAnimated();
    }

    bool animatedLoading() const
    {
        return !m_animatedPath.empty() && (!m_animated || !m_animated->isLoaded()) && !m_animatedFailed;
    }
    bool animatedFailed() const { return m_animatedFailed; }

protected:
    void paint(ui::Painter& painter) override
    {
        if (m_animated && m_animated->isLoaded())
            m_animated->draw(painter, fit->geometry());
    }

    void layout() override
    {
        const ui::SizeF frame = frameSize();
        const ui::RectF rect {m_preview->snap(m_preview->frameX(m_preview->m_zoom)),
                              m_preview->snap(m_preview->frameY(m_preview->m_zoom)), frame.width, frame.height};
        fit->setGeometry(rect);
        detail->setGeometry(rect);
        loadAnimated();
        detail->setVisible(m_preview->zoomed() && detail->status() == ui::ImageView::Status::Ready);
    }

    void pointerPress(const ui::PointerEvent& event) override
    {
        if (event.clickCount == 2) {
            m_preview->toggleActualSize(viewPoint(event.position));
            return;
        }
        m_panning = m_preview->zoomed();
        if (!m_panning)
            return;
        if (m_preview->m_animation)
            m_preview->m_animation->stop();
        m_preview->m_anchored = false;
        m_start = mapToWindow(event.position);
        m_startPan = m_preview->scrollPosition();
        setCursorShape(ui::CursorShape::Grabbing);
    }

    void pointerMove(const ui::PointerEvent& event) override
    {
        if (!m_panning)
            return;
        const ui::PointF now = mapToWindow(event.position);
        m_preview->setPan(m_startPan.x - (now.x - m_start.x), m_startPan.y - (now.y - m_start.y));
    }

    void pointerRelease(const ui::PointerEvent&) override
    {
        m_panning = false;
        updateCursor();
    }

public:
    void updateCursor()
    {
        if (!m_panning)
            setCursorShape(m_preview->zoomed() ? ui::CursorShape::Grab : ui::CursorShape::Default);
    }

private:
    void loadAnimated()
    {
        if (m_animatedPath.empty() || m_animated || !window())
            return;
        m_animatedFailed = false;
        m_animated = std::make_unique<ui::AnimatedImage>(window());
        m_animated->onLoaded = [this](bool ok) {
            m_animatedFailed = !ok;
            if (ok)
                m_animated->setPlaying(true);
            requestPaint();
            if (m_preview->onStatusChanged)
                m_preview->onStatusChanged();
        };
        m_animated->onFrameChanged = [this] { requestPaint(); };
        m_animated->load(m_animatedPath);
    }

    std::string m_animatedPath;
    std::unique_ptr<ui::AnimatedImage> m_animated;
    bool m_animatedFailed = false;

    ui::PointF viewPoint(ui::PointF content) const
    {
        const ui::PointF pan = m_preview->scrollPosition();
        return {content.x - pan.x, content.y - pan.y};
    }

    ImagePreview* m_preview;
    bool m_panning = false;
    ui::PointF m_start;
    ui::PointF m_startPan;
};

ImagePreview::ImagePreview()
{
    setScrollDirections(true, true);
    setClipsChildren(true);
    setTracksHover(true);
    m_canvas = setContent<ImageCanvas>(this);

    // A vector image is drawn again at the zoom it comes to rest at.
    m_vectorTimer.setSingleShot(true);
    m_vectorTimer.setInterval(std::chrono::milliseconds(150));
    m_vectorTimer.onTimeout = [this] {
        if (zoomed()) {
            const ui::SizeF frame = m_canvas->frameSize();
            const ui::SizeF size = cappedSize(std::ceil(frame.width * m_ratio), std::ceil(frame.height * m_ratio));
            m_canvas->detail->setSourceSize({size.width / m_ratio, size.height / m_ratio});
        }
    };
}

ImagePreview::~ImagePreview() = default;

void ImagePreview::setImage(const PreviewFile& file, ui::SizeF prepared, double ratio)
{
    m_file = file;
    m_ratio = ratio > 0 ? ratio : 1;
    m_canvas->detail->setSource({});
    resetZoom();
    const auto path = base::url::toLocalFile(file.url);
    // The size first, so the new source is decoded once.
    ui::SizeF fitSize = prepared.width > 0 ? ui::SizeF {prepared.width / m_ratio, prepared.height / m_ratio}
                                           : geometry().size();
    m_canvas->fit->setSourceSize(fitSize);
    m_canvas->fit->setSource(file.animated ? std::string() : path.value_or(std::string()));
    m_canvas->setAnimated(file.animated ? path.value_or(std::string()) : std::string());
    m_canvas->requestLayout();
    requestLayout();
}

bool ImagePreview::isLoading() const
{
    if (m_file.animated)
        return m_canvas->animatedLoading();
    return m_canvas->fit->status() == ui::ImageView::Status::Loading;
}

bool ImagePreview::hasFailed() const
{
    if (m_file.animated)
        return m_canvas->animatedFailed();
    return m_canvas->fit->status() == ui::ImageView::Status::Error;
}

double ImagePreview::naturalWidth() const
{
    return m_file.width > 0 && m_file.height > 0 ? m_file.width / m_ratio : std::max(1.0, geometry().width);
}

double ImagePreview::naturalHeight() const
{
    return m_file.width > 0 && m_file.height > 0 ? m_file.height / m_ratio : std::max(1.0, geometry().height);
}

double ImagePreview::fitZoom() const
{
    const double width = geometry().width;
    const double height = geometry().height;
    if (width <= 0 || height <= 0)
        return 1;
    return std::min({1.0, width / naturalWidth(), height / naturalHeight()});
}

// Where the frame sits in the content at a zoom: centred while smaller than the view.
double ImagePreview::frameX(double zoom) const
{
    return std::max(0.0, (geometry().width - naturalWidth() * zoom) / 2);
}

double ImagePreview::frameY(double zoom) const
{
    return std::max(0.0, (geometry().height - naturalHeight() * zoom) / 2);
}

double ImagePreview::snap(double value) const
{
    return std::round(value * m_ratio) / m_ratio;
}

void ImagePreview::setPan(double x, double y)
{
    const double maxX = std::max(0.0, naturalWidth() * m_zoom - geometry().width);
    const double maxY = std::max(0.0, naturalHeight() * m_zoom - geometry().height);
    setScrollPosition({std::clamp(x, 0.0, maxX), std::clamp(y, 0.0, maxY)});
}

void ImagePreview::resetZoom()
{
    if (m_animation)
        m_animation->stop();
    m_anchored = false;
    m_fitted = true;
    m_zoom = fitZoom();
    m_canvas->requestLayout();
    requestLayout();
    setPan(0, 0);
    m_canvas->updateCursor();
}

void ImagePreview::setZoom(double zoom)
{
    m_zoom = zoom;
    // Laid out now, so the pan below is clamped against the new content.
    m_canvas->requestLayout();
    layout();
    if (m_anchored)
        setPan(frameX(m_zoom) + m_anchorImage.x * m_zoom - m_anchorView.x,
               frameY(m_zoom) + m_anchorImage.y * m_zoom - m_anchorView.y);
    else
        setPan(scrollPosition().x, scrollPosition().y);
    if (zoomed())
        startDetail();
    if (m_file.vector)
        m_vectorTimer.start();
    m_canvas->updateCursor();
}

// The full-resolution decode, the first time the image is zoomed past fit.
void ImagePreview::startDetail()
{
    if (!m_canvas->detail->source().empty() || m_file.animated)
        return;
    if (!m_file.vector && m_file.width > 0 && m_file.height > 0) {
        const ui::SizeF size = cappedSize(m_file.width, m_file.height);
        m_canvas->detail->setSourceSize({size.width / m_ratio, size.height / m_ratio});
        m_canvas->detail->setMipmap(true);
    } else if (!m_file.vector) {
        return;
    }
    m_canvas->detail->setSource(base::url::toLocalFile(m_file.url).value_or(std::string()));
}

void ImagePreview::zoomTo(double target, ui::PointF at, bool animate)
{
    target = std::clamp(target, fitZoom(), maxZoom());
    const ui::PointF pan = scrollPosition();
    m_anchorImage = {std::clamp((at.x + pan.x - frameX(m_zoom)) / m_zoom, 0.0, naturalWidth()),
                     std::clamp((at.y + pan.y - frameY(m_zoom)) / m_zoom, 0.0, naturalHeight())};
    m_anchorView = at;
    m_fitted = target <= fitZoom() + 0.0001;
    if (m_animation)
        m_animation->stop();
    m_anchored = true;
    if (animate && std::abs(target - m_zoom) > 0.0001 && window()) {
        if (!m_animation) {
            m_animation = std::make_unique<ui::Animation>(window());
            m_animation->setDuration(std::chrono::milliseconds(200));
            m_animation->setEasing(ui::Animation::Easing::OutCubic);
            m_animation->onUpdate = [this](double value) {
                setZoom(m_animationFrom + (m_animationTo - m_animationFrom) * value);
            };
        }
        m_animationFrom = m_zoom;
        m_animationTo = target;
        m_animation->start();
    } else {
        setZoom(target);
    }
}

// Steps compound on an animation's target, so quick wheel clicks add up.
void ImagePreview::zoomBy(double factor, ui::PointF at, bool animate)
{
    const double base = m_animation && m_animation->isRunning() ? m_animationTo : m_zoom;
    zoomTo(base * factor, at, animate);
}

void ImagePreview::toggleActualSize(ui::PointF at)
{
    if (zoomed())
        zoomTo(fitZoom(), at, true);
    else
        zoomTo(fitZoom() < 1 ? 1 : 2, at, true);
}

void ImagePreview::layout()
{
    const ui::SizeF size = geometry().size();
    if (size != m_lastSize) {
        m_lastSize = size;
        if (m_fitted) {
            m_zoom = fitZoom();
            m_canvas->requestLayout();
        }
    }
    ScrollView::layout();
}

bool ImagePreview::scroll(const ui::ScrollEvent& event)
{
    if (!(event.modifiers & ui::ControlModifier)) {
        // A pan ends a running zoom, or its anchor would pull the view back.
        if (m_animation && m_animation->isRunning()) {
            m_animation->stop();
            setZoom(m_animationTo);
        }
        m_anchored = false;
        return ScrollView::scroll(event);
    }
    if (event.value120Y != 0)
        zoomBy(std::pow(1.25, -event.value120Y / 120.0), m_pointer, true);
    else if (event.deltaY != 0)
        zoomBy(std::pow(1.006, -event.deltaY), m_pointer, false);
    return true;
}

bool ImagePreview::gesture(const ui::GestureEvent& event)
{
    if (event.type == ui::GestureEvent::Type::PinchBegin) {
        m_pinchScale = 1;
        return true;
    }
    if (event.type == ui::GestureEvent::Type::PinchUpdate) {
        const double delta = event.scale / m_pinchScale;
        m_pinchScale = event.scale;
        zoomTo(m_zoom * delta, m_pointer, false);
        return true;
    }
    return event.type == ui::GestureEvent::Type::PinchEnd;
}

void ImagePreview::hoverMove(const ui::PointerEvent& event)
{
    m_pointer = event.position;
}

} // namespace fm
