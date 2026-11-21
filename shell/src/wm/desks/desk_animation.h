// Copyright 2020 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/wm/desks/desk_animation_base.h and
//           desk_animation_impl.h
// Revision: 8abbac30b9cf2fe976422c58c000c94685648b65
//
// Adapted for Lingmo: one DeskSwitchNode stands in for the per-root
// animators. The starting frames are taken synchronously, so phase (1) ends
// inside launch(); the ending frames come from the next frame each output
// draws. Metrics, haptics, overview and split view are not carried over.

#ifndef LINGMO_WM_DESKS_DESK_ANIMATION_H_
#define LINGMO_WM_DESKS_DESK_ANIMATION_H_

#include "base/timer.h"
#include "base/weak_ptr.h"
#include "wm/desks/desks_controller.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::WM {

class Desk;

class DeskAnimationBase
{
public:
    // Long enough for every output to draw a frame, even one slowed by a
    // client; past it the switch happens without the animation.
    static constexpr int kEndingScreenshotTimeoutMs = 500;

    DeskAnimationBase(DesksController *controller, int endingDesk,
                      bool isContinuousGestureAnimation);
    virtual ~DeskAnimationBase();

    int endingDeskIndex() const { return m_endingDesk; }
    // While fingers pull the strip, the desk the swipe was chosen to stay on
    // until they lift; -1 otherwise.
    int chosenDeskIndex() const { return m_chosenDesk; }

    // Phase (1), and with it the ending desk's activation.
    void launch();

    // Lands where the animation was heading without animating the rest; the
    // controller lets go of it.
    void finishNow() { activateTargetDeskWithoutAnimation(); }

    // Upstream Replace: takes the ending desk one further along. False when
    // the animation cannot, and the caller has to start another.
    virtual bool replace(bool movingLeft, DesksSwitchSource source) { return false; }
    // See DeskSwitchNode::updateSwipeAnimation and endSwipeAnimation.
    virtual bool updateSwipeAnimation(double deskDelta, double timeMs) { return false; }
    virtual bool endSwipeAnimation(double timeMs) { return false; }

protected:
    // Starts the node on the starting frames; false when there are none.
    virtual bool startNode() = 0;
    virtual void onStartingDeskScreenshotTakenInternal(int endingDesk) = 0;
    virtual void onDeskSwitchAnimationFinishedInternal() = 0;

    void takeEndingScreenshot(int desk);
    void stopScreenshotTimeout() { m_screenshotTimeout.stop(); }
    void onEndingDeskScreenshotTaken();
    void onDeskSwitchAnimationFinished();
    void activateDeskDuringAnimation(Desk *desk, bool updateWindowActivation);
    // Destroys this animation.
    void activateTargetDeskWithoutAnimation();

    DesksController *m_controller = nullptr;
    scene::DeskSwitchNode *m_node = nullptr;
    int m_endingDesk = -1;
    int m_chosenDesk = -1;
    // The fingers move the strip, and let it go into its own slide.
    const bool m_isContinuousGestureAnimation;

private:
    // Hands the node back empty for the next animation.
    void releaseNode();

    base::Timer m_screenshotTimeout;
    bool m_finished = false;
    base::WeakPtrFactory<DeskAnimationBase> m_weakFactory {this};
};

class DeskActivationAnimation : public DeskAnimationBase
{
public:
    DeskActivationAnimation(DesksController *controller, int startingDesk, int endingDesk,
                            DesksSwitchSource source, bool updateWindowActivation);

    bool replace(bool movingLeft, DesksSwitchSource source) override;
    bool updateSwipeAnimation(double deskDelta, double timeMs) override;
    bool endSwipeAnimation(double timeMs) override;

protected:
    bool startNode() override;
    void onStartingDeskScreenshotTakenInternal(int endingDesk) override;
    void onDeskSwitchAnimationFinishedInternal() override;

private:
    void prepareDeskForScreenshot(int index);

    const DesksSwitchSource m_source;
    const bool m_updateWindowActivation;
    const int m_startingDesk;
    const scene::DeskSwitchNode::Type m_type;
};

class DeskRemovalAnimation : public DeskAnimationBase
{
public:
    DeskRemovalAnimation(DesksController *controller, int deskToRemove, int deskToActivate);

protected:
    bool startNode() override;
    void onStartingDeskScreenshotTakenInternal(int endingDesk) override;
    void onDeskSwitchAnimationFinishedInternal() override;

private:
    const int m_deskToRemove;
};

} // namespace Lingmo::WM

#endif // LINGMO_WM_DESKS_DESK_ANIMATION_H_
