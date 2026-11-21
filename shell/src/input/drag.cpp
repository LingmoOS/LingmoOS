// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/{drag_drop_operation,data_device}.cc
//           chromium/src/ash/drag_drop/drag_drop_controller.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include "input/drag.h"

#include <algorithm>
#include <linux/input-event-codes.h>
#include <unistd.h>
#include <utility>

#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "input/hit_test.h"
#include "input/seat.h"
#include "scene/motion.h"
#include "scene/surface_node.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Input {

// ash's kCancelAnimationDuration.
static constexpr double kReturnDurationMs = 250;

static void setSubtreeAlpha(scene::Node *node, float alpha)
{
    if (auto *surface = dynamic_cast<scene::SurfaceNode *>(node))
        surface->setAlpha(alpha);
    for (scene::Node *child : node->children())
        setSubtreeAlpha(child, alpha);
}

DragOffer::DragOffer(std::shared_ptr<clipboard::DataSource> source, uint32_t sourceActions,
                     bool legacy)
    : m_source(std::move(source))
    , m_formats(m_source ? m_source->formats() : std::vector<std::string>())
    , m_sourceActions(sourceActions)
    , m_actions(legacy ? 1 : 0)
    , m_legacy(legacy)
{
    negotiate();
}

DragOffer::~DragOffer()
{
    if (m_state == State::Dropped && m_source) {
        if (m_legacy) {
            m_state = State::Finished;
            m_source->finished();
            m_source->invalidate();
        } else {
            m_state = State::Cancelled;
            m_source->cancel();
        }
    }
}

void DragOffer::accept(const char *mime)
{
    const auto alive = weak_from_this().lock();
    if (m_state != State::Hover && m_state != State::Dropped)
        return;
    m_mime = mime && std::find(m_formats.begin(), m_formats.end(), mime) != m_formats.end() ? mime
                                                                                            : "";
    if (m_state == State::Dropped && m_source && m_source->alive())
        m_source->target(m_mime.empty() ? nullptr : m_mime.c_str());
    feedbackChanged();
}

void DragOffer::setActions(uint32_t actions, uint32_t preferred)
{
    const auto alive = weak_from_this().lock();
    if (m_state != State::Hover && !(m_state == State::Dropped && m_action == 4))
        return;
    m_actions = actions;
    m_preferred = preferred;
    if (m_state == State::Dropped)
        m_modifier = 0;
    negotiate();
    if (m_state == State::Dropped && m_source && m_source->alive())
        m_source->action(m_action);
}

void DragOffer::setModifierAction(uint32_t action)
{
    const auto alive = weak_from_this().lock();
    if (m_state != State::Hover)
        return;
    m_modifier = action;
    negotiate();
}

void DragOffer::negotiate()
{
    const uint32_t available = m_sourceActions & m_actions;
    const uint32_t action = (available & m_modifier) ? m_modifier
            : (available & m_preferred)              ? m_preferred
                                                     : (available & (~available + 1));
    if (m_action != action) {
        m_action = action;
        actionChanged(action);
    }
    feedbackChanged();
}

bool DragOffer::canDrop() const
{
    return m_state == State::Hover
            && (!m_source || (m_source->alive() && !m_mime.empty() && m_action));
}

bool DragOffer::drop()
{
    const auto alive = weak_from_this().lock();
    if (!canDrop())
        return false;
    m_state = State::Dropped;
    if (m_source)
        m_source->dropPerformed();
    return true;
}

void DragOffer::receive(const std::string &mime, int fd)
{
    if ((m_state != State::Hover && m_state != State::Dropped) || !m_source || !m_source->alive()
        || std::find(m_formats.begin(), m_formats.end(), mime) == m_formats.end()) {
        close(fd);
        return;
    }
    m_source->send(mime, fd);
}

bool DragOffer::finish()
{
    const auto alive = weak_from_this().lock();
    if (m_state != State::Dropped || !m_source || m_mime.empty() || !m_action || m_action == 4)
        return false;
    m_state = State::Finished;
    // A source gone since the drop has nobody left to tell; the finish is
    // still a timely one.
    if (m_source->alive()) {
        m_source->finished();
        m_source->invalidate();
    }
    return true;
}

void DragOffer::invalidate()
{
    const auto alive = weak_from_this().lock();
    if (m_state == State::Cancelled || m_state == State::Finished)
        return;
    const bool dropped = m_state == State::Dropped;
    m_state = State::Cancelled;
    feedbackChanged();
    if (dropped && m_source)
        m_source->cancel();
}

DragController::DragController(Seat *seat)
    : m_seat(seat)
{
}

DragController::~DragController()
{
    m_returnsHome = false;
    cancel();
    releaseReturn();
}

void DragController::start(compositor::Surface *origin, compositor::Surface *icon,
                           std::shared_ptr<clipboard::DataSource> source, uint32_t actions,
                           ButtonFlags button)
{
    if (busy() || !origin || m_seat->wasShutdown())
        return;
    m_button = button;
    if (!begin(origin, icon, std::move(source), actions, m_seat->pointerLocation()))
        return;
    const auto generation = m_generation;
    m_seat->clearPointerFocus();
    if (generation != m_generation)
        return;
    started();
    if (generation != m_generation)
        return;
    m_seat->setCursorOverride(idleCursor());
    if (generation != m_generation)
        return;
    motion(Timestamp {}, m_location);
}

void DragController::startTouch(compositor::Surface *origin, compositor::Surface *icon,
                                std::shared_ptr<clipboard::DataSource> source, uint32_t actions,
                                int32_t touchId, const base::PointF &location)
{
    if (busy() || !origin || m_seat->wasShutdown())
        return;
    m_button = NoButton;
    m_touchId = touchId;
    if (!begin(origin, icon, std::move(source), actions, location))
        return;
    const auto generation = m_generation;
    m_seat->takeTouchPoint(touchId);
    if (generation != m_generation)
        return;
    started();
    if (generation != m_generation)
        return;
    m_seat->setCursorOverride(idleCursor());
    if (generation != m_generation)
        return;
    motion(Timestamp {}, m_location);
}

bool DragController::begin(compositor::Surface *origin, compositor::Surface *icon,
                           std::shared_ptr<clipboard::DataSource> source, uint32_t actions,
                           const base::PointF &location)
{
    releaseReturn();
    const auto generation = ++m_generation;
    m_seat->consumeDragSerials();
    m_origin = origin;
    m_icon = icon;
    m_source = std::move(source);
    m_actions = actions;
    m_location = location;
    m_home = location;
    origin->addSurfaceObserver(this);
    if (m_source) {
        m_source->markUsed();
        m_sourceConnection = m_source->invalidated.connect([this] {
            cancel();
        });
    }
    if (icon) {
        icon->setHasRole(true);
        icon->addSurfaceObserver(this);
        icon->reparentNode(m_seat->dragIconContainer());
        if (generation != m_generation)
            return false;
        updateIcon();
    }
    return true;
}

void DragController::startExternal(compositor::Surface *origin,
                                   std::shared_ptr<clipboard::DataSource> source, uint32_t actions)
{
    if (busy() || !origin || !source || m_seat->wasShutdown())
        return;
    ++m_generation;
    m_external = true;
    m_origin = origin;
    m_source = std::move(source);
    m_actions = actions;
    m_location = m_seat->pointerLocation();
    origin->addSurfaceObserver(this);
    m_source->markUsed();
    m_sourceConnection = m_source->invalidated.connect([this] {
        cancel();
    });
}

void DragController::setCursor(CursorShape shape)
{
    // The X11 client that owns an external drag draws its own cursor.
    if (!m_external)
        m_seat->setCursorOverride(shape);
}

std::shared_ptr<DragOffer> DragController::createOffer(bool legacy)
{
    if (!active() || !m_target)
        return nullptr;
    auto offer = std::make_shared<DragOffer>(m_source, m_actions, legacy);
    m_offers.push_back(offer);
    std::erase_if(m_offerConnections, [](const auto &connection) {
        return connection.first.expired();
    });
    m_offerConnections.emplace_back(offer, offer->feedbackChanged.connect([this, raw = offer.get()] {
        updateFeedback(raw);
    }));
    offer->setModifierAction(modifierAction());
    return offer;
}

void DragController::updateFeedback(DragOffer *offer)
{
    if (!active())
        return;
    if (offer->state() != DragOffer::State::Hover) {
        if (selectedOffer() != offer)
            return;
        m_selected.reset();
        for (auto it = m_offers.rbegin(); it != m_offers.rend(); ++it) {
            if (auto candidate = it->lock(); candidate && candidate->canDrop()) {
                m_selected = candidate;
                break;
            }
        }
    } else {
        DragOffer *selected = selectedOffer();
        if (!offer->canDrop() && selected && selected != offer && selected->canDrop())
            return;
        m_selected = offer->weak_from_this();
    }
    DragOffer *selected = selectedOffer();
    const uint32_t action = selected && selected->canDrop() ? selected->action() : 0;
    const auto generation = m_generation;
    const auto source = m_source;
    if (source) {
        const auto mime = selected ? selected->acceptedMime() : std::string();
        source->target(mime.empty() ? nullptr : mime.c_str());
        if (generation != m_generation || !active())
            return;
        source->action(action);
    }
    if (generation != m_generation || !active())
        return;
    notifyWindowTarget();
    setCursor(!m_source             ? CursorShape::Grabbing
                      : action == 1 ? CursorShape::Copy
                      : action == 2 ? CursorShape::Move
                      : action == 4 ? CursorShape::ContextMenu
                                    : idleCursor());
}

void DragController::leave()
{
    auto *target = std::exchange(m_target, nullptr);
    m_selected.reset();
    auto offers = std::exchange(m_offers, {});
    if (target && target != m_origin && target != m_icon)
        target->removeSurfaceObserver(this);
    for (auto &weak : offers) {
        if (auto offer = weak.lock())
            offer->invalidate();
    }
    const auto generation = m_generation;
    const auto source = m_source;
    if (target)
        left();
    if (generation == m_generation && source && source->alive()) {
        source->target(nullptr);
        if (generation == m_generation && source->alive())
            source->action(0);
    }
    if (generation == m_generation && active()) {
        notifyWindowTarget();
        setCursor(idleCursor());
    }
}

void DragController::motion(Timestamp timestamp, const base::PointF &location)
{
    if (!active())
        return;
    m_location = location;
    updateIcon();
    {
        const auto generation = m_generation;
        locationChanged(location);
        if (generation != m_generation || !active())
            return;
    }
    if (m_windowDelegate) {
        const auto generation = m_generation;
        m_windowDelegate->dragMoved(location);
        if (generation != m_generation || !active())
            return;
    }
    base::PointF local;
    auto *target = findTargetSurface(m_seat->compositor(), location, &local);
    if (target != m_target) {
        const auto generation = m_generation;
        const auto lifetime = target ? target->lifetimeToken() : nullptr;
        leave();
        if (generation != m_generation || !active() || m_target || !fuzzyEqual(m_location, location))
            return;
        target = lifetime ? *lifetime : nullptr;
        m_target = target;
        if (target) {
            target->addSurfaceObserver(this);
            entered(target, local);
        }
    }
    if (target && target == m_target && active())
        moved(timestamp, local);
}

void DragController::detach()
{
    ++m_generation;
    m_sourceConnection.reset();
    auto *origin = std::exchange(m_origin, nullptr);
    auto *icon = std::exchange(m_icon, nullptr);
    m_touchId.reset();
    if (origin)
        origin->removeSurfaceObserver(this);
    m_iconOpacity = 1.0f;
    m_plainIdleCursor = false;
    m_returnsHome = false;
    if (icon) {
        setSubtreeAlpha(icon->node(), 1.0f);
        icon->removeSurfaceObserver(this);
        icon->setHasRole(false);
        icon->reparentNode(nullptr);
    }
    if (!std::exchange(m_external, false))
        m_seat->setCursorOverride(std::nullopt);
}

void DragController::button(Timestamp timestamp, uint32_t button, bool pressed)
{
    if (!active())
        return;
    m_seat->updatePointerButtonState(button, pressed);
    if (pressed || !(buttonFlagFromEvdev(button) & m_button))
        return;
    drop();
    m_seat->releasePointerButton(timestamp, button);
    m_seat->onPointerMotion(timestamp, m_location);
}

void DragController::drop()
{
    if (!active())
        return;
    {
        const auto generation = m_generation;
        released();
        if (generation != m_generation || !active())
            return;
    }
    std::shared_ptr<DragOffer> selected;
    for (auto &weak : m_offers) {
        if (auto offer = weak.lock(); offer && offer.get() == selectedOffer())
            selected = std::move(offer);
    }
    if (!selected || !selected->canDrop()) {
        // A window dragged along stays where it is let go, so the source is
        // told of the drop before it hears that nothing took the data.
        const bool released = m_windowDelegate != nullptr;
        if (released && m_source) {
            const auto generation = m_generation;
            m_source->dropPerformed();
            if (generation != m_generation || !active())
                return;
        }
        end(released);
    } else {
        auto *window = std::exchange(m_windowDelegate, nullptr);
        m_finishing = true;
        auto source = std::exchange(m_source, nullptr);
        auto offers = std::exchange(m_offers, {});
        auto *target = std::exchange(m_target, nullptr);
        m_selected.reset();
        if (target && target != m_origin && target != m_icon)
            target->removeSurfaceObserver(this);
        detach();
        for (auto &weak : offers) {
            if (auto offer = weak.lock(); offer && offer != selected)
                offer->invalidate();
        }
        if (selected->drop())
            dropped(selected.get());
        else if (source)
            source->cancel();
        if (target)
            left();
        m_finishing = false;
        if (window)
            window->dragEnded(true);
        ended();
    }
}

void DragController::cancel()
{
    end(false);
}

void DragController::end(bool released)
{
    if (!active())
        return;
    const bool returns = m_returnsHome && !m_seat->wasShutdown();
    auto home = returns ? takeIconForReturn() : nullptr;
    auto *window = std::exchange(m_windowDelegate, nullptr);
    m_finishing = true;
    auto source = std::exchange(m_source, nullptr);
    auto offers = std::exchange(m_offers, {});
    auto *target = std::exchange(m_target, nullptr);
    m_selected.reset();
    if (target && target != m_origin && target != m_icon)
        target->removeSurfaceObserver(this);
    detach();
    for (auto &weak : offers) {
        if (auto offer = weak.lock())
            offer->invalidate();
    }
    if (target)
        left();
    if (source)
        source->cancel();
    m_finishing = false;
    if (window)
        window->dragEnded(released);
    if (returns)
        startReturn(std::move(home));
    ended();
    // Nothing to slide back: home at once, after the end.
    if (m_returning && !m_returnNode)
        finishReturn();
}

void DragController::setWindowDelegate(DragWindowDelegate *delegate)
{
    if (!active() || m_external)
        return;
    m_windowDelegate = delegate;
    if (!selectedOffer() || !selectedOffer()->canDrop())
        setCursor(idleCursor());
}

void DragController::setPlainIdleCursor(bool plain)
{
    if (!active() || m_external)
        return;
    m_plainIdleCursor = plain;
    if (!selectedOffer() || !selectedOffer()->canDrop())
        setCursor(idleCursor());
}

void DragController::setReturnsHome(bool returns)
{
    if (active() && !m_external)
        m_returnsHome = returns;
}

void DragController::setHome(const base::PointF &home)
{
    if (active() && !m_external)
        m_home = home;
}

std::unique_ptr<scene::SurfaceNode> DragController::takeIconForReturn() const
{
    scene::SurfaceNode *icon = m_icon ? m_icon->node() : nullptr;
    if (!icon || !icon->texture() || !m_seat->dragIconContainer())
        return nullptr;
    auto node = std::make_unique<scene::SurfaceNode>("drag-return");
    node->setGeometry(icon->geometry());
    node->setTextureMapping(icon->textureMapping());
    node->setAlpha(m_iconOpacity);
    node->setTexture(icon->takeTexture());
    return node;
}

void DragController::startReturn(std::unique_ptr<scene::SurfaceNode> node)
{
    m_returning = true;
    if (!node)
        return;
    m_returnFrom = node->position();
    // The icon keeps its offset from the pointer position.
    m_returnTo = m_returnFrom + (m_home - m_location).toPoint();
    m_returnOpacity = node->alpha();
    m_returnStarted = -1;
    m_returnNode = static_cast<scene::SurfaceNode *>(
            m_seat->dragIconContainer()->addChild(std::move(node)));
    if (compositor::Compositor *compositor = m_seat->compositor()) {
        m_returnFrames = compositor->frameStarted.connect([this](double frameTimeMs) {
            advanceReturn(frameTimeMs);
        });
        compositor->scheduleRepaint();
    }
}

void DragController::advanceReturn(double frameTimeMs)
{
    if (!m_returning || !m_returnNode)
        return;
    if (m_returnStarted < 0)
        m_returnStarted = frameTimeMs;
    const double t = std::clamp((frameTimeMs - m_returnStarted) / kReturnDurationMs, 0.0, 1.0);
    const auto &curve = scene::motion::kAccel20Decel100;
    const double eased = scene::motion::cubicBezier(curve.x1, curve.y1, curve.x2, curve.y2, t);
    const base::Point travel = m_returnTo - m_returnFrom;
    m_returnNode->setPosition(m_returnFrom
                              + base::Point(base::roundToInt(travel.x() * eased),
                                            base::roundToInt(travel.y() * eased)));
    m_returnNode->setAlpha(m_returnOpacity + (1.0f - m_returnOpacity) * float(eased));
    if (t >= 1)
        finishReturn();
    else
        m_seat->compositor()->scheduleRepaint();
}

void DragController::finishReturn()
{
    if (!m_returning)
        return;
    m_returning = false;
    m_returnFrames.reset();
    returned();
}

void DragController::releaseReturn()
{
    finishReturn();
    if (auto *node = std::exchange(m_returnNode, nullptr)) {
        if (scene::Node *parent = node->parent())
            parent->removeChild(node);
    }
}

void DragController::notifyWindowTarget()
{
    if (m_windowDelegate)
        m_windowDelegate->dragTargetChanged(m_target,
                                            selectedOffer() && selectedOffer()->canDrop());
}

CursorShape DragController::idleCursor() const
{
    if (m_windowDelegate)
        return CursorShape::Grabbing;
    return m_plainIdleCursor ? CursorShape::Default : CursorShape::NoDrop;
}

uint32_t DragController::modifierAction() const
{
    const auto *xkb = m_seat->xkbTracker();
    // Under the Command convention Option copies and Command moves.
    const bool command = m_seat->shortcutConvention() == ShortcutConvention::Command;
    if (xkb->modifierActive(command ? XKB_MOD_NAME_ALT : XKB_MOD_NAME_CTRL))
        return 1;
    if (xkb->modifierActive(command ? XKB_MOD_NAME_LOGO : XKB_MOD_NAME_SHIFT))
        return 2;
    return 0;
}

bool DragController::key(uint32_t code, bool pressed)
{
    // An external drag's keys, Escape included, belong to the X11 client.
    if (!grabsInput())
        return false;
    if (code == KEY_ESC && pressed) {
        cancel();
    } else {
        const auto offers = m_offers;
        for (auto &weak : offers) {
            if (auto offer = weak.lock())
                offer->setModifierAction(modifierAction());
        }
    }
    return true;
}

void DragController::setIconOpacity(float opacity)
{
    if (!active())
        return;
    m_iconOpacity = std::clamp(opacity, 0.0f, 1.0f);
    updateIcon();
}

void DragController::updateIcon()
{
    if (!m_icon)
        return;
    m_icon->setPosition(m_location.toPoint() + m_icon->offset());
    // Subsurfaces can be added to the icon at any commit.
    setSubtreeAlpha(m_icon->node(), m_iconOpacity);
}

void DragController::onSurfaceDestroying(compositor::Surface *surface)
{
    if (surface == m_icon)
        m_icon = nullptr;
    if (surface == m_origin) {
        cancel();
    } else if (surface == m_target) {
        leave();
    }
}

void DragController::onSurfaceCommit(compositor::Surface *surface)
{
    if (surface == m_icon)
        updateIcon();
    // Only a destroyed origin ends the drag, as in exo: Chrome hides the window
    // it drags by its tab when merging it into another, and drags on.
    if (surface == m_target && !surface->isMapped())
        leave();
}

} // namespace Lingmo::Input
