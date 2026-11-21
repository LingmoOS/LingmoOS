// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/seat.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "input/seat.h"
#include "base/auto_reset.h"
#include "input/session_gate.h"

#include <algorithm>


#include "compositor/compositor.h"
#include "compositor/surface.h"
#include "core/logging.h"
#include "input/hit_test.h"
#include "input/keyboard.h"
#include "input/pointer.h"
#include "input/seat_observer.h"
#include "input/touch.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Input {

Seat::Seat(compositor::Compositor *compositor)
    : m_drag(this)
    , m_pointerConstraints(this)
    , m_shortcutsInhibition(this)
    , m_compositor(compositor)
    , m_xkbTracker(std::make_unique<XkbTracker>())
    , m_clientXkbTracker(std::make_unique<XkbTracker>())
    , m_keyTranslator(m_clientXkbTracker.get(),
                      [this](Timestamp timestamp, uint32_t evdevCode, bool pressed) {
                          deliverKey(timestamp, evdevCode, pressed);
                      })
{
    m_keyTranslator.setPointerButtonsProvider([this] {
        return m_pressedButtons != NoButton;
    });
}

Seat::~Seat()
{
    shutdown();
}

void Seat::shutdown()
{
    if (m_wasShutdown)
        return;
    // First, so the drag cancelled here leaves no icon sliding home.
    m_wasShutdown = true;
    m_drag.cancel();
    m_primarySelection.setRecipient(nullptr);
    m_primarySelection.setSource(nullptr);
    m_selection.setRecipient(nullptr);
    m_selection.setSource(nullptr);
    if (m_pointerGrabSurface) {
        compositor::Surface *grab = m_pointerGrabSurface;
        m_pointerGrabSurface = nullptr;
        maybeRemoveSurfaceObserver(grab);
    }
    if (m_focusedSurface) {
        m_focusedSurface->removeSurfaceObserver(this);
        m_focusedSurface = nullptr;
    }
}

void Seat::addObserver(SeatObserver *observer)
{
    if (std::find(m_observers.begin(), m_observers.end(), observer) == m_observers.end())
        m_observers.push_back(observer);
}

void Seat::removeObserver(SeatObserver *observer)
{
    m_observers.erase(std::remove(m_observers.begin(), m_observers.end(), observer),
                      m_observers.end());
}

bool Seat::setKeyboardLayout(const std::string &name)
{
    if (!m_xkbTracker->updateKeyboardLayout(name))
        return false;
    for (const auto &[code, state] : m_pressedKeys)
        m_xkbTracker->updateKey(code, true);
    // The same keymap compiled again cannot fail where the first did not.
    m_clientXkbTracker->updateKeyboardLayout(name);
    for (const uint32_t code : m_keyTranslator.deliveredKeys())
        m_clientXkbTracker->updateKey(code, true);
    for (SeatObserver *observer : std::vector<SeatObserver *>(m_observers)) {
        if (std::find(m_observers.begin(), m_observers.end(), observer) != m_observers.end())
            observer->onKeyboardLayoutUpdated();
    }
    return true;
}

void Seat::setKeyRepeatSettings(const KeyRepeatSettings &settings)
{
    if (m_keyRepeatSettings == settings)
        return;
    m_keyRepeatSettings = settings;
    for (SeatObserver *observer : std::vector<SeatObserver *>(m_observers)) {
        if (std::find(m_observers.begin(), m_observers.end(), observer) != m_observers.end())
            observer->onKeyRepeatSettingsChanged();
    }
}

void Seat::addPointer(Pointer *pointer)
{
    if (std::find(m_pointers.begin(), m_pointers.end(), pointer) == m_pointers.end())
        m_pointers.push_back(pointer);
}

void Seat::removePointer(Pointer *pointer)
{
    m_pointers.erase(std::remove(m_pointers.begin(), m_pointers.end(), pointer), m_pointers.end());
}

void Seat::addKeyboard(Keyboard *keyboard)
{
    if (std::find(m_keyboards.begin(), m_keyboards.end(), keyboard) == m_keyboards.end())
        m_keyboards.push_back(keyboard);
}

void Seat::removeKeyboard(Keyboard *keyboard)
{
    m_keyboards.erase(std::remove(m_keyboards.begin(), m_keyboards.end(), keyboard),
                      m_keyboards.end());
}

void Seat::addTouch(Touch *touch)
{
    if (std::find(m_touches.begin(), m_touches.end(), touch) == m_touches.end())
        m_touches.push_back(touch);
}

void Seat::removeTouch(Touch *touch)
{
    m_touches.erase(std::remove(m_touches.begin(), m_touches.end(), touch), m_touches.end());
}

void Seat::setCursorHiddenByTouch(bool hidden)
{
    if (m_cursorHiddenByTouch == hidden)
        return;
    m_cursorHiddenByTouch = hidden;
    // Hover goes with the cursor, as aura's mouse exit with EF_CURSOR_HIDE
    // clears it; the next mouse event enters again where the pointer is.
    if (hidden)
        clearPointerFocus();
    notifyCursorChanged();
}

void Seat::onTouchDown(Timestamp timestamp, int32_t id, const base::PointF &rootLocation)
{
    // A drag owns the seat: no client gets a new point until it ends.
    if (m_wasShutdown || m_drag.grabsInput())
        return;
    // A locked session targets its secure container only, as the pointer does.
    base::PointF surfaceLocation;
    compositor::Surface *target = findTargetSurfaceIn(
            m_compositor, inputRootFor(m_compositor, m_sessionGate), rootLocation, &surfaceLocation);
    if (target)
        m_touchPoints[id] = DeliveredTouch {rootLocation, target->lifetimeToken(), ++m_touchOrder};
    for (Touch *touch : std::vector<Touch *>(m_touches))
        touch->onTouchDown(target, timestamp, id, rootLocation);
}

void Seat::onTouchMotion(Timestamp timestamp, int32_t id, const base::PointF &rootLocation)
{
    if (m_wasShutdown)
        return;
    if (m_drag.grabsInput() && m_drag.touchId() == id) {
        m_drag.motion(timestamp, rootLocation);
        return;
    }
    if (auto it = m_touchPoints.find(id); it != m_touchPoints.end())
        it->second.location = rootLocation;
    for (Touch *touch : std::vector<Touch *>(m_touches))
        touch->onTouchMotion(timestamp, id, rootLocation);
}

void Seat::onTouchUp(Timestamp timestamp, int32_t id)
{
    if (m_wasShutdown)
        return;
    if (m_drag.grabsInput() && m_drag.touchId() == id) {
        m_drag.drop();
        return;
    }
    m_touchPoints.erase(id);
    for (Touch *touch : std::vector<Touch *>(m_touches))
        touch->onTouchUp(timestamp, id);
}

void Seat::onTouchCancel()
{
    if (m_wasShutdown)
        return;
    if (m_drag.grabsInput() && m_drag.touchId())
        m_drag.cancel();
    m_touchPoints.clear();
    for (Touch *touch : std::vector<Touch *>(m_touches))
        touch->onTouchCancel();
}

std::optional<Seat::TouchPoint> Seat::touchPointOn(const compositor::Surface *root) const
{
    std::optional<TouchPoint> newest;
    uint64_t newestOrder = 0;
    for (const auto &[id, point] : m_touchPoints) {
        compositor::Surface *surface = point.surface ? *point.surface : nullptr;
        while (surface && surface != root)
            surface = surface->parentSurface();
        if (!surface || !root || point.order < newestOrder)
            continue;
        newest = TouchPoint {id, point.location};
        newestOrder = point.order;
    }
    return newest;
}

void Seat::takeTouchPoint(int32_t id)
{
    if (m_wasShutdown || !m_touchPoints.contains(id))
        return;
    for (Touch *touch : std::vector<Touch *>(m_touches)) {
        if (touch->tracks(id))
            touch->onTouchCancel();
    }
    // A cancel ends all of a client's points; what no client has any more is
    // no longer the seat's either.
    std::erase_if(m_touchPoints, [this](const auto &entry) {
        return std::none_of(m_touches.begin(), m_touches.end(), [&](Touch *touch) {
            return touch->tracks(entry.first);
        });
    });
}

void Seat::onTouchFrame()
{
    if (m_wasShutdown)
        return;
    for (Touch *touch : std::vector<Touch *>(m_touches))
        touch->onTouchFrame();
}

bool Seat::hasClientCursor() const
{
    for (const Pointer *pointer : m_pointers) {
        if (pointer->cursorSurface())
            return true;
    }
    return false;
}

bool Seat::hasClientCursorRequest() const
{
    for (const Pointer *pointer : m_pointers) {
        if (pointer->hasCursorRequest())
            return true;
    }
    return false;
}

std::optional<CursorShape> Seat::clientCursorShape() const
{
    for (const Pointer *pointer : m_pointers) {
        if (pointer->hasCursorRequest())
            return pointer->cursorShape();
    }
    return std::nullopt;
}

void Seat::setCompositorCursorShape(CursorShape shape)
{
    if (m_compositorCursorShape == shape)
        return;
    m_compositorCursorShape = shape;
    notifyCursorChanged();
}

void Seat::setCursorOverride(std::optional<CursorShape> shape)
{
    if (m_cursorOverride == shape)
        return;
    m_cursorOverride = shape;
    notifyCursorChanged();
}

void Seat::notifyCursorChanged()
{
    for (SeatObserver *observer : std::vector<SeatObserver *>(m_observers)) {
        if (std::find(m_observers.begin(), m_observers.end(), observer) != m_observers.end())
            observer->onCursorChanged();
    }
}

void Seat::traceCursors(const char *transition) const
{
    if (!lingmoCursorTransitions.isEnabled(base::LogLevel::Debug))
        return;
    for (const Pointer *pointer : m_pointers)
        pointer->traceCursor(transition);
}

void Seat::updateCursorPosition(const base::PointF &position)
{
    if (m_wasShutdown)
        return;
    m_cursorPosition = position;
    // Presentation follows physical input even while WM owns event dispatch.
    // Do not update pointer focus, pressed buttons or keyboard/selection state.
    for (Pointer *pointer : std::vector<Pointer *>(m_pointers)) {
        if (std::find(m_pointers.begin(), m_pointers.end(), pointer) != m_pointers.end())
            pointer->setCursorPosition(position);
    }
    for (SeatObserver *observer : std::vector<SeatObserver *>(m_observers)) {
        if (std::find(m_observers.begin(), m_observers.end(), observer) != m_observers.end())
            observer->onCursorPositionChanged();
    }
}

void Seat::notifySurfaceFocused(compositor::Surface *gainedFocus, compositor::Surface *lostFocus)
{
    // A keyboard may be destroyed from within a focus callback, so iterate a
    // copy. Exo gets the same protection from base::ObserverList.
    for (SeatObserver *observer : std::vector<SeatObserver *>(m_observers)) {
        if (std::find(m_observers.begin(), m_observers.end(), observer) != m_observers.end())
            observer->onSurfaceFocused(gainedFocus, lostFocus);
    }
}

void Seat::maybeRemoveSurfaceObserver(compositor::Surface *surface)
{
    if (surface && surface != m_focusedSurface && surface != m_pointerGrabSurface)
        surface->removeSurfaceObserver(this);
}

void Seat::setPointerGrabSurface(compositor::Surface *surface)
{
    if (surface == m_pointerGrabSurface)
        return;

    compositor::Surface *released = m_pointerGrabSurface;
    m_pointerGrabSurface = surface;
    m_dragSerials.clear();
    if (m_pointerGrabSurface)
        m_pointerGrabSurface->addSurfaceObserver(this);
    maybeRemoveSurfaceObserver(released);
}

void Seat::setSessionGate(SessionGate *gate)
{
    m_sessionGate = gate;
}

bool Seat::allowsFocus(const compositor::Surface *surface) const
{
    return !m_sessionGate || m_sessionGate->allowsFocus(surface);
}

void Seat::setFocusedSurface(compositor::Surface *surface, bool borrowed)
{
    // A refused surface clears focus rather than leaving the old one behind:
    // the surface that held focus before a lock is exactly what must not keep
    // receiving keys.
    if (surface && !allowsFocus(surface))
        surface = nullptr;
    borrowed = borrowed && surface;
    if (m_wasShutdown || (surface == m_focusedSurface && borrowed == m_focusBorrowed))
        return;

    compositor::Surface *lostFocus = m_focusedSurface;

    m_focusedSurface = surface;
    m_focusBorrowed = borrowed;
    if (m_focusedSurface)
        m_focusedSurface->addSurfaceObserver(this);
    maybeRemoveSurfaceObserver(lostFocus);

    notifySurfaceFocused(m_focusedSurface, lostFocus);
    m_shortcutsInhibition.update();
    m_pointerConstraints.update();
}

compositor::Surface *Seat::pointerFocus() const
{
    for (const Pointer *pointer : m_pointers) {
        if (pointer->focusedSurface())
            return pointer->focusedSurface();
    }
    return nullptr;
}

bool Seat::breakInputCapture()
{
    const bool inhibited = m_shortcutsInhibition.breakByUserAction();
    const bool constrained = m_pointerConstraints.breakByUserAction();
    return inhibited || constrained;
}

void Seat::onSurfaceDestroying(compositor::Surface *surface)
{
    // The surface removes every observer itself once this returns, so drop the
    // references without calling back into it.
    if (surface == m_pointerGrabSurface)
        m_pointerGrabSurface = nullptr;
    if (surface != m_focusedSurface)
        return;
    m_focusedSurface = nullptr;
    m_focusBorrowed = false;
    notifySurfaceFocused(nullptr, surface);
    m_shortcutsInhibition.update();
    m_pointerConstraints.update();
}

void Seat::clearPointerFocus()
{
    if (m_wasShutdown)
        return;
    // A pointer may be destroyed from inside the leave it is being sent.
    for (Pointer *pointer : std::vector<Pointer *>(m_pointers)) {
        if (std::find(m_pointers.begin(), m_pointers.end(), pointer) != m_pointers.end())
            pointer->clearFocus();
    }
    m_pointerConstraints.update();
}

compositor::Surface *Seat::pointerTargetAt(scene::Node *root, const base::PointF &location,
                                           base::PointF *surfaceLocation)
{
    if (m_inPointerMotion && m_pointerTarget && m_pointerTarget->root == root
        && fuzzyEqual(m_pointerTarget->location, location)
        && (!m_pointerTarget->surface || *m_pointerTarget->surface)) {
        *surfaceLocation = m_pointerTarget->surfaceLocation;
        return m_pointerTarget->surface ? *m_pointerTarget->surface : nullptr;
    }
    compositor::Surface *target = findTargetSurfaceIn(m_compositor, root, location, surfaceLocation);
    if (m_inPointerMotion) {
        m_pointerTarget = PointerTarget {root, location,
                                         target ? target->lifetimeToken() : nullptr,
                                         *surfaceLocation};
    }
    return target;
}

void Seat::beginPointerMotion()
{
    m_inPointerMotion = true;
    m_pointerTarget.reset();
}

void Seat::endPointerMotion()
{
    m_inPointerMotion = false;
    m_pointerTarget.reset();
}

void Seat::onPointerMotion(Timestamp timestamp, const base::PointF &rootLocation,
                           const RelativeMotion *relative)
{
    if (m_wasShutdown)
        return;
    m_pointerLocation = rootLocation;
    if (m_drag.grabsInput()) {
        // A drag that follows a finger is not the pointer's to move.
        if (!m_drag.touchId())
            m_drag.motion(timestamp, rootLocation);
        return;
    }
    for (Pointer *pointer : std::vector<Pointer *>(m_pointers))
        pointer->onPointerMotion(timestamp, rootLocation, relative);
    // After every pointer has its new focus: a lock's cursor hint warps, which
    // dispatches motion again.
    m_pointerConstraints.update();
}

void Seat::onPointerButton(Timestamp timestamp, const base::PointF &rootLocation, uint32_t evdevButton,
                           bool pressed)
{
    if (m_wasShutdown)
        return;

    m_pointerLocation = rootLocation;
    // Command+click is Ctrl+click to the client.
    if (pressed)
        m_keyTranslator.flushModifiers(timestamp);
    if (m_drag.grabsInput()) {
        m_drag.button(timestamp, evdevButton, pressed);
        return;
    }
    const ButtonFlags changed = buttonFlagFromEvdev(evdevButton);
    if (changed == NoButton)
        return;

    // The pressed set is updated before dispatch so that a pointer entering a
    // surface during this event reports the button as already down, which is
    // what wl_pointer.enter's caller expects.
    updatePointerButtonState(evdevButton, pressed);

    // The first press takes the implicit grab, so the rest of the drag stays
    // with this surface however far the pointer travels.
    if (pressed && !m_pointerGrabSurface) {
        base::PointF surfaceLocation;
        setPointerGrabSurface(findTargetSurface(m_compositor, rootLocation, &surfaceLocation));
    }

    for (Pointer *pointer : std::vector<Pointer *>(m_pointers))
        pointer->onPointerButton(timestamp, m_pointerLocation, changed, pressed);

    if (pressed) {
        m_shortcutsInhibition.onPointerPressed(m_pointerGrabSurface);
        m_pointerConstraints.onPointerPressed(m_pointerGrabSurface);
    }
    m_pointerConstraints.update();
    endPointerGrabIfReleased();
}

void Seat::updatePointerButtonState(uint32_t evdevButton, bool pressed)
{
    if (m_wasShutdown)
        return;
    const auto changed = buttonFlagFromEvdev(evdevButton);
    if (pressed) {
        m_pressedButtons |= changed;
    } else {
        m_pressedButtons &= ~changed;
        std::erase_if(m_dragSerials, [changed](const auto &entry) {
            return entry.second & changed;
        });
    }
}

void Seat::recordDragSerial(uint32_t serial, ButtonFlags button)
{
    if (m_pointerGrabSurface && (m_pressedButtons & button)) {
        if (m_dragSerials.size() >= 256)
            m_dragSerials.clear();
        m_dragSerials[serial] = button;
    }
}

ButtonFlags Seat::dragSerial(compositor::Surface *origin, uint32_t serial) const
{
    const auto it = m_dragSerials.find(serial);
    return origin && origin == m_pointerGrabSurface && it != m_dragSerials.end()
            ? it->second & m_pressedButtons
            : NoButton;
}

void Seat::endPointerGrabIfReleased()
{
    // wl_pointer's implicit grab ends once the last button is up, where aura
    // drops its pressed handler on any release. Only after the release has been
    // delivered: it still belongs to the grabbed surface.
    if (m_pressedButtons == NoButton)
        setPointerGrabSurface(nullptr);
}

void Seat::releasePointerButton(Timestamp timestamp, uint32_t evdevButton)
{
    if (m_wasShutdown)
        return;
    const auto changed = buttonFlagFromEvdev(evdevButton);
    updatePointerButtonState(evdevButton, false);
    for (Pointer *pointer : std::vector<Pointer *>(m_pointers)) {
        if (std::find(m_pointers.begin(), m_pointers.end(), pointer) != m_pointers.end())
            pointer->releasePressedButtons(timestamp, changed);
    }

    endPointerGrabIfReleased();
}

void Seat::onPointerAxis(Timestamp timestamp, const base::PointF &rootLocation, const base::PointF &offset,
                         bool discrete)
{
    if (m_wasShutdown)
        return;
    m_pointerLocation = rootLocation;
    m_keyTranslator.flushModifiers(timestamp);
    for (Pointer *pointer : std::vector<Pointer *>(m_pointers))
        pointer->onPointerAxis(timestamp, m_pointerLocation, offset, discrete);
    m_pointerConstraints.update();
}

void Seat::onPointerAxisStop(Timestamp timestamp, const base::PointF &rootLocation)
{
    if (m_wasShutdown)
        return;
    m_pointerLocation = rootLocation;
    for (Pointer *pointer : std::vector<Pointer *>(m_pointers))
        pointer->onPointerAxisStop(timestamp, m_pointerLocation);
}

void Seat::onPointerGestureBegin(Timestamp timestamp, const base::PointF &rootLocation,
                                 GestureType type, uint32_t fingers)
{
    if (m_wasShutdown || m_drag.grabsInput())
        return;
    m_pointerLocation = rootLocation;
    for (Pointer *pointer : std::vector<Pointer *>(m_pointers)) {
        if (std::find(m_pointers.begin(), m_pointers.end(), pointer) != m_pointers.end())
            pointer->onPointerGestureBegin(timestamp, m_pointerLocation, type, fingers);
    }
}

void Seat::onPointerGestureUpdate(Timestamp timestamp, GestureType type,
                                  const GestureUpdate &update)
{
    if (m_wasShutdown)
        return;
    for (Pointer *pointer : std::vector<Pointer *>(m_pointers)) {
        if (std::find(m_pointers.begin(), m_pointers.end(), pointer) != m_pointers.end())
            pointer->onPointerGestureUpdate(timestamp, type, update);
    }
}

void Seat::onPointerGestureEnd(Timestamp timestamp, GestureType type, bool cancelled)
{
    if (m_wasShutdown)
        return;
    for (Pointer *pointer : std::vector<Pointer *>(m_pointers)) {
        if (std::find(m_pointers.begin(), m_pointers.end(), pointer) != m_pointers.end())
            pointer->onPointerGestureEnd(timestamp, type, cancelled);
    }
}

std::map<uint32_t, KeyState> Seat::clientPressedKeys() const
{
    std::map<uint32_t, KeyState> keys;
    for (const uint32_t code : m_keyTranslator.deliveredKeys()) {
        if (!m_inputMethodKeys.contains(code))
            keys.emplace(code, KeyState {code});
    }
    return keys;
}

bool Seat::updateKeyState(uint32_t evdevCode, bool pressed)
{
    if (m_wasShutdown)
        return false;
    if (pressed) {
        if (m_pressedKeys.contains(evdevCode))
            return false;
        m_pressedKeys.emplace(evdevCode, KeyState {evdevCode});
    } else {
        if (!m_pressedKeys.erase(evdevCode))
            return false;
        m_suppressedKeys.erase(evdevCode);
    }
    m_xkbTracker->updateKey(evdevCode, pressed);
    return true;
}

void Seat::notifyKeyboardModifiers()
{
    for (SeatObserver *observer : std::vector<SeatObserver *>(m_observers)) {
        if (std::find(m_observers.begin(), m_observers.end(), observer) != m_observers.end())
            observer->onKeyboardModifierUpdated();
    }
}

void Seat::syncKeyboardLocks()
{
    if (m_clientXkbTracker->copyLocks(*m_xkbTracker))
        notifyKeyboardModifiers();
}

void Seat::dispatchKey(Timestamp timestamp, uint32_t evdevCode, bool pressed)
{
    if (m_wasShutdown)
        return;
    m_keyTranslator.onKey(timestamp, evdevCode, pressed);
}

void Seat::deliverKey(Timestamp timestamp, uint32_t evdevCode, bool pressed)
{
    if (m_wasShutdown)
        return;
    const KeyboardModifiers before = m_clientXkbTracker->modifiers();
    m_clientXkbTracker->updateKey(evdevCode, pressed);
    // A translated key can carry modifiers of its own, which must reach the
    // clients before the next key does.
    const base::ScopeGuard modifiers([this, before] {
        if (m_clientXkbTracker->modifiers() != before)
            notifyKeyboardModifiers();
    });
    const bool captured = m_inputMethodKeys.contains(evdevCode);
    if ((pressed || captured) && m_inputMethodFilter
        && m_inputMethodFilter(timestamp, evdevCode, pressed)) {
        if (pressed)
            m_inputMethodKeys.insert(evdevCode);
        else
            m_inputMethodKeys.erase(evdevCode);
        return;
    }
    if (!pressed && m_inputMethodKeys.erase(evdevCode))
        return;
    dispatchKeyToClients(timestamp, evdevCode, pressed);
}

void Seat::tapKeys(Timestamp timestamp, const std::vector<uint32_t> &codes)
{
    for (uint32_t code : codes)
        deliverKey(timestamp, code, true);
    for (auto it = codes.rbegin(); it != codes.rend(); ++it)
        deliverKey(timestamp, *it, false);
}

void Seat::dispatchKeyToClients(Timestamp timestamp, uint32_t evdevCode, bool pressed)
{
    if (m_wasShutdown)
        return;
    for (Keyboard *keyboard : std::vector<Keyboard *>(m_keyboards)) {
        if (std::find(m_keyboards.begin(), m_keyboards.end(), keyboard) != m_keyboards.end())
            keyboard->onKey(timestamp, evdevCode, pressed);
    }
}

void Seat::onKey(Timestamp timestamp, uint32_t evdevCode, bool pressed)
{
    const bool suppressed = keySuppressed(evdevCode);
    if (!updateKeyState(evdevCode, pressed))
        return;
    if (m_drag.key(evdevCode, pressed)) {
        if (pressed)
            suppressKey(evdevCode);
        else if (!suppressed)
            dispatchKey(timestamp, evdevCode, false);
        syncKeyboardLocks();
        return;
    }
    if (!suppressed)
        dispatchKey(timestamp, evdevCode, pressed);
}

} // namespace Lingmo::Input
