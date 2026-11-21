// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell/shell_window.h"

#include <algorithm>
#include <cmath>

#include <xdg-shell-server-protocol.h>

#include "compositor/buffer.h"
#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "render/renderer.h"
#include "shell/shell.h"
#include "shell/shell_host.h"
#include "shell/shell_input.h"
#include "shell/window_renderer.h"
#include "toolkit/toolkit_host.h"
#include "ui/embedded_window.h"
#include "ui/view.h"
#include "wayland/protocols/xdg_positioner.h"
#include "wm/shell_surface.h"
#include "wm/window.h"

namespace Lingmo::Shell {

// The contents of a shell window's surface: a texture the toolkit rendered
// into, sampled where it is, as a DMA-BUF client buffer is. It holds no
// pixels of its own and owes no release to anybody.
class PanelBuffer final : public compositor::Buffer
{
public:
    explicit PanelBuffer(const base::Size &size)
        : m_size(size)
    {
    }

    base::Size size() const override { return m_size; }
    render::PixelFormat format() const override { return render::PixelFormat::Rgba8888; }
    const uint8_t *pixels() const override { return nullptr; }
    int stride() const override { return 0; }
    std::shared_ptr<render::Texture> texture() const override { return m_texture; }

    void setTexture(std::shared_ptr<render::Texture> texture) { m_texture = std::move(texture); }

private:
    const base::Size m_size;
    std::shared_ptr<render::Texture> m_texture;
};

// A picture in memory, as a client's shm buffer would hold it: the cursor's.
class BitmapBuffer final : public compositor::Buffer
{
public:
    explicit BitmapBuffer(ui::Bitmap bitmap)
        : m_bitmap(bitmap.converted(ui::Bitmap::Format::Argb32Premultiplied))
    {
    }

    base::Size size() const override { return m_bitmap.size(); }
    render::PixelFormat format() const override { return render::PixelFormat::Bgra8888; }
    const uint8_t *pixels() const override { return m_bitmap.constBits(); }
    int stride() const override { return int(m_bitmap.stride()); }

private:
    const ui::Bitmap m_bitmap;
};

namespace {

int roundEdge(double value)
{
    return int(std::lround(value));
}

base::Rect toRect(const ui::RectF &rect)
{
    const int left = roundEdge(rect.x);
    const int top = roundEdge(rect.y);
    return base::Rect(left, top, std::max(1, roundEdge(rect.x + rect.width) - left),
                      std::max(1, roundEdge(rect.y + rect.height) - top));
}

} // namespace

ShellWindow::ShellWindow(Shell *shell, ShellWindow *parent, ui::EmbeddedWindow *popup,
                         wm::WindowRole role)
    : m_shell(shell)
    , m_parent(parent)
{
    m_metadata.role = role;
    if (popup) {
        m_window = popup;
    } else {
        m_ownedWindow = std::make_unique<ui::EmbeddedWindow>();
        m_window = m_ownedWindow.get();
    }
    m_window->onRenderRequested = [this] { scheduleRender(); };
    m_window->onCursorShapeChanged = [this](ui::CursorShape) {
        m_shell->input()->cursorShapeChanged(this);
    };
    m_texture = std::make_unique<toolkit::PanelTexture>(m_window);

    m_surface = m_shell->compositor()->createSurface();
    m_shellSurface = std::make_unique<wm::ShellSurface>(m_shell->windowManager(), m_surface.get());
    m_shell->addWindow(this);
}

ShellWindow::ShellWindow(Shell *shell, wm::WindowRole role)
    : ShellWindow(shell, nullptr, nullptr, role)
{
    m_metadata.focusPolicy =
            role == wm::WindowRole::Osd ? wm::FocusPolicy::NoFocus : wm::FocusPolicy::Normal;
    m_shellSurface->setToplevel();
    const std::shared_ptr<bool> alive = m_alive;
    m_shellSurface->setCloseCallback([this, alive] {
        if (*alive && onCloseRequested)
            onCloseRequested();
    });
    applyMetadata();
    m_outputsChanged = m_shell->outputsChanged.connect([this] { updateScale(); });
    updateScale();
}

ShellWindow::ShellWindow(ShellWindow *parent, ui::EmbeddedWindow *popup)
    : ShellWindow(parent->m_shell, parent, popup, wm::WindowRole::Normal)
{
    m_grab = popup->isGrabbingPopup();
    parent->m_popups.push_back(this);
    const ui::SizeF size = popup->size();
    m_size = base::Size(std::max(1, roundEdge(size.width)), std::max(1, roundEdge(size.height)));
    const base::Rect bounds = placePopup(popup->popupPlacement(), m_size);
    m_shellSurface->setPopup(parent->m_shellSurface.get(), bounds);
    const std::shared_ptr<bool> alive = m_alive;
    m_shellSurface->setCloseCallback([this, alive] {
        // popup_done: the grab was broken, by a press outside the chain. The
        // toolkit closes the whole chain from its root, innermost first.
        if (!*alive)
            return;
        if (ShellWindow *root = this->root(); root && root->m_window)
            root->m_window->closePopups();
    });
    m_outputsChanged = m_shell->outputsChanged.connect([this] { updateScale(); });
    m_size = base::Size();
    setPopupBounds(bounds);
}

ShellWindow::~ShellWindow()
{
    *m_alive = false;
    for (ShellWindow *popup : std::vector<ShellWindow *>(m_popups)) {
        popup->hide();
        popup->m_parent = nullptr;
    }
    if (m_parent)
        m_parent->m_popups.erase(std::remove(m_parent->m_popups.begin(), m_parent->m_popups.end(),
                                             this),
                                 m_parent->m_popups.end());
    m_outputsChanged.reset();
    m_shell->renderer()->cancel(this);
    m_shell->input()->windowDestroying(this);
    m_cursorSurface.reset();
    m_cursorBuffer.reset();
    if (m_shell->host())
        m_shell->host()->windowDestroying(this);
    m_shell->removeWindow(this);
    if (m_observed) {
        m_observed->removeObserver(this);
        m_observed = nullptr;
    }
    wm::WindowManager *manager = m_shell->windowManager();
    manager->clearExclusiveZone(this);
    manager->clearShellMetadata(m_surface.get());
    // The window goes with the shell surface, then the surface tells the seat.
    m_shellSurface.reset();
    m_surface.reset();
    m_buffer.reset();
    m_texture.reset();
    if (m_window && !m_ownedWindow) {
        m_window->onRenderRequested = nullptr;
        m_window->onCursorShapeChanged = nullptr;
    }
    toolkit::destroyWindow(m_ownedWindow);
}

void ShellWindow::setContentView(std::unique_ptr<ui::View> view)
{
    m_window->setContentView(std::move(view));
}

wm::Window *ShellWindow::wmWindow() const
{
    return m_shellSurface ? m_shellSurface->window() : nullptr;
}

void ShellWindow::setFocusPolicy(wm::FocusPolicy policy)
{
    m_metadata.focusPolicy = policy;
    applyMetadata();
}

void ShellWindow::setPanelBehavior(wm::PanelBehavior behavior)
{
    m_metadata.panelBehavior = behavior;
    applyMetadata();
}

void ShellWindow::setOutput(const std::string &name)
{
    m_metadata.outputName = name;
    applyMetadata();
    updateScale();
}

void ShellWindow::setParentWindow(uint64_t windowId)
{
    m_metadata.parentWindow = windowId;
    applyMetadata();
}

void ShellWindow::setProtectedFromControl(bool protectedFromControl)
{
    m_metadata.protectedFromControl = protectedFromControl;
    applyMetadata();
}

void ShellWindow::setDialog(bool dialog)
{
    if (dialog == m_dialog || isPopup())
        return;
    m_dialog = dialog;
    m_surface->setSkipTaskbar(dialog);
    updateSizeLimits();
    if (m_shown)
        commit();
}

void ShellWindow::updateSizeLimits()
{
    // Equal limits are what makes the window manager hold a window's size,
    // as xdg_toplevel's set_min_size and set_max_size do for a client.
    const base::Size limit = m_dialog ? m_size : base::Size();
    m_shellSurface->setMinimumSize(limit);
    m_shellSurface->setMaximumSize(limit);
}

void ShellWindow::setExclusiveZone(wm::ScreenEdge edge, int thickness, bool yieldsToMaximized)
{
    m_shell->windowManager()->setExclusiveZone(this, edge, thickness, m_metadata.outputName,
                                               yieldsToMaximized);
}

void ShellWindow::clearExclusiveZone()
{
    m_shell->windowManager()->clearExclusiveZone(this);
}

void ShellWindow::activate()
{
    wm::Window *window = wmWindow();
    if (!window || isPopup())
        return;
    wm::WindowManager *manager = m_shell->windowManager();
    if (window->role() == wm::WindowRole::Capture || window->role() == wm::WindowRole::Picker)
        manager->setKeyboardGrab(window);
    else
        manager->activation()->activate(window);
}

void ShellWindow::setPosition(const base::Point &position)
{
    if (isPopup())
        return;
    m_position = position;
    m_metadata.position = position;
    m_metadata.hasPosition = true;
    applyMetadata();
    updateScale();
}

void ShellWindow::setSize(const base::Size &size)
{
    // A popup's size is its toolkit window's, through place().
    if (size == m_size || isPopup() || !m_window)
        return;
    m_size = size;
    m_window->setSize({double(size.width()), double(size.height())});
    if (m_dialog)
        updateSizeLimits();
    if (m_shown)
        commit();
    scheduleRender();
}

void ShellWindow::setGeometry(const base::Rect &geometry)
{
    setPosition(geometry.topLeft());
    setSize(geometry.size());
}

bool ShellWindow::lastFrameOnGpu() const
{
    return m_texture && m_texture->lastFrameOnGpu();
}

base::Rect ShellWindow::drawnBounds() const
{
    if (const wm::Window *window = wmWindow())
        return window->bounds().translated(window->presentationOffset());
    return geometry();
}

void ShellWindow::setInputRegion(std::optional<base::Region> region)
{
    if (region)
        m_surface->setInputRegion(*region);
    else
        m_surface->resetInputRegion();
    if (m_shown)
        commit();
}

void ShellWindow::setCursor(std::optional<Cursor> cursor)
{
    if (cursor && !cursor->image.isNull()) {
        if (!m_cursorSurface) {
            m_cursorSurface = m_shell->compositor()->createSurface();
            // Off the scene until the pointer takes it, as a client's cursor.
            m_cursorSurface->reparentNode(nullptr);
        }
        const bool same = m_cursor && m_cursor->image == cursor->image && m_cursor->scale == cursor->scale;
        if (!same) {
            m_cursorBuffer = std::make_shared<BitmapBuffer>(cursor->image);
            m_cursorSurface->attach(m_cursorBuffer, {});
            m_cursorSurface->damageBuffer(cursor->image.rect());
            m_cursorSurface->setBufferScale(std::max(1, cursor->scale));
            m_cursorSurface->commit();
        }
    }
    m_cursor = std::move(cursor);
    m_shell->input()->cursorShapeChanged(this);
}

void ShellWindow::setBackgroundBlur(const base::Rect &rect, int radius,
                                    const render::GlassParams &glass, const ui::Path &shape)
{
    m_surface->setBackgroundBlur(rect, radius);
    m_surface->setGlass(glass);
    m_surface->setBackgroundBlurShape(shape);
    if (m_shown)
        commit();
}

void ShellWindow::show()
{
    if (m_shown || m_size.isEmpty() || !m_window)
        return;
    m_shown = true;
    if (m_grab)
        m_shellSurface->grab();
    commit();
    scheduleRender();
}

void ShellWindow::hide()
{
    if (!m_shown)
        return;
    m_shown = false;
    // The toolkit closes its popups, which the host then lets go of; any
    // left are unmapped with the window.
    if (!isPopup() && m_window)
        m_window->closePopups();
    for (ShellWindow *popup : std::vector<ShellWindow *>(m_popups))
        popup->hide();
    m_shell->renderer()->cancel(this);
    m_surface->attach(nullptr, {});
    m_surface->commit();
    observeWindow();
}

void ShellWindow::place()
{
    if (!isPopup() || !m_parent || !m_window || m_placing)
        return;
    const ui::SizeF size = m_window->size();
    const base::Size requested(std::max(1, roundEdge(size.width)),
                               std::max(1, roundEdge(size.height)));
    const base::Rect bounds = placePopup(m_window->popupPlacement(), requested);
    m_shellSurface->reposition(bounds);
    setPopupBounds(bounds);
}

void ShellWindow::setPopupBounds(const base::Rect &bounds)
{
    m_position = m_parent->m_shellSurface->absoluteOrigin() + bounds.topLeft();
    updateScale();
    // A size other than the popup's own resizes it, as the positioner's
    // resize adjustment allows.
    {
        m_placing = true;
        m_window->setPopupGeometry(
                ui::RectF {double(bounds.x()), double(bounds.y()), double(bounds.width()),
                           double(bounds.height())});
        m_placing = false;
    }
    if (bounds.size() != m_size) {
        m_size = bounds.size();
        if (m_shown)
            commit();
    }
    scheduleRender();
}

ShellWindow *ShellWindow::root()
{
    ShellWindow *window = this;
    while (window->m_parent)
        window = window->m_parent;
    return window;
}

void ShellWindow::release()
{
    // Out of reach of input first: the toolkit window may be half destroyed.
    m_shell->input()->windowDestroying(this);
    if (m_shell->host())
        m_shell->host()->windowDestroying(this);
    m_shell->removeWindow(this);
    m_shell->renderer()->cancel(this);
    if (m_window && !m_ownedWindow) {
        m_window->onRenderRequested = nullptr;
        m_window->onCursorShapeChanged = nullptr;
    }
    m_window = nullptr;
    hide();
}

base::Rect ShellWindow::popupBounds() const
{
    return m_shellSurface->popupBounds();
}

void ShellWindow::produceFrame(render::Renderer &renderer, uint64_t frameTimeNs)
{
    if (!m_shown || m_size.isEmpty())
        return;
    m_texture->setFrameTime(frameTimeNs);
    m_producing = true;
    if (m_window && m_shell->host())
        m_shell->host()->frameStarting(this);
    std::shared_ptr<render::Texture> texture = m_texture->sharedTexture(renderer, true);
    m_producing = false;
    if (!texture || !m_shown)
        return;
    if (!m_buffer || m_buffer->size() != texture->size()) {
        m_buffer = std::make_shared<PanelBuffer>(texture->size());
        m_buffer->setTexture(std::move(texture));
        m_surface->attach(m_buffer, {});
    } else {
        m_buffer->setTexture(std::move(texture));
        // The same buffer again: its texture changed where the damage says.
        m_surface->attach(m_buffer, {});
        for (const base::Rect &rect : m_texture->lastDamage())
            m_surface->damageBuffer(rect);
    }
    m_surface->setViewport(m_size);
    m_surface->commit();
    observeWindow();
    if (onFrameProduced)
        onFrameProduced();
}

void ShellWindow::setKeyboardFocus(bool focused)
{
    const bool changed = focused != m_keyboardFocus;
    m_keyboardFocus = focused;
    if (m_window)
        m_window->setKeyboardFocus(focused);
    if (changed && onKeyboardFocusChanged)
        onKeyboardFocusChanged(focused);
}

void ShellWindow::onWindowActivationChanged(wm::Window *window, bool active)
{
    if (window == m_observed && m_window)
        m_window->setActive(active);
}

void ShellWindow::onWindowDestroying(wm::Window *window)
{
    if (window != m_observed)
        return;
    window->removeObserver(this);
    m_observed = nullptr;
    if (m_window)
        m_window->setActive(false);
}

void ShellWindow::applyMetadata()
{
    if (!isPopup())
        m_shell->windowManager()->setShellMetadata(m_surface.get(), m_metadata);
}

void ShellWindow::updateScale()
{
    if (!m_window)
        return;
    // A popup takes the scale of the output it lands on, as a client's
    // popup is told its output's.
    const Shell::Output *output = m_metadata.outputName.empty() || isPopup()
            ? m_shell->outputAt(m_position)
            : m_shell->output(m_metadata.outputName);
    double scale = m_parent ? m_parent->m_scale : 1;
    if (output) {
        scale = output->scale;
        m_window->setScreenSize({double(output->bounds.width()), double(output->bounds.height())});
    }
    if (scale != m_scale) {
        m_scale = scale;
        m_window->setScale(scale);
        if (m_shown)
            commit();
        scheduleRender();
    }
    for (ShellWindow *popup : m_popups)
        popup->updateScale();
}

void ShellWindow::commit()
{
    if (!m_window || m_producing)
        return;
    const base::Size pixels(m_window->pixelWidth(), m_window->pixelHeight());
    if (pixels.isEmpty())
        return;
    if (!m_buffer || m_buffer->size() != pixels) {
        // Empty until the next render pass, which comes before the next
        // frame is composed; until then the window keeps what it showed.
        m_buffer = std::make_shared<PanelBuffer>(pixels);
        m_surface->attach(m_buffer, {});
    } else if (!m_surface->buffer()) {
        m_surface->attach(m_buffer, {});
    }
    m_surface->setViewport(m_size);
    m_surface->commit();
    observeWindow();
}

void ShellWindow::scheduleRender()
{
    if (m_shown)
        m_shell->renderer()->scheduleRender(this);
}

void ShellWindow::observeWindow()
{
    wm::Window *window = wmWindow();
    if (window == m_observed)
        return;
    if (m_observed)
        m_observed->removeObserver(this);
    m_observed = window;
    if (m_observed)
        m_observed->addObserver(this);
    if (m_window)
        m_window->setActive(m_observed && m_observed->isActive());
    mappedChanged();
}

base::Rect ShellWindow::placePopup(const ui::PopupPlacement &placement,
                                   const base::Size &size) const
{
    wayland::WaylandPositioner positioner;
    positioner.setSize(size);
    positioner.setAnchorRect(toRect(placement.anchorRect));
    // ui::PopupAnchor follows xdg_positioner's anchor and gravity values.
    positioner.setAnchor(uint32_t(placement.anchor));
    positioner.setGravity(uint32_t(placement.gravity));
    uint32_t adjustment = XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_NONE;
    if (placement.slide) {
        adjustment |= XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_X
                | XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_SLIDE_Y;
    }
    if (placement.flipX)
        adjustment |= XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_X;
    if (placement.flipY)
        adjustment |= XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_FLIP_Y;
    if (placement.resize) {
        adjustment |= XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_X
                | XDG_POSITIONER_CONSTRAINT_ADJUSTMENT_RESIZE_Y;
    }
    positioner.setAdjustment(adjustment);
    positioner.setOffset(base::Point(roundEdge(placement.offset.x), roundEdge(placement.offset.y)));
    // As xdg_surface.get_popup does: the work area in the parent's window
    // geometry coordinates.
    base::Rect workArea = m_shell->windowManager()->workAreaFor(nullptr);
    workArea.translate(-m_parent->m_shellSurface->absoluteOrigin());
    return positioner.calculateBounds(workArea).toRect();
}

} // namespace Lingmo::Shell
