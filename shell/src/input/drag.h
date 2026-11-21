// Copyright 2019 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/{drag_drop_operation,data_device}.cc
//           chromium/src/ash/drag_drop/drag_drop_controller.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#pragma once

#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "base/geometry.h"
#include "base/signal.h"
#include "base/weak_ptr.h"
#include "clipboard/selection.h"
#include "compositor/surface_observer.h"
#include "input/cursor_shape.h"
#include "input/types.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Scene {
class SurfaceNode;
}

namespace Lingmo::Input {

class Seat;

class DragOffer : public std::enable_shared_from_this<DragOffer>
{
public:
    enum class State {
        Hover,
        Dropped,
        Finished,
        Cancelled
    };
    DragOffer(std::shared_ptr<clipboard::DataSource> source, uint32_t sourceActions, bool legacy);
    ~DragOffer();
    void accept(const char* mime);
    void setActions(uint32_t actions, uint32_t preferred);
    void setModifierAction(uint32_t action);
    void receive(const std::string& mime, int fd);
    bool finish();
    void invalidate();
    bool drop();
    bool canDrop() const;

    uint32_t action() const { return m_action; }

    const std::string& acceptedMime() const { return m_mime; }

    uint32_t sourceActions() const { return m_sourceActions; }

    State state() const { return m_state; }

    const std::vector<std::string>& formats() const { return m_formats; }

    bool legacy() const { return m_legacy; }

    const std::shared_ptr<clipboard::DataSource>& source() const { return m_source; }

    base::Signal<> feedbackChanged;
    base::Signal<uint32_t> actionChanged;

private:
    void negotiate();
    std::shared_ptr<clipboard::DataSource> m_source;
    std::vector<std::string> m_formats;
    State m_state = State::Hover;
    uint32_t m_sourceActions;
    uint32_t m_actions;
    uint32_t m_preferred = 0;
    uint32_t m_modifier = 0;
    uint32_t m_action = 0;
    bool m_legacy;
    std::string m_mime;
};

// Moves a window along with a drag, for xdg_toplevel_drag_v1. The data
// transfer goes on as usual; the window follows the pointer on top of it.
class DragWindowDelegate
{
public:
    virtual ~DragWindowDelegate() = default;
    virtual void dragMoved(const base::PointF& location) = 0;
    // The surface under the pointer, and whether it takes the drop; null when
    // the drag is over nothing.
    virtual void dragTargetChanged(compositor::Surface* target, bool accepted) = 0;
    // `released` is a drop, whether or not a target took the data; otherwise
    // the drag was cancelled, by Escape or by its source or origin going away.
    virtual void dragEnded(bool released) = 0;
};

class DragController : public compositor::SurfaceObserver
{
public:
    explicit DragController(Seat* seat);
    ~DragController() override;

    bool active() const { return m_origin != nullptr; }

    // Driven by XDND messages instead of the pointer: the client that started
    // the drag keeps its implicit grab, and input goes on reaching it.
    bool isExternal() const { return m_external; }

    bool grabsInput() const { return active() && !m_external; }

    bool busy() const { return active() || m_finishing; }

    compositor::Surface* origin() const { return m_origin; }

    compositor::Surface* target() const { return m_target; }

    // Over a target whose offer would take a drop now.
    bool targetAcceptsDrop() const
    {
        const DragOffer* offer = selectedOffer();
        return m_target && offer && offer->canDrop();
    }

    bool hasSource() const { return bool(m_source); }

    const std::shared_ptr<clipboard::DataSource>& source() const { return m_source; }

    // For the running drag only, the icon and its subsurfaces included.
    void setIconOpacity(float opacity);

    // For the running drag only: over nothing that takes the drop it shows
    // the default pointer rather than no-drop.
    void setPlainIdleCursor(bool plain);

    // For the running drag only: ended with nothing taking the data, by a
    // cancel or a release, its icon slides back to where the drag started,
    // as ash's cancel animation does.
    void setReturnsHome(bool returns);
    // Where the pointer position goes back to; where the drag started otherwise.
    void setHome(const base::PointF& home);

    // The ended drag's icon is on its way back; returned() follows.
    bool isReturning() const { return m_returning; }

    // A returned icon waits where it arrived, so whoever draws it there next
    // can take over without a frame showing neither; this lets it go, and
    // ends a return still on its way.
    void releaseReturn();

    void start(compositor::Surface* origin, compositor::Surface* icon,
               std::shared_ptr<clipboard::DataSource> source, uint32_t actions, ButtonFlags button);
    // A drag that follows touch point `touchId` from `location` and drops where
    // it is lifted, as ash drags with a touch source. The clients that have the
    // point are sent a cancel.
    void startTouch(compositor::Surface* origin, compositor::Surface* icon,
                    std::shared_ptr<clipboard::DataSource> source, uint32_t actions,
                    int32_t touchId, const base::PointF& location);
    // The touch point a drag follows; empty for a pointer or external drag.
    std::optional<int32_t> touchId() const { return m_touchId; }
    // An X11 drag seen through the XDND proxy. motion() follows XdndPosition,
    // and drop() or cancel() end it.
    void startExternal(compositor::Surface* origin, std::shared_ptr<clipboard::DataSource> source,
                       uint32_t actions);
    std::shared_ptr<DragOffer> createOffer(bool legacy);
    void motion(Timestamp timestamp, const base::PointF& location);
    void button(Timestamp timestamp, uint32_t button, bool pressed);
    bool key(uint32_t code, bool pressed);
    // Drops on the selected offer, or cancels when nothing can take the drop.
    void drop();
    void cancel();
    uint32_t modifierAction() const;
    // For the running drag only; it is forgotten when the drag ends. A drag
    // with a window drops anywhere: the source learns of the drop even when no
    // target accepts it, which is how it tells a drop from Escape.
    void setWindowDelegate(DragWindowDelegate* delegate);

    DragWindowDelegate* windowDelegate() const { return m_windowDelegate; }

    void onSurfaceDestroying(compositor::Surface* surface) override;
    void onSurfaceCommit(compositor::Surface* surface) override;

    base::WeakPtr<DragController> weakPtr() { return m_weakFactory.getWeakPtr(); }

    base::Signal<> started;
    base::Signal<compositor::Surface*, const base::PointF&> entered;
    base::Signal<Timestamp, const base::PointF&> moved;
    // Wherever the drag goes, over a target or not; `moved` is target-local.
    base::Signal<const base::PointF&> locationChanged;
    base::Signal<> left;
    // The drag was let go, before the source hears how the drop went.
    base::Signal<> released;
    // The icon of a drag that returns home is back, or had none to slide.
    base::Signal<> returned;
    base::Signal<DragOffer*> dropped;
    base::Signal<> ended;

private:
    bool begin(compositor::Surface* origin, compositor::Surface* icon,
               std::shared_ptr<clipboard::DataSource> source, uint32_t actions,
               const base::PointF& location);
    void leave();
    DragOffer* selectedOffer() const { return m_selected.lock().get(); }
    // Cancels the transfer; `released` tells the window delegate it was a drop.
    void end(bool released);
    void detach();
    void updateIcon();
    void updateFeedback(DragOffer* offer);
    void setCursor(CursorShape shape);
    // The cursor over nothing that takes the drop.
    CursorShape idleCursor() const;
    void notifyWindowTarget();
    // The icon's content, moved to a node of its own before the icon goes.
    std::unique_ptr<scene::SurfaceNode> takeIconForReturn() const;
    void startReturn(std::unique_ptr<scene::SurfaceNode> node);
    void advanceReturn(double frameTimeMs);
    void finishReturn();
    Seat* m_seat;
    compositor::Surface* m_origin = nullptr;
    compositor::Surface* m_icon = nullptr;
    compositor::Surface* m_target = nullptr;
    base::PointF m_location;
    std::shared_ptr<clipboard::DataSource> m_source;
    std::vector<std::weak_ptr<DragOffer>> m_offers;
    std::weak_ptr<DragOffer> m_selected;
    uint32_t m_actions = 0;
    float m_iconOpacity = 1.0f;
    bool m_plainIdleCursor = false;
    bool m_returnsHome = false;
    base::PointF m_home;
    bool m_returning = false;
    scene::SurfaceNode* m_returnNode = nullptr;
    base::Point m_returnFrom;
    base::Point m_returnTo;
    float m_returnOpacity = 1.0f;
    double m_returnStarted = -1;
    base::Subscription m_returnFrames;
    ButtonFlags m_button = NoButton;
    std::optional<int32_t> m_touchId;
    DragWindowDelegate* m_windowDelegate = nullptr;
    bool m_finishing = false;
    bool m_external = false;
    uint64_t m_generation = 0;
    base::Subscription m_sourceConnection;
    // Each offer's feedbackChanged, dropped once the offer is gone.
    std::vector<std::pair<std::weak_ptr<DragOffer>, base::Subscription>> m_offerConnections;
    base::WeakPtrFactory<DragController> m_weakFactory {this};
};

} // namespace Lingmo::Input
