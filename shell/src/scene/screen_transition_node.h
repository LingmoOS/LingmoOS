// Copyright 2015 The Chromium Authors
// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ash/rotator/screen_rotation_animator.cc
//           chromium/src/ash/rotator/screen_rotation_animation.cc
//           chromium/src/ui/gfx/geometry/cubic_bezier.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#ifndef LINGMO_SCENE_SCREEN_TRANSITION_NODE_H_
#define LINGMO_SCENE_SCREEN_TRANSITION_NODE_H_

#include <functional>
#include <map>
#include <memory>

#include <string>

#include "scene/node.h"
#include "base/geometry.h"
#include "base/transform.h"

namespace Lingmo::Scene {

// ScreenRotationAnimator's smooth animation for every output that rotates. The
// outgoing frame holds the screen until the new content is ready, which upstream
// gets from an extended compositor lock; then a copy of the new content turns in
// over a black mask while the outgoing copy turns on and fades out. Unturned, it
// is the startup cover: another DRM master's last frame, held until the session
// is ready and then faded out.
class ScreenTransitionNode : public Node
{
public:
    struct Output {
        std::unique_ptr<render::Texture> outgoing;
        // The outgoing frame's turn in the new orientation, in base::AffineTransform
        // degrees: 90, -90 or -180.
        int degrees = 0;
    };

    using Outputs = std::map<std::string, Output>;
    using Paint = std::function<void(render::Renderer &)>;

    // Upstream kRotationDegrees and kRotationDurationInMs.
    static constexpr int kRotationDegrees = 20;
    static constexpr int kRotationDurationMs = 250;

    explicit ScreenTransitionNode(std::function<int64_t()> clock = {});

    // `outputs` are keyed by OutputRenderContext::outputId. `paintIncoming`
    // paints the live content the node covers. The hold ends at the first frame
    // where `ready` holds, or once `timeoutMs` has passed; the caller must
    // request a frame at the deadline.
    void start(Outputs outputs, Paint paintIncoming, std::function<bool()> ready, int timeoutMs);

    // Upstream StopAnimating: a running animation ends at once, while an
    // outgoing frame that is still held stays for the next rotation to capture.
    void stopAnimating();

    bool isActive() const { return !m_outputs.empty(); }

    // Whether the held phase is over and the animation is running.
    bool isAnimating() const { return m_animationStarted >= 0; }

    void render(render::Renderer &renderer) const override;

    Content content() const override { return isActive() ? Content::Translucent : Content::None; }

    bool paintsSubtree() const override { return isActive(); }

    bool acceptsInput(const base::PointF &) const override { return false; }

    void releaseRenderResources() override;

    // gfx::Tween::FAST_OUT_LINEAR_IN.
    static double fastOutLinearIn(double t);

    struct RotationSample {
        double oldDegrees;
        float oldOpacity;
        double newDegrees;
    };

    // Both layers of the animation at untweened `progress`.
    static RotationSample sampleRotation(int degrees, double progress);

    // A `source`-sized frame turned by `degrees`, scaled to cover `viewport`.
    static base::RectF coverRect(const base::SizeF &source, int degrees, const base::RectF &viewport);

    static base::AffineTransform aboutCenter(const base::PointF &center, double degrees);

private:
    struct OutputState {
        Output output;
        std::unique_ptr<render::Texture> incoming;
        bool incomingFailed = false;
    };

    void finish() const;

    mutable std::map<std::string, OutputState> m_outputs;
    mutable Paint m_paintIncoming;
    mutable std::function<bool()> m_ready;
    std::function<int64_t()> m_now;
    int64_t m_deadline = 0;
    mutable int64_t m_animationStarted = -1;
};

} // namespace Lingmo::Scene

#endif // LINGMO_SCENE_SCREEN_TRANSITION_NODE_H_
