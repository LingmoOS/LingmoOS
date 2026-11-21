// Copyright 2017 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/seat.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.
//
// Lingmo owns xkb state, client event dispatch, selection and drag capture.

#ifndef LINGMO_INPUT_SEAT_H_
#define LINGMO_INPUT_SEAT_H_

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <vector>


#include "clipboard/selection.h"
#include "base/geometry.h"
#include "compositor/surface_observer.h"
#include "input/cursor_shape.h"
#include "input/drag.h"
#include "input/pointer_gesture_delegate.h"
#include "input/pointer_constraints.h"
#include "input/shortcuts_inhibition.h"
#include "input/types.h"
#include "input/key_translator.h"
#include "input/xkb_tracker.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Compositor {
class Compositor;
}

namespace Lingmo::Scene {
class Node;
}

namespace Lingmo::Input {

class Keyboard;
class Pointer;
struct RelativeMotion;
class SeatObserver;
class SessionGate;
class Touch;

// A group of input devices, and the input focus they share.
class Seat : public compositor::SurfaceObserver
{
public:
    explicit Seat(compositor::Compositor *compositor);
    Seat(const Seat &) = delete;
    Seat &operator=(const Seat &) = delete;
    ~Seat() override;

    // After shutdown the seat stops dispatching. Called before the clients are
    // torn down, so a Pointer or Keyboard being destroyed cannot re-enter.
    void shutdown();

    bool wasShutdown() const { return m_wasShutdown; }

    compositor::Compositor *compositor() const { return m_compositor; }

    // The keys as the compositor sees them, which is what its own shortcuts
    // match: under the Command convention, Command is Meta here.
    const XkbTracker *xkbTracker() const { return m_xkbTracker.get(); }

    XkbTracker *xkbTracker() { return m_xkbTracker.get(); }

    // The keys as clients see them, after the KeyTranslator: wl_keyboard and
    // the input method take their modifiers from here.
    const XkbTracker *clientXkbTracker() const { return m_clientXkbTracker.get(); }

    KeyTranslator *keyTranslator() { return &m_keyTranslator; }

    ShortcutConvention shortcutConvention() const { return m_keyTranslator.convention(); }

    void setShortcutConvention(ShortcutConvention convention)
    {
        m_keyTranslator.setConvention(convention);
    }

    // Replaces the keymap of every keyboard; see XkbTracker::updateKeyboardLayout.
    // Keys held across the change stay held in the new state.
    bool setKeyboardLayout(const std::string &name);

    const KeyRepeatSettings &keyRepeatSettings() const { return m_keyRepeatSettings; }

    void setKeyRepeatSettings(const KeyRepeatSettings &settings);

    void addObserver(SeatObserver *observer);
    void removeObserver(SeatObserver *observer);

    scene::Node *dragIconContainer() const { return m_dragIconContainer; }

    void setDragIconContainer(scene::Node *container) { m_dragIconContainer = container; }

    // The scene node cursor surfaces are parented into while they are the
    // pointer image. Null leaves the cursor unrendered, which is what the
    // protocol tests use.
    scene::Node *cursorContainer() const { return m_cursorContainer; }

    void setCursorContainer(scene::Node *container) { m_cursorContainer = container; }

    // Image presence and cursor ownership differ: a valid null request owns
    // a hidden cursor. Only absence of an owned request permits the fallback.
    bool hasClientCursor() const;
    bool hasClientCursorRequest() const;
    void notifyCursorChanged();
    std::optional<CursorShape> clientCursorShape() const;

    CursorShape compositorCursorShape() const { return m_compositorCursorShape; }

    void setCompositorCursorShape(CursorShape shape);

    std::optional<CursorShape> cursorOverride() const { return m_cursorOverride; }

    void setCursorOverride(std::optional<CursorShape> shape);

    base::PointF cursorPosition() const { return m_cursorPosition; }

    // A touch hides the pointer until the mouse moves, clicks or scrolls
    // again, as wm::CompoundEventFilter does on a touch press.
    bool cursorHiddenByTouch() const { return m_cursorHiddenByTouch; }

    // Hiding also takes the pointer's focus away, so that no hover is left.
    void setCursorHiddenByTouch(bool hidden);

    void traceCursors(const char *transition) const;
    void updateCursorPosition(const base::PointF &position);
    void releasePointerButton(Timestamp timestamp, uint32_t evdevButton);
    void updatePointerButtonState(uint32_t evdevButton, bool pressed);
    void endPointerGrabIfReleased();

    void consumeDragSerials() { m_dragSerials.clear(); }

    void recordDragSerial(uint32_t serial, ButtonFlags button);
    ButtonFlags dragSerial(compositor::Surface *origin, uint32_t serial) const;

    DragController *drag() { return &m_drag; }

    PointerConstraints *pointerConstraints() { return &m_pointerConstraints; }

    ShortcutsInhibition *shortcutsInhibition() { return &m_shortcutsInhibition; }

    // The user took the pointer and the keyboard back from the focused window:
    // breaks its pointer constraint and its shortcuts inhibitor until the next
    // click on it. Returns whether either was active.
    bool breakInputCapture();

    clipboard::SelectionController *primarySelection() { return &m_primarySelection; }

    clipboard::SelectionController *selection() { return &m_selection; }

    // --- Focus ---

    // Whether the session lets a surface hold focus and where input may be
    // targeted at all. Null leaves the seat unrestricted, which is what the
    // protocol-only tests want.
    void setSessionGate(SessionGate *gate);

    SessionGate *sessionGate() const { return m_sessionGate; }

    // Whether `surface` may hold keyboard or pointer focus. Every focus path -
    // the window manager, xdg-activation, Xwayland, the pointer's own hit test
    // - goes through this, so a locked session has one place to fail closed.
    bool allowsFocus(const compositor::Surface *surface) const;

    compositor::Surface *focusedSurface() const { return m_focusedSurface; }

    // Whether the focused surface only borrows the keys; see setFocusedSurface().
    bool focusIsBorrowed() const { return m_focusBorrowed; }

    // The surface the client pointers are focused on, if any.
    compositor::Surface *pointerFocus() const;

    // The window manager sets this from window activation. The surface is
    // expected to be a root surface: keyboard focus follows the hierarchy, not
    // the individual subsurface that was hit.
    // A `borrowed` focus takes the keys without taking focus from the client
    // that has it: that client gets no leave, since toolkits deactivate on one,
    // and no new key presses until focus returns.
    void setFocusedSurface(compositor::Surface *surface, bool borrowed = false);

    // Makes every pointer leave whatever it is focused on. The window manager
    // uses it when the pointer moves onto compositor-owned content.
    void clearPointerFocus();

    // The surface holding the implicit pointer grab: the one a press was
    // delivered to, which keeps every pointer event until the last button is
    // released, wherever the pointer travels. Counterpart of aura's
    // WindowEventDispatcher::mouse_pressed_handler_.
    compositor::Surface *pointerGrabSurface() const { return m_pointerGrabSurface; }

    // Updates the pressed keys and the compositor's xkb state, without telling
    // clients. What they get comes out of dispatchKey(): each delivered key,
    // then the modifiers it changed, as wlroots, KWin and Mutter send them: Qt
    // and GTK read a modifier key's own event against the state before it, and
    // would otherwise see Meta's press without Meta.
    bool updateKeyState(uint32_t evdevCode, bool pressed);
    void notifyKeyboardModifiers();
    // After a key the clients never got: a Caps Lock toggled on the lock
    // screen must reach them and the lights too, or the two states drift.
    void syncKeyboardLocks();
    void dispatchKey(Timestamp timestamp, uint32_t evdevCode, bool pressed);

    using KeyFilter = std::function<bool(Timestamp, uint32_t, bool)>;

    void setInputMethodFilter(KeyFilter filter) { m_inputMethodFilter = std::move(filter); }

    // Delivers a key that already went through the KeyTranslator, e.g. one the
    // input method forwards.
    void dispatchKeyToClients(Timestamp timestamp, uint32_t evdevCode, bool pressed);

    // Presses `codes` in order and releases them in reverse, as keys clients
    // see: past the compositor's shortcuts and the KeyTranslator.
    void tapKeys(Timestamp timestamp, const std::vector<uint32_t> &codes);

    bool keySuppressed(uint32_t code) const { return m_suppressedKeys.contains(code); }

    void suppressKey(uint32_t code) { m_suppressedKeys.insert(code); }

    // The delivered keys clients hold pressed, less those the input method took.
    std::map<uint32_t, KeyState> clientPressedKeys() const;

    // --- Physical event entry points, called by InputManager ---

    // Every pointer event carries the location, as it does in Exo, so a button
    // that arrives before any motion is still placed correctly.
    // `relative` is the physical motion behind this location, if there was one.
    void onPointerMotion(Timestamp timestamp, const base::PointF &rootLocation,
                         const RelativeMotion *relative = nullptr);
    void onPointerButton(Timestamp timestamp, const base::PointF &rootLocation, uint32_t evdevButton,
                         bool pressed);
    void onPointerAxis(Timestamp timestamp, const base::PointF &rootLocation, const base::PointF &offset,
                       bool discrete);
    void onPointerAxisStop(Timestamp timestamp, const base::PointF &rootLocation);
    void onPointerGestureBegin(Timestamp timestamp, const base::PointF &rootLocation,
                               GestureType type, uint32_t fingers);
    void onPointerGestureUpdate(Timestamp timestamp, GestureType type,
                                const GestureUpdate &update);
    void onPointerGestureEnd(Timestamp timestamp, GestureType type, bool cancelled);
    void onKey(Timestamp timestamp, uint32_t evdevCode, bool pressed);

    // A new touch point is targeted once, at the surface it lands on; the
    // rest of its events follow it there. `rootLocation` is in output
    // coordinates. Frames close each batch from the device.
    void onTouchDown(Timestamp timestamp, int32_t id, const base::PointF &rootLocation);
    void onTouchMotion(Timestamp timestamp, int32_t id, const base::PointF &rootLocation);
    void onTouchUp(Timestamp timestamp, int32_t id);
    void onTouchCancel();
    void onTouchFrame();

    struct TouchPoint {
        int32_t id = 0;
        base::PointF location;
    };

    // The newest point still down that a client was given on `root` or a
    // surface below it, and where it is now. What a move or resize requested
    // with a touch-down serial follows, as ash follows its gesture target.
    std::optional<TouchPoint> touchPointOn(const compositor::Surface *root) const;
    // Takes point `id` from the clients that have it. Each is sent a cancel,
    // which ends every point it had.
    void takeTouchPoint(int32_t id);

    base::PointF pointerLocation() const { return m_pointerLocation; }

    // The surface under `location` in `root`'s subtree. Every handler asking
    // during one pointer motion gets the one answer, as aura resolves a located
    // event's target once for all its handlers. Outside a motion, and for a
    // press, whose activation may restack, it is searched afresh.
    compositor::Surface *pointerTargetAt(scene::Node *root, const base::PointF &location,
                                         base::PointF *surfaceLocation);
    // Brackets the dispatch of one pointer motion.
    void beginPointerMotion();
    void endPointerMotion();

    ButtonFlags pressedButtons() const { return m_pressedButtons; }

    // Currently pressed keys, keyed by physical evdev code.
    const std::map<uint32_t, KeyState> &pressedKeys() const { return m_pressedKeys; }

    // --- Registration, used by Pointer and Keyboard ---

    void addPointer(Pointer *pointer);
    void removePointer(Pointer *pointer);
    void addKeyboard(Keyboard *keyboard);
    void removeKeyboard(Keyboard *keyboard);
    void addTouch(Touch *touch);
    void removeTouch(Touch *touch);

    // compositor::SurfaceObserver:
    void onSurfaceDestroying(compositor::Surface *surface) override;

private:
    void setPointerGrabSurface(compositor::Surface *surface);

    // Focus and the implicit grab observe the same surfaces, so an observer is
    // only dropped once neither refers to it.
    void maybeRemoveSurfaceObserver(compositor::Surface *surface);

    void notifySurfaceFocused(compositor::Surface *gainedFocus, compositor::Surface *lostFocus);

    // One key out of the KeyTranslator: into the client xkb state, then to
    // the input method or the clients, then the modifiers it changed.
    void deliverKey(Timestamp timestamp, uint32_t evdevCode, bool pressed);

    std::map<uint32_t, ButtonFlags> m_dragSerials;
    DragController m_drag;
    PointerConstraints m_pointerConstraints;
    ShortcutsInhibition m_shortcutsInhibition;
    clipboard::SelectionController m_selection;
    clipboard::SelectionController m_primarySelection;
    compositor::Compositor *m_compositor = nullptr;
    scene::Node *m_cursorContainer = nullptr;
    scene::Node *m_dragIconContainer = nullptr;
    std::unique_ptr<XkbTracker> m_xkbTracker;
    std::unique_ptr<XkbTracker> m_clientXkbTracker;
    KeyTranslator m_keyTranslator;
    KeyRepeatSettings m_keyRepeatSettings;

    std::vector<SeatObserver *> m_observers;
    std::vector<Pointer *> m_pointers;
    std::vector<Keyboard *> m_keyboards;
    std::vector<Touch *> m_touches;
    bool m_cursorHiddenByTouch = false;

    struct DeliveredTouch {
        base::PointF location;
        std::shared_ptr<compositor::Surface *> surface;
        uint64_t order = 0;
    };

    // Points a client was given, until they are lifted or cancelled.
    std::map<int32_t, DeliveredTouch> m_touchPoints;
    uint64_t m_touchOrder = 0;

    SessionGate *m_sessionGate = nullptr;
    compositor::Surface *m_focusedSurface = nullptr;
    bool m_focusBorrowed = false;
    compositor::Surface *m_pointerGrabSurface = nullptr;

    base::PointF m_pointerLocation;

    struct PointerTarget {
        scene::Node *root;
        base::PointF location;
        std::shared_ptr<compositor::Surface *> surface;
        base::PointF surfaceLocation;
    };

    bool m_inPointerMotion = false;
    std::optional<PointerTarget> m_pointerTarget;
    base::PointF m_cursorPosition;
    CursorShape m_compositorCursorShape = CursorShape::Default;
    std::optional<CursorShape> m_cursorOverride;
    ButtonFlags m_pressedButtons = NoButton;
    std::map<uint32_t, KeyState> m_pressedKeys;

    KeyFilter m_inputMethodFilter;
    std::set<uint32_t> m_inputMethodKeys;
    bool m_wasShutdown = false;
    std::set<uint32_t> m_suppressedKeys;
};

} // namespace Lingmo::Input

#endif // LINGMO_INPUT_SEAT_H_
