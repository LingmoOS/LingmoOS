// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Modelled on chromium/src/ash/wm/event_client_impl.cc
// (EventClientImpl::GetCanProcessEventsWithinSubtree) and
// chromium/src/ash/wm/window_util.cc (GetBlockingContainersForRoot).
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Ash asks the session controller whether the user session is blocked and, if
// it is, confines both event targeting and activation to the lock-screen
// containers. Lingmo keeps that shape: input and activation ask a gate, never
// a window role and never a z-order comparison.

#ifndef LINGMO_INPUT_SESSION_GATE_H_
#define LINGMO_INPUT_SESSION_GATE_H_

namespace Lingmo::Compositor {
class Surface;
}

namespace Lingmo::Scene {
class Node;
}

namespace Lingmo::Input {

class EventSink;

class SessionGate
{
public:
    virtual ~SessionGate() = default;

    // Whether input targeting is confined at all. False is the unlocked case.
    virtual bool restrictsInput() const = 0;

    virtual EventSink *secureInput() const { return nullptr; }

    // The only scene subtree input may target while restrictsInput() holds.
    // Null there means nothing is targetable, which is the safe answer when
    // there is no secure container to send input to.
    virtual scene::Node *inputRoot() const = 0;

    // Whether `surface` may hold keyboard or pointer focus right now.
    virtual bool allowsFocus(const compositor::Surface *surface) const = 0;

    // Whether any window may become active. Windows never live in the secure
    // container, so a blocked session refuses every one of them.
    virtual bool allowsActivation() const = 0;
};

} // namespace Lingmo::Input

#endif // LINGMO_INPUT_SESSION_GATE_H_
