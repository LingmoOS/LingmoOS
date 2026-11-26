// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "shell/stage/stage_window.h"

#include <algorithm>
#include <chrono>

#include "shell/shell_window.h"
#include "shell/common/task_manager.h"
#include "ui/button.h"
#include "ui/animation.h"
#include "ui/bitmap_painter.h"
#include "ui/image_view.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/painter.h"
#include "ui/scroll_view.h"
#include "ui/theme.h"
#include "ui/view.h"
#include "wm/stage/stage_manager.h"
#include "wm/window.h"
#include "wm/window_manager.h"

namespace Lingmo::Shell {

namespace {

constexpr int kWidth = 190;
constexpr int kCardHeight = 112;
constexpr int kLabelHeight = 20;   // app name label below the card
constexpr int kGap = 16;           // gap between card+label rows
constexpr int kMargin = 8;
constexpr int kEdgeInset = 10;

class Card final : public ui::View
{
public:
    Card()
    {
        setLayered(true);
        m_face = addChild<ui::View>();
        m_image = m_face->addChild<ui::ImageView>();
        m_image->setFillMode(ui::ImageView::FillMode::PreserveAspectCrop);
        m_image->setRendering(ui::ImageView::Rendering::MaskedLayer);
        m_image->setCornerRadius(8);
        m_title = m_face->addChild<ui::Label>();
        m_title->setVerticallyCentered(true);
        m_title->setElide(ui::Label::Elide::Right);
        m_title->setColor(ui::Color {255, 255, 255, 255});
        m_title->setVisible(false);
        m_icon = m_face->addChild<ui::ThemeIcon>("application-x-executable", 26);
        m_placeholder = m_face->addChild<ui::ThemeIcon>("application-x-executable", 44);
        m_placeholder->setOpacity(.76);
        m_raster = addChild<ui::ImageView>();
        m_raster->setFillMode(ui::ImageView::FillMode::Stretch);
        m_raster->setVisible(false);
        setTracksHover(true);
    }

    std::function<void()> onClicked;
    std::function<void()> onDragOut;

    bool acceptsPointer() const override { return true; }

    bool setGroup(wm::StageManager::Id group)
    {
        if (m_group == group)
            return false;
        m_group = group;
        m_thumbnailGroup = 0;
        m_thumbnailRevision = 0;
        m_thumbnailPending = false;
        if (m_thumbnailAnimation)
            m_thumbnailAnimation->stop();
        m_raster->setOpacity(1);
        m_raster->setTransform({{}, 1});
        m_image->setSource({});
        m_placeholder->setVisible(true);
        invalidateFace();
        return true;
    }

    void setThumbnail(wm::StageManager::Id group, uint64_t revision, const ui::Bitmap &bitmap)
    {
        if (bitmap.isNull()
            || (m_thumbnailGroup == group && m_thumbnailRevision == revision))
            return;
        m_thumbnailGroup = group;
        m_thumbnailRevision = revision;
        m_image->setPixels(bitmap.width(), bitmap.height(),
                           reinterpret_cast<const uint32_t *>(bitmap.constBits()),
                           bitmap.stride(), true);
        m_placeholder->setVisible(false);
        m_thumbnailPending = true;
        invalidateFace();
        requestPaint();
    }

    void clearThumbnail()
    {
        m_image->setSource({});
        m_thumbnailGroup = 0;
        m_thumbnailRevision = 0;
        m_placeholder->setVisible(true);
        m_thumbnailPending = false;
        if (m_thumbnailAnimation)
            m_thumbnailAnimation->stop();
        invalidateFace();
    }

    void animateIn()
    {
        m_entryProgress = 0;
        applyTransform();
        setOpacity(0);
        if (!window()) {
            m_entryProgress = 1;
            applyTransform();
            setOpacity(1);
            return;
        }
        if (!m_entryAnimation)
            m_entryAnimation = std::make_unique<ui::Animation>(window());
        m_entryAnimation->stop();
        m_entryAnimation->setDuration(std::chrono::milliseconds(240));
        m_entryAnimation->setEasing(ui::Animation::Easing::OutCubic);
        m_entryAnimation->onUpdate = [this](double progress) {
            m_entryProgress = progress;
            setOpacity(progress);
            applyTransform();
        };
        m_entryAnimation->onFinished = [this] {
            m_entryProgress = 1;
            setOpacity(1);
            applyTransform();
        };
        m_entryAnimation->start();
    }

    void setSelected(bool selected)
    {
        if (selected == m_selected)
            return;
        m_selected = selected;
        requestPaint();
    }

    void setTitle(std::string title)
    {
        if (title == m_title->text())
            return;
        m_title->setText(std::move(title));
        invalidateFace();
    }

    // Expose the stored title text so the View can feed it to the label below.
    const std::string &titleText() const { return m_title->text(); }

    void setApplicationIcon(std::string icon)
    {
        icon = icon.empty() ? "application-x-executable" : std::move(icon);
        if (icon == m_icon->iconName() && icon == m_placeholder->iconName())
            return;
        m_icon->setIconName(std::move(icon));
        m_placeholder->setIconName(m_icon->iconName());
        invalidateFace();
    }

    void setStackCount(size_t count)
    {
        m_stackCount = std::min<size_t>(count, 3);
        requestPaint();
    }

    // Perspective parameters: dy_norm is the signed normalised distance of
    // this card's centre from the strip's vertical centre (-1=top, +1=bottom).
    // The card rasterises with a right-side trapezoid warp so the right edge
    // appears to converge toward the vanishing point at (0, stripCenterY).
    void setPerspectiveFactor(double dyNorm)
    {
        if (std::abs(dyNorm - m_dyNorm) < 0.001)
            return;
        m_dyNorm = dyNorm;
        invalidateFace();
    }

    void childrenLaidOut() override
    {
        if (!m_faceDirty || !window() || geometry().width <= 0 || geometry().height <= 0)
            return;
        m_faceDirty = false;
        const ui::Bitmap source = m_face->grab(window()->scale());
        if (source.isNull())
            return;
        ui::Bitmap tilted(source.size(), ui::Bitmap::Format::Argb32Premultiplied);
        if (tilted.isNull())
            return;

        const double scale = window()->scale();
        const double W = source.width();
        const double H = source.height();
        const double inset = 4 * scale;

        // Perspective approximation using N horizontal slices, each given a
        // slightly different horizontal shear. This avoids needing a
        // non-affine API: each slice is a thin affine-sheared strip.
        //
        // m_dyNorm: signed normalised distance of card centre from strip
        // centre (-1 = top of strip, +1 = bottom). At the centre the card
        // is rectangular. The right edge leans based on position.
        //
        // For a card above the strip centre (dyNorm < 0):
        //   right-top corner leans outward (+x), right-bottom inward.
        // For a card below the strip centre (dyNorm > 0): reversed.
        //
        // kMaxShrink: max physical pixel shift of the right edge extreme.
        constexpr double kMaxShrink = 14.0;
        constexpr int kSlices = 16;

        ui::BitmapPainter painter(tilted);
        painter.setAntialiasing(true);

        const double sliceH = (H - 2 * inset) / kSlices;
        for (int s = 0; s < kSlices; ++s) {
            const double vTop = double(s)       / kSlices;  // 0..1 within content area
            const double vBot = double(s + 1)   / kSlices;
            const double vMid = (vTop + vBot) / 2.0;

            // dy for this slice within the card: maps 0..1 → -0.5..+0.5
            // then scaled by m_dyNorm to get the signed influence.
            const double localDy = vMid - 0.5;  // -0.5 at top, +0.5 at bottom

            // Combined: strip-level position (m_dyNorm) modulates how much
            // the within-card gradient tilts. At strip centre dyNorm≈0 the
            // card is flat; at extremes the right edge tilts more.
            const double shearFactor = m_dyNorm * localDy;  // positive → right leans right
            const double xShift = -kMaxShrink * scale * shearFactor;

            const double srcY0 = inset + vTop * (H - 2 * inset);
            const double srcY1 = inset + vBot * (H - 2 * inset);
            const double srcX0 = inset;
            const double srcX1 = W - inset;

            // Apply a horizontal shear: left column stays at srcX0,
            // right column shifts by xShift (varying per slice).
            // This is an affine shear: for a point (x, y) in the source,
            //   x' = x + xShift * (x - srcX0) / (srcX1 - srcX0)
            // which equals an AffineTransform with m21 = xShift / (srcX1-srcX0).
            const double shearPerUnit = xShift / (srcX1 - srcX0);
            // Translate so origin is at srcX0, apply shear, translate back.
            base::AffineTransform t;
            t.translate(srcX0, srcY0);
            // m11=1 m12=0 m21=shearPerUnit m22=1 (shear x by y is m21)
            // But we want shear x by x-offset, so it's a column-independent
            // horizontal stretch + shift. Simpler: just translate the source
            // rect to shift the right side.
            // We use a plain translate with scale:
            // For each slice, the left x stays, the right x shifts by xShift.
            // Map source rect to slightly widened/shifted dest rect.
            const double destX0 = srcX0;
            const double destX1 = srcX1 + xShift;
            const double scaleX = (destX1 - destX0) / (srcX1 - srcX0);
            // x' = destX0 + (x - srcX0) * scaleX = x * scaleX + (destX0 - srcX0*scaleX)
            t = base::AffineTransform(scaleX, 0, 0, 1,
                                      destX0 - srcX0 * scaleX,
                                      0);
            painter.setTransform(t);
            painter.drawBitmap({srcX0, srcY0, srcX1 - srcX0, srcY1 - srcY0}, source, true);
        }

        m_raster->setPixels(tilted.width(), tilted.height(),
                            reinterpret_cast<const uint32_t *>(tilted.constBits()),
                            tilted.stride(), true, scale);
        m_raster->setVisible(true);
        m_face->setVisible(false);
        if (std::exchange(m_thumbnailPending, false))
            animateThumbnail();
    }

protected:
    void hoverChanged() override
    {
        if (!m_hoverAnimation) {
            if (!window())
                return;
            m_hoverAnimation = std::make_unique<ui::Animation>(window());
            m_hoverAnimation->setDuration(std::chrono::milliseconds(140));
            m_hoverAnimation->setEasing(ui::Animation::Easing::OutCubic);
        }
        m_hoverAnimation->stop();
        const double from = m_hoverProgress;
        const double to = isHovered() ? 1.0 : 0.0;
        m_hoverAnimation->onUpdate = [this, from, to](double progress) {
            m_hoverProgress = from + (to - from) * progress;
            applyTransform();
        };
        m_hoverAnimation->start();
    }

    void pointerPress(const ui::PointerEvent &event) override
    {
        if (m_dragAnimation)
            m_dragAnimation->stop();
        m_press = event.position;
        m_pressWindow = mapToWindow(event.position);
        m_dragged = false;
        m_dragOffset = {};
        setOpacity(1);
    }

    void pointerMove(const ui::PointerEvent &event) override
    {
        if (m_press && !m_dragged) {
            const ui::PointF position = mapToWindow(event.position);
            const double dx = position.x - m_pressWindow.x;
            const double dy = position.y - m_pressWindow.y;
            if (std::hypot(dx, dy) >= 12)
                m_dragged = true;
        }
        if (m_dragged) {
            const ui::PointF position = mapToWindow(event.position);
            m_dragOffset = {position.x - m_pressWindow.x, position.y - m_pressWindow.y};
            applyTransform();
            setOpacity(0.78);
        }
    }

    void pointerRelease(const ui::PointerEvent &event) override
    {
        const bool dragged = m_dragged;
        const bool draggedOut = dragged && event.position.x > geometry().width;
        m_press.reset();
        m_dragged = false;
        if (dragged) {
            if (draggedOut && onDragOut)
                onDragOut();
            animateDragReset();
        } else if (event.button == ui::MouseButton::Left && onClicked) {
            m_dragOffset = {};
            applyTransform();
            onClicked();
        }
    }

    void layout() override
    {
        m_face->setGeometry(localRect());
        m_image->setGeometry({4, 4, geometry().width - 8, geometry().height - 8});
        m_title->setGeometry({0, 0, 0, 0});
        m_icon->setGeometry({10, geometry().height - 38, 30, 30});
        m_placeholder->setGeometry({(geometry().width - 44) / 2,
                                    (geometry().height - 44) / 2, 44, 44});
        m_raster->setGeometry(localRect());
    }

    void paint(ui::Painter &painter) override
    {
        const auto &theme = ui::Theme::current();
        const ui::Color fill = m_selected ? theme.highlight.withAlpha(0.24)
                                         : theme.secondBackground.withAlpha(0.30);
        for (size_t i = m_stackCount; i > 1; --i) {
            const double offset = double(i - 1) * 4;
            const ui::RectF back {offset, offset, geometry().width - offset,
                                  geometry().height - offset};
            painter.drawShadow(back, theme.mediumRadius, 8,
                               ui::Color {0, 0, 0, 90}, {0, 2});
            painter.fillRoundedRect(back, theme.mediumRadius,
                                    theme.secondBackground.withAlpha(0.68));
        }
        painter.drawShadow(localRect(), theme.mediumRadius, 12,
                           ui::Color {0, 0, 0, 100}, {0, 3});
        painter.fillRoundedRect(localRect(), theme.mediumRadius, fill);
        if (m_selected)
            painter.drawRoundedRect(localRect(), {theme.mediumRadius, 0, ui::Corners::Continuous},
                                    {}, 1, theme.highlight.withAlpha(0.65));
    }

private:
    void animateDragReset()
    {
        const ui::PointF from = m_dragOffset;
        if (!window() || (from.x == 0 && from.y == 0)) {
            m_dragOffset = {};
            applyTransform();
            setOpacity(1);
            return;
        }
        if (!m_dragAnimation)
            m_dragAnimation = std::make_unique<ui::Animation>(window());
        m_dragAnimation->stop();
        m_dragAnimation->setDuration(std::chrono::milliseconds(190));
        m_dragAnimation->setEasing(ui::Animation::Easing::OutCubic);
        m_dragAnimation->onUpdate = [this, from](double progress) {
            m_dragOffset = from * (1 - progress);
            applyTransform();
            setOpacity(.78 + .22 * progress);
        };
        m_dragAnimation->onFinished = [this] {
            m_dragOffset = {};
            applyTransform();
            setOpacity(1);
        };
        m_dragAnimation->start();
    }

    void animateThumbnail()
    {
        m_raster->setOpacity(0);
        m_raster->setTransform({{-12, 0}, .84});
        if (!window()) {
            m_raster->setOpacity(1);
            m_raster->setTransform({{}, 1});
            return;
        }
        if (!m_thumbnailAnimation)
            m_thumbnailAnimation = std::make_unique<ui::Animation>(window());
        m_thumbnailAnimation->stop();
        m_thumbnailAnimation->setDuration(std::chrono::milliseconds(220));
        m_thumbnailAnimation->setEasing(ui::Animation::Easing::OutCubic);
        m_thumbnailAnimation->onUpdate = [this](double progress) {
            m_raster->setOpacity(progress);
            m_raster->setTransform({{-12 * (1 - progress), 0}, .84 + .16 * progress});
        };
        m_thumbnailAnimation->onFinished = [this] {
            m_raster->setOpacity(1);
            m_raster->setTransform({{}, 1});
        };
        m_thumbnailAnimation->start();
    }

    void invalidateFace()
    {
        m_faceDirty = true;
        if (m_face) {
            m_face->setVisible(true);
            m_raster->setVisible(false);
            m_face->requestLayout();
        }
    }

    void applyTransform()
    {
        const double entryScale = .94 + .06 * m_entryProgress;
        const double scale = entryScale * (1.0 + .035 * m_hoverProgress);
        const double entryOffset = -18 * (1 - m_entryProgress);
        setTransform({{-geometry().width * (scale - 1) / 2 + entryOffset + m_dragOffset.x,
                       -geometry().height * (scale - 1) / 2 + m_dragOffset.y}, scale});
    }

    bool m_selected = false;
    bool m_faceDirty = true;
    bool m_thumbnailPending = false;
    wm::StageManager::Id m_group = 0;
    wm::StageManager::Id m_thumbnailGroup = 0;
    uint64_t m_thumbnailRevision = 0;
    ui::View *m_face = nullptr;
    ui::ImageView *m_image = nullptr;
    ui::Label *m_title = nullptr;
    ui::ThemeIcon *m_icon = nullptr;
    ui::ThemeIcon *m_placeholder = nullptr;
    ui::ImageView *m_raster = nullptr;
    size_t m_stackCount = 1;
    std::unique_ptr<ui::Animation> m_entryAnimation;
    std::unique_ptr<ui::Animation> m_thumbnailAnimation;
    std::unique_ptr<ui::Animation> m_hoverAnimation;
    std::unique_ptr<ui::Animation> m_dragAnimation;
    double m_entryProgress = 1;
    double m_hoverProgress = 0;
    double m_tilt = 0.0;
    double m_dyNorm = 0.0;  // signed normalised dist from strip centre (-1..1)
    std::optional<ui::PointF> m_press;
    ui::PointF m_pressWindow;
    ui::PointF m_dragOffset;
    bool m_dragged = false;
};

} // namespace

class StageWindow::View final : public ui::View
{
public:
    explicit View(StageWindow *owner) : m_owner(owner)
    {
        setName("stage-manager-strip");
        m_scroll = addChild<ui::ScrollView>();
        auto content = std::make_unique<ui::View>();
        m_content = content.get();
        m_scroll->setContentView(std::move(content));
    }

    void sync(const wm::StageManager::Snapshot &snapshot)
    {
        m_snapshot = snapshot;
        m_visibleGroups.clear();
        for (const auto &group : m_snapshot.groups) {
            const auto active = m_snapshot.active.find(group.output);
            if (active == m_snapshot.active.end() || active->second != group.id)
                m_visibleGroups.push_back(group);
        }
        std::stable_sort(m_visibleGroups.begin(), m_visibleGroups.end(),
                         [](const auto &left, const auto &right) {
                             if (left.lastUsed != right.lastUsed)
                                 return left.lastUsed > right.lastUsed;
                             return left.id < right.id;
                         });
        while (m_cards.size() < m_visibleGroups.size()) {
            auto *card = m_content->addChild<Card>();
            m_cards.push_back(card);
            // App name label: sits below each card in the content view,
            // outside the card itself so it doesn't disturb the thumbnail.
            auto *label = m_content->addChild<ui::Label>();
            label->setElide(ui::Label::Elide::Right);
            label->setVerticallyCentered(true);
            label->setColor(ui::Color {230, 230, 230, 255});
            label->setFontSize(12);
            m_labels.push_back(label);
        }
        for (size_t i = 0; i < m_cards.size(); ++i) {
            const bool visible = i < m_visibleGroups.size();
            const bool wasVisible = m_cards[i]->isVisible();
            m_cards[i]->setVisible(visible);
            m_labels[i]->setVisible(visible);
            if (!visible)
                continue;
            const auto &group = m_visibleGroups[i];
            const bool changedGroup = m_cards[i]->setGroup(group.id);
            if (!wasVisible || changedGroup)
                m_cards[i]->animateIn();
            (void)i; // stack position handled by setPerspectiveFactor in layout()
            std::string title = "Stage " + std::to_string(i + 1);
            std::string icon = "application-x-executable";
            if (!group.windows.empty()) {
                if (wm::Window *window = m_owner->m_shell->windowManager()->windowById(group.windows.front())) {
                    if (!window->title().empty())
                        title = window->title();
                    if (!window->applicationId().empty())
                        icon = window->applicationId();
                    if (const auto entry = m_owner->m_shell->tasks().entryForWindow(window->id())) {
                        if (!entry->name.empty())
                            title = entry->name;
                        if (!entry->icon.empty())
                            icon = entry->icon;
                    }
                }
            }
            m_cards[i]->setTitle(std::move(title));
            m_labels[i]->setText(m_cards[i]->titleText());
            m_cards[i]->setApplicationIcon(std::move(icon));
            m_cards[i]->setStackCount(group.windows.size());
            m_cards[i]->setSelected(false);
            m_cards[i]->onClicked = [this, id = group.id] {
                m_owner->m_shell->windowManager()->stageManager()->switchTo(id);
            };
            m_cards[i]->onDragOut = [this, id = group.id, groupIndex = i] {
                if (groupIndex < m_visibleGroups.size()
                    && !m_visibleGroups[groupIndex].windows.empty()) {
                    m_owner->m_shell->windowManager()->stageManager()->separateWindow(
                            m_visibleGroups[groupIndex].windows.front());
                }
            };
        }
        requestLayout();
        requestPaint();
    }

    void setThumbnail(wm::StageManager::Id group, uint64_t revision, const ui::Bitmap &bitmap)
    {
        const auto it = std::find_if(m_visibleGroups.begin(), m_visibleGroups.end(),
                                     [group](const auto &candidate) { return candidate.id == group; });
        if (it == m_visibleGroups.end())
            return;
        const size_t index = size_t(it - m_visibleGroups.begin());
        if (index < m_cards.size())
            m_cards[index]->setThumbnail(group, revision, bitmap);
    }

    size_t visibleGroupCount() const { return m_visibleGroups.size(); }

protected:
    void layout() override
    {
        const double width = geometry().width;
        const double visibleHeight = geometry().height - 2 * kMargin;
        m_scroll->setGeometry({kMargin, kMargin, width - 2 * kMargin, visibleHeight});
        const double rowHeight = kCardHeight + kLabelHeight + kGap;
        const size_t n = m_visibleGroups.size();
        m_content->setGeometry({0, 0, width - 2 * kMargin, double(n) * rowHeight});

        // Compute the strip's total content height and its vertical centre
        // in content coordinates, then map each card's centre to a normalised
        // distance [-1, +1] and pass it to the card for perspective warping.
        const double totalH = double(n) * rowHeight;
        const double stripCy = totalH / 2.0;

        for (size_t i = 0; i < m_cards.size(); ++i) {
            if (!m_cards[i]->isVisible())
                continue;
            const double y = double(i) * rowHeight;
            const double cardWidth = width - 4 * kMargin;
            m_cards[i]->setGeometry({0, y, cardWidth, kCardHeight});
            m_labels[i]->setGeometry({0, y + kCardHeight + 2, cardWidth, kLabelHeight});

            // dy_norm: signed normalised distance of this card's centre from
            // the strip centre.  Clamped so edge cards never go beyond ±1.
            const double cardCy = y + kCardHeight / 2.0;
            const double dyNorm = totalH > 0
                    ? std::clamp((cardCy - stripCy) / (totalH / 2.0), -1.0, 1.0)
                    : 0.0;
            m_cards[i]->setPerspectiveFactor(dyNorm);
        }
    }

private:
    StageWindow *m_owner;
    wm::StageManager::Snapshot m_snapshot;
    std::vector<wm::StageManager::Group> m_visibleGroups;
    ui::ScrollView *m_scroll = nullptr;
    ui::View *m_content = nullptr;
    std::vector<Card *> m_cards;
    std::vector<ui::Label *> m_labels;
};

StageWindow::StageWindow(Shell *shell, std::unique_ptr<ScreenGrabber> grabber)
    : m_shell(shell)
    , m_grabber(std::move(grabber))
{
    m_window = std::make_unique<ShellWindow>(shell, wm::WindowRole::Panel);
    m_window->setFocusPolicy(wm::FocusPolicy::NoFocus);
    m_window->setPanelBehavior(wm::PanelBehavior::StayAbove);
    auto view = std::make_unique<View>(this);
    m_view = view.get();
    m_window->setContentView(std::move(view));
    m_window->setSize({kWidth, kCardHeight + 2 * kMargin});
    auto *manager = m_shell->windowManager()->stageManager();
    m_subscriptions.push_back(manager->changed.connect([this](wm::StageManager::Snapshot snapshot) {
        sync(snapshot);
    }));
    m_subscriptions.push_back(m_shell->outputsChanged.connect([this] { place(); }));
    m_snapshot = manager->snapshot();
    manager->setEnabled(manager->preferenceEnabled());
    sync(m_snapshot);
    refreshThumbnails();
    place();
    updateVisibility();
}

StageWindow::~StageWindow()
{
    *m_alive = false;
    m_subscriptions.clear();
    if (m_visibilityAnimation)
        m_visibilityAnimation->stop();
    m_visibilityAnimation.reset();
    m_window.reset();
}

bool StageWindow::isShown() const
{
    return m_window->isShown();
}

void StageWindow::toggle()
{
    m_userVisible = !m_userVisible;
    updateVisibility();
}

void StageWindow::sync(const wm::StageManager::Snapshot &snapshot)
{
    m_snapshot = snapshot;
    if (m_view) {
        m_view->sync(snapshot);
        for (const auto &group : snapshot.groups) {
            const auto thumbnail = m_thumbnails.find(group.id);
            if (thumbnail != m_thumbnails.end()) {
                m_view->setThumbnail(group.id, thumbnail->second.revision,
                                     thumbnail->second.bitmap);
            }
        }
    }
    place();
    updateVisibility();
    refreshThumbnails();
}

void StageWindow::refreshThumbnails()
{
    if (!m_grabber || m_capturePending || !m_view)
        return;
    auto group = m_snapshot.groups.end();
    wm::Window *window = nullptr;
    for (auto it = m_snapshot.groups.begin(); it != m_snapshot.groups.end(); ++it) {
        if (it->windows.empty()
            || (m_captured.contains(it->id) && m_captured[it->id] == it->contentRevision))
            continue;
        wm::Window *candidate = m_shell->windowManager()->windowById(it->windows.front());
        if (candidate) {
            group = it;
            window = candidate;
            break;
        }
    }
    if (group == m_snapshot.groups.end())
        return;
    const auto id = group->id;
    const uint64_t revision = group->contentRevision;
    m_capturePending = true;
    m_grabber->grabWindow(window, false,
                          [this, alive = m_alive, id, revision](std::vector<ui::Bitmap> images) {
                              if (!*alive)
                                  return;
                              m_capturePending = false;
                              const auto current = std::find_if(
                                      m_snapshot.groups.begin(), m_snapshot.groups.end(),
                                      [id](const auto &item) { return item.id == id; });
                              if (current != m_snapshot.groups.end()
                                  && current->contentRevision == revision) {
                                  m_captured[id] = revision;
                                  if (!images.empty() && !images.front().isNull()) {
                                      if (images.front().width() > 320
                                          || images.front().height() > 180) {
                                          const base::Size size = ui::scaledSize(
                                                  images.front().size(), {320, 180},
                                                  ui::Bitmap::Scaling::Fit);
                                          images.front() = images.front().scaled(size, true);
                                      }
                                      m_thumbnails[id] = {revision, images.front()};
                                      m_view->setThumbnail(id, revision, images.front());
                                  }
                              }
                              refreshThumbnails();
                          }, true);
}

void StageWindow::place()
{
    const Shell::Output *output = m_shell->output({});
    if (!output)
        return;
    const base::Rect workArea = m_shell->workArea(*output);
    if (workArea.isEmpty())
        return;
    const int height = std::clamp(kCardHeight + kLabelHeight + 2 * kMargin,
                                  kCardHeight + kLabelHeight + 2 * kMargin,
                                  workArea.height() - 2 * kEdgeInset);
    m_window->setOutput(output->name);
    const int rowHeight = kCardHeight + kLabelHeight + kGap;
    const int stripHeight = std::min(workArea.height() - 2 * kEdgeInset,
                                     std::max(height, int(m_view->visibleGroupCount())
                                                              * rowHeight
                                                              + 2 * kMargin));
    m_window->setGeometry({workArea.left() + kEdgeInset, workArea.top() + kEdgeInset,
                           kWidth, stripHeight});
}

void StageWindow::updateVisibility()
{
    const bool shouldShow = m_userVisible && m_snapshot.enabled && !m_snapshot.blocked
            && m_view && m_view->visibleGroupCount() > 0;
    if (m_visibilityAnimation)
        m_visibilityAnimation->stop();

    double from = m_visibilityProgress;
    const double to = shouldShow ? 1.0 : 0.0;
    if (shouldShow && !m_window->isShown()) {
        from = 0;
        m_visibilityProgress = 0;
        m_view->setOpacity(0);
        m_view->setTransform({{-kWidth * .82, 0}, 1});
        m_window->show();
        m_view->setEnabled(true);
    } else if (shouldShow) {
        m_view->setEnabled(true);
    } else {
        m_view->setEnabled(false);
        if (!m_window->isShown()) {
            m_view->setOpacity(0);
            m_view->setTransform({{-kWidth * .82, 0}, 1});
            return;
        }
    }
    from = m_visibilityProgress;
    if (!shouldShow && from == 0) {
        m_window->hide();
        return;
    }
    if (from == to)
        return;

    if (!m_visibilityAnimation)
        m_visibilityAnimation = std::make_unique<ui::Animation>(m_view->window());
    m_visibilityAnimation->setDuration(std::chrono::milliseconds(150));
    m_visibilityAnimation->setEasing(ui::Animation::Easing::OutCubic);
    m_visibilityAnimation->onUpdate = [this, from, to](double progress) {
        m_visibilityProgress = from + (to - from) * progress;
        m_view->setOpacity(m_visibilityProgress);
        m_view->setTransform({{(m_visibilityProgress - 1) * kWidth * .82, 0}, 1});
    };
    m_visibilityAnimation->onFinished = [this, shouldShow] {
        m_visibilityProgress = shouldShow ? 1.0 : 0.0;
        m_view->setOpacity(m_visibilityProgress);
        m_view->setTransform({{(m_visibilityProgress - 1) * kWidth * .82, 0}, 1});
        if (!shouldShow && m_window->isShown())
            m_window->hide();
    };
    m_visibilityAnimation->start();
}

} // namespace Lingmo::Shell
