// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_BACKEND_DRM_DRM_OUTPUT_H_
#define LINGMO_BACKEND_DRM_DRM_OUTPUT_H_

#include <chrono>
#include <ctime>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>
#include <xf86drmMode.h>

#include "backend/drm/drm_commit_coordinator.h"
#include "base/geometry.h"
#include "base/signal.h"
#include "base/timer.h"
#include "base/weak_ptr.h"
#include "compositor/output.h"
#include "core/main_loop.h"
#include "backend/drm/drm_plane.h"
#include "backend/drm/drm_properties.h"
#include "backend/drm/presentation_tracker.h"
#include "backend/drm/scanout_controller.h"
#include "render/cursor_sink.h"
#include "render/gpu_timer.h"
#include "render/output_render_context.h"
#include "render/output_scheduler.h"
#include "render/texture.h"
#include "scene/overlay_candidate.h"
namespace scene = Lingmo::Scene;

struct gbm_bo;
struct gbm_surface;

namespace Lingmo::Render {

class EglRenderer;
class EglRenderTarget;
class Renderer;
} // namespace Lingmo::Render

namespace Lingmo::Backend {

class DrmBackend;
class DrmCursor;
class DrmDeviceApi;

// One connector driven by one CRTC, compositing onto the primary plane and,
// where the hardware allows it, scanning client buffers out of overlay planes
// and the cursor plane directly.
class DrmOutput : public render::CursorSink
{
public:
    struct Config {
        uint32_t connectorId = 0;
        uint32_t crtcId = 0;
        // Index into drmModeRes, which is what possible_crtcs refers to.
        int crtcIndex = -1;
        uint32_t planeId = 0;
        drmModeModeInfo mode = {};
        std::vector<drmModeModeInfo> modes;
        std::string name;
        // The EDID blob's bytes.
        std::string edid;
        std::string portPath;
        base::Size physicalSizeMm;
        int subpixel = DRM_MODE_SUBPIXEL_UNKNOWN;
        DrmPropertyMap connectorProperties;
        DrmPropertyMap crtcProperties;
        DrmPropertyMap planeProperties;
        // Every plane on the device, in ascending id.
        std::vector<DrmPlane> planes;
    };

    // Property names an atomic commit cannot do without.
    static std::vector<std::string> requiredConnectorProperties();
    static std::vector<std::string> requiredCrtcProperties();
    static std::vector<std::string> requiredPlaneProperties();

    DrmOutput(DrmBackend *backend, Config config);
    DrmOutput(DrmDeviceApi *device, Config config, std::function<bool()> sessionActive,
              bool monotonicClock,
              std::function<render::OutputScheduler::Time()> now =
                      render::OutputScheduler::Clock::now);
    ~DrmOutput() override;

    struct CompositedBuffer {
        CompositedBuffer() = default;
        CompositedBuffer(const CompositedBuffer &) = delete;
        CompositedBuffer &operator=(const CompositedBuffer &) = delete;
        PlaneContent primary;
        std::function<void()> release;

        ~CompositedBuffer()
        {
            if (release)
                release();
        }
    };

    using AcquireBuffer = std::function<std::unique_ptr<CompositedBuffer>()>;
    using CreateTarget = std::function<std::unique_ptr<render::EglRenderTarget>(base::Size)>;
    bool init(render::EglRenderer *renderer, std::unique_ptr<render::EglRenderTarget> target,
              AcquireBuffer acquire, CreateTarget createTarget = {});

    uint32_t connectorId() const { return m_config.connectorId; }

    uint32_t crtcId() const { return m_config.crtcId; }

    // Where this CRTC's cursor plane showed an image when the device was probed,
    // in CRTC pixels: at startup, the cursor the previous DRM master left.
    std::optional<base::Point> probedCursor() const;

    const std::string &edid() const { return m_config.edid; }

    const std::string &portPath() const
    {
        return m_config.portPath.empty() ? m_config.name : m_config.portPath;
    }

    // `keepRoute`: the output replacing this one drives the same connector from
    // the same CRTC, so the route stays lit on its last frame until the new
    // output's first commit replaces it, unless that frame shows a client
    // buffer, which is only released once KMS has stopped reading it.
    bool retireForReprobe(bool connectorExists, bool keepRoute = false);

    const std::string &name() const { return m_config.name; }

    std::string publicName() const;

    DrmBackend *backend() const { return m_backend; }

    base::Size size() const;

    // DRM fourcc -> the modifiers the primary plane accepts for it; an empty
    // list means the plane only has a legacy format list. What linux-dmabuf
    // advertises as scanout-able.
    std::map<uint32_t, std::vector<uint64_t>> primaryPlaneFormats() const;

    base::Size physicalSizeMm() const { return m_config.physicalSizeMm; }

    compositor::Output::Subpixel subpixel() const;

    double refreshRate() const;

    // The rate frames are scheduled and reported at: the mode's refresh, or
    // 60 Hz once flips show the device has no vblank to back that refresh.
    double pacingRate() const;

    const drmModeModeInfo &mode() const { return m_config.mode; }

    const std::vector<drmModeModeInfo> &modes() const { return m_config.modes; }

    // Creates the GBM surface and the renderer's target.
    bool init(render::EglRenderer *renderer);

    // Draws the first frame and performs the initial atomic modeset.
    bool enable();

    // Stops wakeups. Retains potentially scanned-out BOs until KMS retires them.
    void disable();

    bool isEnabled() const { return m_enabled; }

    bool isBlanked() const { return m_blanked; }

    bool requiresSoftwareBlanking() const;

    void setLinearBuffers(bool linear) { m_linearBuffers = linear; }
    // The GBM surface frames for this output are rendered into.
    gbm_surface *createSurface(const base::Size &size) const;

    struct Configuration {
        DrmOutput *output;
        drmModeModeInfo mode;
        bool enabled;
        bool blanked = false;
    };

    static bool configure(const std::vector<Configuration> &configuration, bool testOnly,
                          DrmCommitStatus *status = nullptr, std::string *diagnostic = nullptr,
                          DrmDeviceApi **failedDevice = nullptr);

    // Painting is a callback so DRM/KMS stays independent of the scene.
    void setRenderCallback(std::function<void(render::Renderer &, uint64_t)> callback);

    // Per-frame work that has to happen whether or not the scene is composited:
    // texture uploads and frame-callback accounting. Runs with the GL context
    // current, before the scene is painted.
    void setPrepareCallback(std::function<void(render::Renderer &, uint64_t)> callback);

    // Called as each scheduled frame begins, with its frame time in fractional
    // milliseconds, so what steps on frames steps whether or not this output
    // draws. Returns whether the output has anything to draw; a frame with
    // nothing is skipped rather than composited and flipped unchanged.
    void setBeginFrameCallback(std::function<bool(double)> callback);

    // Asked once per frame for the content that could go on hardware planes.
    // Absent, or returning an empty plan, means everything is composited.
    void setScanoutPlanner(std::function<scene::OverlayPlan(const base::Size &, size_t)> planner);

    // Paints `context.logicalViewport` into a texture outside the frame cycle.
    // Nothing is presented; the next frame repaints in full.
    std::unique_ptr<render::Texture>
    renderOffscreen(const render::OutputRenderContext &context,
                    const std::function<void(render::Renderer &)> &paint);

    // Client buffers may be scanned out directly instead of composited.
    // Enabled by default; tests and troubleshooting turn it off.
    void setDirectScanoutEnabled(bool enabled);

    // While set, a modeset that carries over the CRTC's current frame also keeps
    // a copy of it, for the scene to hold on screen until the session is ready.
    void setKeepCarriedFrame(bool keep) { m_keepCarriedFrame = keep; }

    // The copy the last such modeset kept, if any; the caller owns it.
    std::unique_ptr<render::Texture> takeCarriedFrame() { return std::move(m_carriedFrame); }

    // The login screen is blocked from its first frame but has no session of its
    // own on screen to protect, so it carries over the frame it starts on too.
    void setCarryOverWhileSecure(bool carry) { m_carryOverWhileSecure = carry; }

    // The session is blocked. The scanout planner is not consulted, and every
    // commit explicitly turns off every plane but the composited primary, so no
    // application buffer from the previous session frame can still be in the
    // KMS state. Set before the transition's first frame, not after it.
    void setSecureScanout(bool secure);

    // A session transition began: frames drawn before it, on screen or still on
    // their way, show where it starts and must not complete it.
    void withdrawSecurePresentation();

    // Whether a frame drawn at the given frame time shows the session's lock
    // transition over; one drawn mid-fade is not secure, whatever its planes.
    void setSecureContentCheck(std::function<bool(double)> check)
    {
        m_secureContentCheck = std::move(check);
    }

    // Whether the frame now on screen was committed in secure mode with nothing
    // but the compositor's own framebuffer programmed. The session's
    // presentation barrier waits for this, so "locked" cannot be reported while
    // a previous-session plane is still live. A dark output is secure by
    // definition and never holds the barrier open.
    bool lastPresentedFrameWasSecure() const;

    // Whether `planes` is the compositor's own framebuffer and nothing else.
    // The definition of "no application plane is live", in one place.
    static bool isSecureConfiguration(const std::vector<PlaneContent> &planes);

    // Whether this output's CRTC can tint what it shows; see
    // ScanoutController::supportsColorTransform().
    bool supportsColorTransform() const;

    // Whether the tint is blended over composited frames, the CRTC having none.
    bool blendsColorTransform() const
    {
        return !m_colorTransform.isIdentity() && !supportsColorTransform();
    }

    // Tints everything this output shows from its next frame on, through KMS,
    // so neither composition nor direct scanout changes. A frame with nothing
    // new to draw still flips to carry it; a dark output takes it when it wakes.
    // A CRTC with neither CTM nor gamma LUT gets it blended over each
    // composited frame instead, and on its cursor plane's image.
    void setColorTransform(const render::ColorTransform &transform);

    // render::CursorSink: the KMS cursor plane, when the device has one.
    bool setCursorImage(const render::CursorImage &image) override;
    void hideCursor() override;
    void moveCursor(const base::PointF &position) override;

    // Frames whose primary plane showed a client buffer rather than the
    // composited scene.
    uint64_t directScanoutFrames() const { return m_directScanoutFrames; }

    uint64_t overlayFrames() const { return m_overlayFrames; }

    using Reason = render::OutputScheduler::Reason;
    void scheduleFrame(Reason reason = Reason::Scene);
    // Draws a frame even with nothing changed, to wake an idle GPU before frames
    // that have to make their slots.
    void warmUp();
    // Answers whether the frame requested next redraws the whole output, for
    // the scheduler to begin it early enough.
    void setFullRepaintQuery(std::function<bool()> query) { m_fullRepaintQuery = std::move(query); }

    base::WeakPtr<DrmOutput> weakPtr() { return m_weakFactory.getWeakPtr(); }

    struct FlipEvent {
        base::WeakPtr<DrmOutput> output;
        uint64_t frame;
        bool delivered = false;
    };

    void setEventRetirementHandler(std::function<void(std::unique_ptr<FlipEvent>)> handler)
    {
        m_retireEvent = std::move(handler);
    }

    // How long flips may go without a kernel event before the output stops
    // waiting for the oldest one.
    void setFlipTimeout(std::chrono::milliseconds timeout) { m_flipTimeout.setInterval(timeout); }

    static void dispatchPageFlip(FlipEvent event, uint32_t seconds, uint32_t microseconds,
                                 uint32_t sequence);
    void onPageFlip(uint64_t frame, uint32_t seconds, uint32_t microseconds, uint32_t sequence);
    std::shared_ptr<render::PresentationOutput> presentationOutput;
    std::function<void(const render::PresentationSubmission &)> presentationSubmitted;
    void reportStats() const;
    // The same counters as reportStats() logs, for a debug interface to read.
    std::string statsLine() const;

    // The frame was composed and handed to KMS, committed or queued behind a
    // pending flip: viz's DrawAndSwap, where it acks the frames it drew.
    // The second value is the refresh it was drawn for, monotonic nanoseconds.
    base::Signal<uint64_t, uint64_t> frameDrawn;
    // Matched normal submission completed, with its time in milliseconds and
    // its generation. Scheduling uses a separate monotonic nominal clock.
    base::Signal<uint32_t, uint64_t> framePresented;
    // The generation and when it reached the screen, monotonic nanoseconds.
    base::Signal<uint64_t, uint64_t> imagePresented;
    base::Signal<uint64_t> frameDiscarded;
    // The last thing the destructor does; the output still answers.
    base::Signal<> destroyed;

private:
    // The planes and the GBM buffer one committed frame is made of.
    struct SubmittedFrame {
        render::PresentationSubmission presentation;
        std::vector<PlaneContent> planes;
        // The composited buffer on the primary plane, or null when the frame
        // scanned a client buffer out directly.
        std::unique_ptr<CompositedBuffer> composited;
        // The CRTC's OUT_FENCE for this commit: it signals when the commit
        // reaches the screen, which is when the frame it replaces stops being
        // read. That makes it the release fence for the previous frame.
        render::SyncFence releaseFence;
        // When the GPU finished composing the primary plane's buffer: a
        // timestamp query, or a fence where the GL has none.
        render::GpuTimer::Mark drawnMark;
        render::SyncFence drawnFence;
        // Committed in secure mode with no application plane programmed.
        bool secure = false;
    };

    // A frame rendered in its own refresh slot while the previous one was
    // still on its way to the screen. KMS holds one flip per CRTC, so it is
    // committed the moment that flip lands, the way ui::GbmSurfaceless keeps
    // unsubmitted frames and schedules the next page flip from OnPresentation.
    struct QueuedFrame {
        uint64_t frame = 0;
        SubmittedFrame submitted;
    };

    std::unique_ptr<CompositedBuffer> acquireComposited(gbm_surface *surface);
    void commitQueued();
    // Forgets every frame rendered but not presented, returning their tokens
    // for frameDiscarded once the caller's state is consistent.
    std::vector<uint64_t> dropOutstandingFrames();
    bool renderFrame(uint64_t frame, bool modeset);
    // One attempt at a frame. `allowPromotion` false forces plain composition,
    // which is the fallback when a promoted configuration fails to commit.
    bool renderAttempt(uint64_t frame, bool modeset, bool allowPromotion, bool &prepared,
                       bool retry = false);
    void wakeup();
    void armWakeup();
    // The refresh the frame begun last is drawn for, in monotonic nanoseconds.
    uint64_t aimedNs() const;
    // What follows a flip, landed or given up on: the pending buffers go on
    // screen, the queued frame is committed and the scheduler moves on.
    void finishFlip(uint64_t frame, uint32_t timeMs,
                    std::optional<render::OutputScheduler::Time> presented);
    void onFlipTimeout();
    // The kernel keeps a committed event's address as its cookie, so it must
    // outlive this output.
    void retireEvent(std::unique_ptr<FlipEvent> event);
    bool commit(const std::vector<PlaneContent> &planes, bool modeset, uint64_t frame = 0);
    // Upstream: ScreenManager::GetModesetPlanes and FillModesetBuffer, which
    // copy the CRTC's current framebuffer into the modeset buffer when the
    // sizes match and clear it black otherwise.
    bool fillModesetBuffer(render::EglRenderTarget *target, const base::Size &size, bool copyScanout);
    bool createModeBlob();
    void destroyModeBlob();
    void releaseScanoutBuffers();
    void clearPromotions();
    // Hands the buffers of a retired frame back, with the fence that says when
    // the display actually stopped reading them.
    void retire(SubmittedFrame &frame, const render::SyncFence &fence);

    DrmBackend *m_backend = nullptr;
    Config m_config;
    DrmDeviceApi *m_device = nullptr;
    std::function<bool()> m_sessionActive;
    bool m_monotonicClock = false;
    AcquireBuffer m_acquireBuffer;
    CreateTarget m_createTarget;

    render::EglRenderer *m_renderer = nullptr;
    gbm_surface *m_gbmSurface = nullptr;
    std::unique_ptr<render::EglRenderTarget> m_target;

    uint32_t m_modeBlobId = 0;

    // Scanning out now; released only once KMS has moved on.
    gbm_bo *m_scanoutBuffer = nullptr;
    // Committed but not yet flipped to.
    gbm_bo *m_pendingBuffer = nullptr;

    std::function<void(render::Renderer &, uint64_t)> m_renderCallback;
    std::function<void(render::Renderer &, uint64_t)> m_prepareCallback;
    std::function<bool(double)> m_beginFrameCallback;
    std::function<bool()> m_fullRepaintQuery;
    // The next frame draws whatever the scene says: after a resume, a modeset
    // or a change in how frames are committed, the screen needs a new one.
    bool m_forceFrame = true;
    std::function<scene::OverlayPlan(const base::Size &, size_t)> m_scanoutPlanner;

    std::unique_ptr<ScanoutController> m_scanout;
    std::unique_ptr<DrmCursor> m_cursor;
    // Nodes this frame took off the compositor's hands; cleared once the frame
    // has been committed, so a later fallback composites them again.
    std::vector<scene::SurfaceNode *> m_promoted;
    PresentationTracker m_presentations;
    SubmittedFrame m_pendingFrame;
    SubmittedFrame m_scanoutFrame;
    std::optional<QueuedFrame> m_queuedFrame;
    // The out-fence of the commit just issued, moved into the frame it belongs
    // to once that frame's bookkeeping is built.
    render::SyncFence m_lastReleaseFence;
    bool m_directScanoutEnabled = true;
    // Kept for the scanout controller init() creates.
    render::ColorTransform m_colorTransform;
    // Full repaints still owed after a blended tint ends, one per buffer the
    // GBM surface may still hold tinted.
    int m_untintedRepaints = 0;
    bool m_secureScanout = false;
    std::function<bool(double)> m_secureContentCheck;
    bool m_keepCarriedFrame = false;
    bool m_carryOverWhileSecure = false;
    std::unique_ptr<render::Texture> m_carriedFrame;
    bool m_lastPresentedSecure = false;
    uint64_t m_directScanoutFrames = 0, m_overlayFrames = 0;

    bool m_enabled = false;
    bool m_blanked = false;
    bool m_linearBuffers = false;
    bool m_transferUnavailable = false;
    int m_transferRoute = -1;
    bool m_retiring = false;
    bool m_configuring = false;
    std::function<render::OutputScheduler::Time()> m_now;
    render::OutputScheduler m_scheduler;
    base::Timer m_wakeup {core::mainLoop()};
    render::OutputScheduler::Time m_armedDeadline {};
    base::Timer m_flipTimeout {core::mainLoop()};
    std::map<uint64_t, std::unique_ptr<FlipEvent>> m_events;
    std::function<void(std::unique_ptr<FlipEvent>)> m_retireEvent;
    uint64_t m_bufferFrame = 0;
    // Frames whose wl_surface.frame callbacks are still owed, oldest first:
    // committed, queued, or a modeset's; gone once presented or discarded.
    std::vector<uint64_t> m_outstanding;
    uint64_t m_frameCount = 0, m_commits = 0, m_flipCommits = 0, m_completions = 0;
    std::clock_t m_cpuStart = std::clock();
    uint32_t m_lastSequence = 0, m_lastTimeMs = 0;
    bool m_unsynced = false;
    std::chrono::steady_clock::time_point m_statsStart = std::chrono::steady_clock::now();
    // Last, so flip events lose this output before anything else of it goes.
    base::WeakPtrFactory<DrmOutput> m_weakFactory {this};
};

} // namespace Lingmo::Backend

#endif // LINGMO_BACKEND_DRM_DRM_OUTPUT_H_
