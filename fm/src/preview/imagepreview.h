// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "core/common.h"
#include "preview/previewcontroller.h"

#include "base/timer.h"
#include "ui/scroll_view.h"

#include <functional>
#include <memory>

namespace Lingmo::ui {
class Animation;
class ImageView;
}

namespace fm {

class ImageCanvas;

// An image fitted to the view, never enlarged past its
// own pixels until the user zooms. Zoom 1 is one image pixel per device pixel.
class ImagePreview : public ui::ScrollView
{
public:
    ImagePreview();
    ~ImagePreview() override;

    // The file, the fitted decode size the window preloaded, and the ratio
    // the window shows it at.
    void setImage(const PreviewFile& file, ui::SizeF prepared, double ratio);
    bool isLoading() const;
    bool hasFailed() const;

    void zoomBy(double factor, ui::PointF at, bool animate);
    void zoomTo(double target, ui::PointF at, bool animate);
    void zoomToFit() { zoomTo(fitZoom(), center(), true); }
    void zoomToActualSize() { zoomTo(1, center(), true); }
    ui::PointF center() const { return {geometry().width / 2, geometry().height / 2}; }

    std::function<void()> onStatusChanged;

protected:
    void layout() override;
    bool scroll(const ui::ScrollEvent& event) override;
    bool gesture(const ui::GestureEvent& event) override;
    void hoverMove(const ui::PointerEvent& event) override;

private:
    friend class ImageCanvas;

    double naturalWidth() const;
    double naturalHeight() const;
    double fitZoom() const;
    double maxZoom() const { return std::max(fitZoom(), 8.0); }
    bool zoomed() const { return m_zoom > fitZoom() + 0.0001; }
    double frameX(double zoom) const;
    double frameY(double zoom) const;
    double snap(double value) const;
    void setZoom(double zoom);
    void setPan(double x, double y);
    void resetZoom();
    void toggleActualSize(ui::PointF at);
    void startDetail();

    ImageCanvas* m_canvas;
    PreviewFile m_file;
    double m_ratio = 1;
    double m_zoom = 1;
    bool m_fitted = true;
    bool m_anchored = false;
    ui::PointF m_anchorView;
    ui::PointF m_anchorImage;
    ui::PointF m_pointer;
    double m_animationFrom = 1;
    double m_animationTo = 1;
    std::unique_ptr<ui::Animation> m_animation;
    base::Timer m_vectorTimer;
    ui::SizeF m_lastSize;
    double m_pinchScale = 1;
};

} // namespace fm
