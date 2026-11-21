// Copyright 2015 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/display.h
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.
//
// Renamed from Display, which in Lingmo would be confused with a physical
// output, and reduced to the factories Lingmo needs.

#ifndef LINGMO_COMPOSITOR_COMPOSITOR_H_
#define LINGMO_COMPOSITOR_COMPOSITOR_H_

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/geometry.h"

#include "base/signal.h"

#include "compositor/output_scale.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Render {
class DmaBufDevice;
class PresentationSubmission;
class Renderer;
struct DmaBufAttributes;
} // namespace Lingmo::Render

namespace Lingmo::Scene {
class Node;
class SurfaceNode;
} // namespace Lingmo::Scene

namespace Lingmo::Compositor {

class Buffer;
class ShmPool;
class Surface;

// Surface contents the compositor draws itself, such as the shell's internal
// windows. Produced on the renderer's context at the start of every output
// frame, before the surfaces' textures are updated, so their commits are part
// of the frame being composed.
class ContentProducer
{
public:
    // Whether a frame of the output showing `viewport` has content to produce.
    virtual bool hasPendingContent(const base::Rect &viewport) const = 0;
    virtual void produceContent(render::Renderer &renderer) = 0;

protected:
    virtual ~ContentProducer() = default;
};

// Creates the compositor-side objects the Wayland protocol layer hands out, and
// owns the relationship between committed surface state and presented frames.
// Deliberately not a place for protocol handling or for window management.
class Compositor
{
public:
    struct OutputView {
        std::string id;
        base::Rect bounds;
        OutputScale scale;
        // The wl_output whose composition carries this view, which is what a
        // render context names; `id` is the logical display's session id.
        std::string waylandName;

        friend bool operator==(const OutputView &, const OutputView &) = default;
    };

    // `surfaceContainer` is the scene node new surfaces are parented into. It
    // may be null in tests that only exercise protocol behavior.
    explicit Compositor(scene::Node *surfaceContainer);
    ~Compositor();

    // The wl_surface resource takes ownership of the returned Surface.
    std::unique_ptr<Surface> createSurface();

    // Takes ownership of `fd` in every case, including failure.
    std::shared_ptr<ShmPool> createShmPool(int fd, int32_t size);

    // The device linux-dmabuf buffers are imported through. Null when there is
    // no GPU renderer, in which case no linux-dmabuf global is advertised.
    void setDmaBufDevice(render::DmaBufDevice *device) { m_dmaBufDevice = device; }

    // Takes ownership of the plane fds in `attributes` in every case, including
    // failure. Returns nullptr when the import fails.
    std::shared_ptr<Buffer> createDmaBufBuffer(render::DmaBufAttributes attributes);

    const std::vector<Surface *> &surfaces() const { return m_surfaces; }

    OutputScale outputScale() const { return m_outputScale; }

    void setOutputScale(OutputScale scale);
    void setOutputViews(std::vector<OutputView> outputs);

    const std::vector<OutputView> &outputViews() const { return m_outputViews; }

    void refreshOutputScales();
    uint32_t preferredScaleFor(const Surface *surface) const;

    // Surfaces that moved, resized, showed or hid since the last call.
    std::vector<std::shared_ptr<Surface *>> takeMovedSurfaces();

    // Whether a frame of the output showing `viewport` has frame callbacks or
    // presentation feedback to deliver, pixels aside.
    bool awaitsFrame(const base::Rect &viewport) const;

    // The scene node new surfaces are parented into. Null in tests that only
    // exercise protocol behavior.
    scene::Node *surfaceContainer() const { return m_surfaceContainer; }

    // Where input method candidate popups float, above every window. Hit
    // testing the session searches it before the surface container.
    void setInputMethodContainer(scene::Node *container) { m_inputMethodContainer = container; }
    scene::Node *inputMethodContainer() const { return m_inputMethodContainer; }

    // The surface owning `node`, or nullptr. Used by input hit testing, which
    // walks the scene in stacking order.
    Surface *surfaceForNode(const scene::Node *node) const;

    // Null for none. The producer outlives the compositor or unsets itself.
    void setContentProducer(ContentProducer *producer) { m_contentProducer = producer; }
    ContentProducer *contentProducer() const { return m_contentProducer; }

    // Produces the compositor's own content, then uploads every surface whose
    // contents changed since the last frame.
    void updateTextures(render::Renderer &renderer);

    // Snapshot callbacks immediately before composition. Roll back on failed
    // submission or suspension; only the matching completion may finish them.
    void frameSubmitted(uint64_t generation = 0);
    void frameSubmittedForOutput(const base::Rect &viewport, uint64_t generation);
    // Discards the feedback an output's accepted attempt left out, for content
    // no other output can still show: hidden, wholly inside `viewport`, or on
    // no output. Content spanning outputs waits for one of them to present it,
    // or for its replacement.
    void abandonUnsubmittedPresentation(const base::Rect &viewport,
                                        const render::PresentationSubmission &submission);
    void frameDiscarded(uint64_t generation);
    // That frame was drawn and handed to the display: its callbacks are done.
    void frameDrawn(uint32_t timeMs, uint64_t generation = 0);

    // Called by Surface when committed state changed.
    void scheduleRepaint(bool decoration = false);

    // Surface lifecycle; not part of the public API.
    scene::SurfaceNode *createSurfaceNode(Surface *surface);
    void surfaceDestroyed(Surface *surface);

    base::Signal<bool> repaintNeeded;
    base::Signal<> outputScaleChanged;
    base::Signal<> outputViewsChanged;
    base::Signal<render::Renderer &> outputRendered;
    // An output is about to render the frame timed at `frameTimeMs`, on the
    // same clock renderer.frameTime uses. Animations that move scene nodes
    // rather than paint them advance here, before the frame is drawn.
    base::Signal<double> frameStarted;
    // Handed to the display, which is expected to show it at `aimedNs`, on the
    // monotonic clock.
    base::Signal<uint64_t, uint64_t> outputDrawn;
    base::Signal<uint64_t, uint64_t> outputPresented;
    base::Signal<uint64_t> outputDiscarded;

private:
    OutputScale m_outputScale;
    std::vector<OutputView> m_outputViews;
    scene::Node *m_surfaceContainer = nullptr;
    scene::Node *m_inputMethodContainer = nullptr;
    render::DmaBufDevice *m_dmaBufDevice = nullptr;
    ContentProducer *m_contentProducer = nullptr;
    std::vector<Surface *> m_surfaces;
    // Hit testing asks this for every node it passes.
    std::unordered_map<const scene::Node *, Surface *> m_surfaceByNode;
    // Cascade counter, so surfaces the window manager never places do not
    // sit on top of each other.
    int m_placementIndex = 0;
};

} // namespace Lingmo::Compositor

#endif // LINGMO_COMPOSITOR_COMPOSITOR_H_
