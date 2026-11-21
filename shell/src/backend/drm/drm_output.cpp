// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "backend/drm/drm_output.h"


#include <GLES2/gl2.h>
#include <drm_fourcc.h>
#include <gbm.h>
#include <xf86drm.h>

#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <optional>
#include <set>

#include "backend/drm/drm_backend.h"
#include "backend/drm/drm_cursor.h"
#include "backend/drm/drm_device_api.h"
#include "backend/drm/drm_framebuffer.h"
#include "backend/drm/drm_multi_device_planner.h"
#include "backend/drm/drm_selection.h"
#include "base/auto_reset.h"
#include "base/strings.h"
#include "backend/session/session.h"
#include "core/logging.h"
#include "render/egl_renderer.h"
#include "scene/surface_node.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Backend {

namespace {

// Mesa's GBM surfaces hold up to four colour buffers.
constexpr int kSwapBuffers = 4;

// A mode without timings has no rate; viz paces such a display at
// BeginFrameArgs::DefaultInterval(), 60 Hz. llround(inf) is undefined, and on
// amd64 gives a negative period.
render::OutputScheduler::Duration framePeriod(double hz)
{
    return std::chrono::nanoseconds(static_cast<int64_t>(std::llround(1e9 / (hz > 0 ? hz : 60.0))));
}

} // namespace

std::vector<std::string> DrmOutput::requiredConnectorProperties()
{
    return {"CRTC_ID"};
}

std::vector<std::string> DrmOutput::requiredCrtcProperties()
{
    return {"MODE_ID", "ACTIVE"};
}

std::vector<std::string> DrmOutput::requiredPlaneProperties()
{
    return {
            "FB_ID",  "CRTC_ID", "SRC_X",  "SRC_Y",  "SRC_W",
            "SRC_H",  "CRTC_X",  "CRTC_Y", "CRTC_W", "CRTC_H",
    };
}

DrmOutput::DrmOutput(DrmBackend *backend, Config config)
    : DrmOutput(
              backend->device(), std::move(config),
              [backend] {
                  return backend->session() && backend->session()->isActive();
              },
              backend->hasMonotonicPresentationClock())
{
    m_backend = backend;
}

DrmOutput::DrmOutput(DrmDeviceApi *device, Config config, std::function<bool()> sessionActive,
                     bool monotonicClock,
                     std::function<render::OutputScheduler::Time()> now)
    : m_config(std::move(config))
    , m_device(device)
    , m_sessionActive(std::move(sessionActive))
    , m_monotonicClock(monotonicClock)
    , m_now(std::move(now))
    , m_scheduler(framePeriod(refreshRate()), m_now)
{
    m_wakeup.setSingleShot(true);
    m_wakeup.onTimeout = [this] { wakeup(); };
    // Ozone waits for every flip event, because a stuck GPU process gets
    // restarted (PageFlipWatchdog). Nothing restarts this process, so an event
    // the driver loses would hold every later frame for the rest of the session.
    m_flipTimeout.setSingleShot(true);
    m_flipTimeout.setInterval(std::chrono::seconds(1));
    m_flipTimeout.onTimeout = [this] { onFlipTimeout(); };
    // ChromeOS's triple-buffered BufferQueue: one frame at KMS, one queued
    // behind it, and the GBM surface's spare buffer for the one being drawn.
    m_scheduler.setMaxPendingFrames(2);
}

DrmOutput::~DrmOutput()
{
    disable();
    for (auto &entry : m_events)
        retireEvent(std::move(entry.second));
    reportStats();
    m_cursor.reset();
    releaseScanoutBuffers(); // Existing final GBM teardown, never a timeout path.

    if (m_renderer)
        m_renderer->releaseTarget(m_target.get());
    m_target.reset();
    if (m_gbmSurface)
        gbm_surface_destroy(m_gbmSurface);
    destroyed();
}

std::string DrmOutput::publicName() const
{
    return m_backend && m_backend->usesForeignRenderer()
            ? std::filesystem::path(m_backend->devicePath()).filename().string() + '-'
                    + m_config.name
            : m_config.name;
}

std::unique_ptr<DrmOutput::CompositedBuffer> DrmOutput::acquireComposited(gbm_surface *surface)
{
    m_transferUnavailable = false;
    auto *bo = gbm_surface_lock_front_buffer(surface);
    if (!bo)
        return nullptr;
    DrmFramebuffer *framebuffer = nullptr;
    const base::Size size(gbm_bo_get_width(bo), gbm_bo_get_height(bo));
    auto route = chooseCrossGpuTransfer({!m_backend->usesForeignRenderer(), true, false, false,
                                         int64_t(size.width()) * size.height()});
    // Whatever the route, the framebuffer is kept on the bo: the surface's two
    // or three buffers are imported or allocated once, not on every frame.
    if (route == CrossGpuTransfer::Direct) {
        framebuffer = DrmFramebuffer::forBuffer(m_device, bo);
    } else {
        framebuffer = DrmFramebuffer::importForBuffer(m_device, bo);
        if (!framebuffer) {
            route = chooseCrossGpuTransfer({false, false, false,
                                            gbm_bo_get_format(bo) == DRM_FORMAT_XRGB8888,
                                            int64_t(size.width()) * size.height()});
            if (route == CrossGpuTransfer::CpuCopy) {
                glFinish();
                uint32_t stride = 0;
                void *mapData = nullptr;
                auto *pixels =
                        static_cast<uint8_t *>(gbm_bo_map(bo, 0, 0, size.width(), size.height(),
                                                          GBM_BO_TRANSFER_READ, &stride, &mapData));
                if (pixels) {
                    framebuffer = DrmFramebuffer::copyForBuffer(m_device, bo, pixels, stride);
                    gbm_bo_unmap(bo, mapData);
                }
            }
        }
    }
    if (!framebuffer) {
        m_transferUnavailable = m_backend->usesForeignRenderer();
        gbm_surface_release_buffer(surface, bo);
        LM_WARNING(lingmoDrm) << "No scanout transfer path for" << publicName()
                          << size;
        return nullptr;
    }
    if (m_transferRoute != int(route)) {
        m_transferRoute = int(route);
        LM_INFO(lingmoDrm) << "Scanout transfer for" << publicName()
                       << (route == CrossGpuTransfer::Direct                 ? "direct"
                                  : route == CrossGpuTransfer::DmaBufImport ? "DMA-BUF import"
                                                                            : "bounded CPU copy");
    }
    auto buffer = std::make_unique<CompositedBuffer>();
    buffer->primary = {m_config.planeId,        framebuffer->id(),     framebuffer->format(),
                       framebuffer->modifier(), base::Rect(size), base::Rect(size)};
    buffer->release = [surface, bo] {
        gbm_surface_release_buffer(surface, bo);
    };
    return buffer;
}

base::Size DrmOutput::size() const
{
    return base::Size(m_config.mode.hdisplay, m_config.mode.vdisplay);
}

compositor::Output::Subpixel DrmOutput::subpixel() const
{
    using S = compositor::Output::Subpixel;
    switch (m_config.subpixel) {
    case DRM_MODE_SUBPIXEL_NONE:
        return S::None;
    case DRM_MODE_SUBPIXEL_HORIZONTAL_RGB:
        return S::HorizontalRgb;
    case DRM_MODE_SUBPIXEL_HORIZONTAL_BGR:
        return S::HorizontalBgr;
    case DRM_MODE_SUBPIXEL_VERTICAL_RGB:
        return S::VerticalRgb;
    case DRM_MODE_SUBPIXEL_VERTICAL_BGR:
        return S::VerticalBgr;
    default:
        return S::Unknown;
    }
}

double DrmOutput::refreshRate() const
{
    return Lingmo::Backend::refreshRate(m_config.mode);
}

double DrmOutput::pacingRate() const
{
    return m_unsynced ? 60.0 : refreshRate();
}

void DrmOutput::setRenderCallback(std::function<void(render::Renderer &, uint64_t)> callback)
{
    m_renderCallback = std::move(callback);
}

void DrmOutput::setPrepareCallback(std::function<void(render::Renderer &, uint64_t)> callback)
{
    m_prepareCallback = std::move(callback);
}

void DrmOutput::setBeginFrameCallback(std::function<bool(double)> callback)
{
    m_beginFrameCallback = std::move(callback);
}

void DrmOutput::setScanoutPlanner(std::function<scene::OverlayPlan(const base::Size &, size_t)> planner)
{
    m_scanoutPlanner = std::move(planner);
}

std::unique_ptr<render::Texture>
DrmOutput::renderOffscreen(const render::OutputRenderContext &context,
                           const std::function<void(render::Renderer &)> &paint)
{
    if (!m_enabled || m_blanked || m_retiring || !m_renderer || !m_target)
        return {};
    m_renderer->setOutputContext(context);
    if (!m_renderer->beginFrame(m_target.get()))
        return {};
    std::unique_ptr<render::Texture> texture;
    const bool rendered = m_renderer->renderToTexture(texture, context.logicalViewport, [&] {
        base::AutoReset<bool> capturing(m_renderer->capturing, true);
        paint(*m_renderer);
    });
    m_renderer->cancelFrame();
    if (!rendered)
        texture.reset();
    return texture;
}

void DrmOutput::setDirectScanoutEnabled(bool enabled)
{
    if (m_directScanoutEnabled == enabled)
        return;
    m_directScanoutEnabled = enabled;
    // Leaving a client buffer on the primary plane after scanout is switched
    // off would freeze the screen on that buffer; ask for a composited frame.
    m_forceFrame = true;
    scheduleFrame(Reason::Scene);
}

bool DrmOutput::supportsColorTransform() const
{
    return m_scanout && m_scanout->supportsColorTransform();
}

void DrmOutput::setColorTransform(const render::ColorTransform &transform)
{
    const bool changed = transform != m_colorTransform;
    const bool blended = blendsColorTransform();
    m_colorTransform = transform;
    if (m_scanout && m_scanout->setColorTransform(transform)) {
        // Nothing on screen has to change for it, but only a commit carries it.
        if (m_enabled && m_sessionActive && m_sessionActive()) {
            m_forceFrame = true;
            scheduleFrame(Reason::Scene);
        }
        return;
    }
    if (!changed || supportsColorTransform())
        return;
    // Ash's composited colour matrix, for virtual GPUs. The previous frames'
    // buffers hold the old tint, so every frame repaints whole until each has
    // been drawn again.
    if (blended && !blendsColorTransform())
        m_untintedRepaints = kSwapBuffers;
    if (m_enabled && m_sessionActive && m_sessionActive()) {
        m_forceFrame = true;
        scheduleFrame(Reason::Scene);
    }
}

void DrmOutput::setSecureScanout(bool secure)
{
    if (m_secureScanout == secure)
        return;
    m_secureScanout = secure;
    // Whatever is on screen right now was composed under the old policy. Until
    // a frame committed under the new one has been presented, nothing may be
    // reported as securely presented.
    withdrawSecurePresentation();
}

void DrmOutput::withdrawSecurePresentation()
{
    m_lastPresentedSecure = false;
    m_scanoutFrame.secure = false;
    m_pendingFrame.secure = false;
    if (m_queuedFrame)
        m_queuedFrame->submitted.secure = false;
    m_forceFrame = true;
    scheduleFrame(Reason::Scene);
}

bool DrmOutput::lastPresentedFrameWasSecure() const
{
    // Nothing is on screen, so nothing of the session can be: a DPMS-off or
    // disabled output must not stall a lock waiting for a frame it will never
    // produce.
    if (!m_enabled || m_blanked)
        return true;
    return m_lastPresentedSecure;
}

bool DrmOutput::isSecureConfiguration(const std::vector<PlaneContent> &planes)
{
    return planes.size() == 1 && !planes.front().isDirectScanout();
}

gbm_surface *DrmOutput::createSurface(const base::Size &size) const
{
    const auto create = [&](bool linear) {
        return gbm_surface_create(m_backend->gbmDevice(), size.width(), size.height(),
                                  m_backend->format(),
                                  GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING
                                          | (linear ? GBM_BO_USE_LINEAR : 0));
    };
    // The render GPU would pick a layout for its own display engine, which
    // another device rarely imports; linear is the one PRIME layout every
    // scanout device takes, as mutter and wlroots fall back to for a second GPU.
    const bool foreign = m_backend->usesForeignRenderer();
    gbm_surface *surface = create(m_linearBuffers || foreign);
    if (!surface && foreign && !m_linearBuffers)
        surface = create(false);
    return surface;
}

bool DrmOutput::init(render::EglRenderer *renderer)
{
    m_renderer = renderer;

    m_gbmSurface = createSurface(size());
    if (!m_gbmSurface) {
        LM_CRITICAL(lingmoDrm) << "Failed to create a GBM surface for" << m_config.name;
        return false;
    }

    m_target = m_renderer->createTarget(m_gbmSurface, size());
    if (!m_target)
        return false;

    auto acquire = [this] {
        return acquireComposited(m_gbmSurface);
    };
    return init(renderer, std::move(m_target), std::move(acquire));
}

bool DrmOutput::init(render::EglRenderer *renderer, std::unique_ptr<render::EglRenderTarget> target,
                     AcquireBuffer acquire, CreateTarget createTarget)
{
    m_renderer = renderer;
    m_target = std::move(target);
    m_acquireBuffer = std::move(acquire);
    m_createTarget = std::move(createTarget);
    if (!m_renderer || !m_target || !m_acquireBuffer)
        return false;
    ScanoutController::Config scanoutConfig;
    scanoutConfig.crtcId = m_config.crtcId;
    scanoutConfig.crtcIndex = m_config.crtcIndex;
    scanoutConfig.connectorId = m_config.connectorId;
    scanoutConfig.crtcProperties = m_config.crtcProperties;
    scanoutConfig.connectorProperties = m_config.connectorProperties;
    scanoutConfig.planes = m_config.planes;
    scanoutConfig.modeSize = size();
    m_scanout = std::make_unique<ScanoutController>(m_device, std::move(scanoutConfig));

    m_scanout->setColorTransform(m_colorTransform);

    m_cursor = DrmCursor::create(m_device, m_config.crtcId);
    m_scanout->setHardwareCursor(m_cursor != nullptr);

    LM_INFO(lingmoDrm).printf("Render target initialized for %s: %zu overlay planes, hardware cursor %s",
                          m_config.name.c_str(), m_scanout->maxOverlays(),
                          m_cursor ? "available" : "unavailable");
    return true;
}

bool DrmOutput::requiresSoftwareBlanking() const
{
    if (!m_backend)
        return false;
    const auto driver = m_backend->driverName();
    return driver == "virtio_gpu" || driver == "vmwgfx"
            || driver == "qxl";
}

bool DrmOutput::configure(const std::vector<Configuration> &configuration, bool testOnly,
                          DrmCommitStatus *status, std::string *diagnostic,
                          DrmDeviceApi **failedDevice)
{
    std::string detail;
    DrmDeviceApi *device = nullptr;
    auto report = [status, diagnostic, failedDevice, &device, &detail](DrmCommitStatus result) {
        if (status)
            *status = result;
        if (failedDevice)
            *failedDevice = result == DrmCommitStatus::Success ? nullptr : device;
        if (diagnostic)
            *diagnostic = drmCommitMessage(result) + (detail.empty() ? "" : "; " + detail);
        return result == DrmCommitStatus::Success;
    };
    std::set<std::pair<DrmDeviceApi *, uint32_t>> connectors, crtcs, primaryPlanes;
    std::map<DrmBackend *, uint64_t> epochs;
    std::vector<std::unique_ptr<base::AutoReset<bool>>> guards;
    for (const auto &change : configuration) {
        auto *output = change.output;
        if (!output || !output->m_scanout || !output->m_device || output->m_configuring
            || connectors.contains({output->m_device, output->m_config.connectorId})
            || crtcs.contains({output->m_device, output->m_config.crtcId})
            || primaryPlanes.contains({output->m_device, output->m_config.planeId}))
            return report(DrmCommitStatus::InvalidRequest);
        device = output->m_device;
        connectors.insert({device, output->m_config.connectorId});
        crtcs.insert({device, output->m_config.crtcId});
        primaryPlanes.insert({device, output->m_config.planeId});
        if (output->m_backend)
            epochs[output->m_backend] = output->m_backend->topologyEpoch();
        guards.push_back(std::make_unique<base::AutoReset<bool>>(output->m_configuring, true));
    }
    const auto valid = [&] {
        for (const auto &[backend, epoch] : epochs)
            if (backend->topologyEpoch() != epoch)
                return false;
        return std::all_of(configuration.begin(), configuration.end(), [](const auto &change) {
            return !change.output->m_retiring && change.output->m_sessionActive
                    && change.output->m_sessionActive();
        });
    };
    if (!device)
        return report(DrmCommitStatus::InvalidRequest);
    if (!valid())
        return report(DrmCommitStatus::Invalidated);
    report(DrmCommitStatus::PreparationFailed);

    struct Prepared {
        DrmOutput *output = nullptr;
        drmModeModeInfo mode = {};
        bool enabled = false;
        bool blanked = false;
        gbm_surface *surface = nullptr;
        std::unique_ptr<render::EglRenderTarget> target;
        std::unique_ptr<CompositedBuffer> buffer;
        uint32_t blob = 0;

        ~Prepared()
        {
            buffer.reset();
            if (target)
                output->m_renderer->releaseTarget(target.get());
            target.reset();
            if (surface)
                gbm_surface_destroy(surface);
            if (blob)
                output->m_device->destroyPropertyBlob(blob);
        }
    };

    std::vector<std::unique_ptr<Prepared>> prepared;

    struct DeviceBatch {
        DrmDeviceApi *device = nullptr;
        AtomicRequest candidate;
        AtomicRequest previous;
        bool committed = false;
        bool restored = false;
    };

    std::vector<std::unique_ptr<DeviceBatch>> batches;
    const auto batchFor = [&](DrmDeviceApi *device) -> DeviceBatch * {
        for (const auto &batch : batches)
            if (batch->device == device)
                return batch.get();
        auto batch = std::make_unique<DeviceBatch>();
        batch->device = device;
        batches.push_back(std::move(batch));
        return batches.back().get();
    };
    for (const auto &change : configuration) {
        auto *output = change.output;
        device = output->m_device;
        report(DrmCommitStatus::PreparationFailed);
        auto *batch = batchFor(device);
        auto item = std::make_unique<Prepared>();
        item->output = output;
        item->mode = change.mode;
        item->enabled = change.enabled;
        item->blanked = change.enabled && change.blanked;
        std::vector<PlaneContent> planes;
        if (change.enabled) {
            const base::Size size(change.mode.hdisplay, change.mode.vdisplay);
            if (size.isEmpty() || (!output->m_backend && !output->m_createTarget)
                || !device->createPropertyBlob(&change.mode, sizeof(change.mode), &item->blob))
                return false;
            if (output->m_createTarget) {
                item->target = output->m_createTarget(size);
            } else {
                item->surface = output->createSurface(size);
                if (!item->surface)
                    return false;
                item->target = output->m_renderer->createTarget(item->surface, size);
            }
            if (!item->target
                || !output->fillModesetBuffer(item->target.get(), size,
                                              !item->blanked && !testOnly))
                return false;
            if (output->m_createTarget) {
                item->buffer = output->m_acquireBuffer();
                if (!item->buffer)
                    return false;
                item->buffer->primary.planeId = output->m_config.planeId;
                item->buffer->primary.displayRect = base::Rect(size);
            } else {
                item->buffer = output->acquireComposited(item->surface);
                if (!item->buffer)
                    return report(output->m_transferUnavailable
                                          ? DrmCommitStatus::TransferUnavailable
                                          : DrmCommitStatus::PreparationFailed);
            }
            planes.push_back(item->buffer->primary);
        }
        // A modeset is where a DPMS wake comes back on screen. A blocked session
        // has to clear the application planes here too, or the display returns
        // showing the frame the previous session left on them.
        const auto request = output->m_scanout->buildRequest(
                planes, true, item->blob, !change.enabled, output->m_secureScanout);
        batch->candidate.properties.insert(batch->candidate.properties.end(),
                                           request.properties.begin(), request.properties.end());
        auto oldPlanes = output->m_pendingFrame.planes.empty() ? output->m_scanoutFrame.planes
                                                               : output->m_pendingFrame.planes;
        if (!output->m_enabled && oldPlanes.empty()) {
            PlaneContent disabled;
            disabled.planeId = output->m_config.planeId;
            oldPlanes.push_back(disabled);
        }
        const auto previous = output->m_scanout->buildRequest(oldPlanes, true, output->m_modeBlobId,
                                                              !output->m_enabled);
        batch->previous.properties.insert(batch->previous.properties.end(),
                                          previous.properties.begin(), previous.properties.end());
        prepared.push_back(std::move(item));
    }
    DrmCommitStatus outcome = DrmCommitStatus::Success;
    if (testOnly) {
        for (const auto &batch : batches) {
            device = batch->device;
            DrmCommitCoordinator coordinator(batch->device);
            outcome = coordinator.execute(batch->candidate, true, DRM_MODE_ATOMIC_ALLOW_MODESET,
                                          nullptr, valid);
            if (outcome != DrmCommitStatus::Success)
                break;
        }
        return report(outcome);
    }
    std::vector<DrmDeviceTransaction> transactions;
    for (const auto &owned : batches) {
        auto *batch = owned.get();
        std::string key = std::to_string(transactions.size());
        for (const auto &change : configuration)
            if (change.output->m_device == batch->device && change.output->m_backend)
                key = change.output->m_backend->devicePath();
        transactions.push_back({key,
                             [&, batch] {
                                 device = batch->device;
                                 DrmCommitCoordinator coordinator(batch->device);
                                 outcome = coordinator.execute(batch->candidate, true,
                                                               DRM_MODE_ATOMIC_ALLOW_MODESET,
                                                               nullptr, valid);
                                 return outcome == DrmCommitStatus::Success;
                             },
                             [&, batch] {
                                 device = batch->device;
                                 if (!valid()) {
                                     outcome = DrmCommitStatus::Invalidated;
                                     return false;
                                 }
                                 batch->committed = batch->device->atomicCommit(
                                         batch->candidate, DRM_MODE_ATOMIC_ALLOW_MODESET, nullptr);
                                 if (!batch->committed)
                                     outcome = DrmCommitStatus::CommitFailed;
                                 return batch->committed;
                             },
                             [batch] {
                                 batch->restored = batch->device->atomicCommit(
                                         batch->previous, DRM_MODE_ATOMIC_ALLOW_MODESET, nullptr);
                                 return batch->restored;
                             }});
    }
    const auto committed = DrmMultiDeviceCommitCoordinator::run(transactions, 0, [&] {
        return valid() ? 0 : 1;
    });
    if (!committed.succeeded) {
        detail = committed.failedDevice + ": " + committed.message;
        LM_WARNING(lingmoDrm) << detail;
        if (outcome == DrmCommitStatus::Success)
            outcome = DrmCommitStatus::Invalidated;
        for (const auto &batch : batches)
            if (batch->committed && !batch->restored)
                outcome = DrmCommitStatus::RollbackFailed;
        if (outcome != DrmCommitStatus::RollbackFailed)
            return report(outcome);
    }
    std::vector<std::pair<DrmOutput *, uint64_t>> discarded;
    for (auto &item : prepared) {
        auto *output = item->output;
        auto *batch = batchFor(output->m_device);
        if (!committed.succeeded && (!batch->committed || batch->restored))
            continue;
        output->m_wakeup.stop();
        output->m_scheduler.suspend();
        for (const uint64_t frame : output->dropOutstandingFrames())
            discarded.push_back({output, frame});
        output->releaseScanoutBuffers();
        output->m_bufferFrame = 0;
        output->destroyModeBlob();
        output->m_enabled = item->enabled;
        output->m_blanked = item->blanked;
        if (!item->enabled || item->blanked)
            output->hideCursor();
        output->m_scanout->m_activePlanes.clear();
        if (!item->enabled)
            continue;
        output->m_renderer->releaseTarget(output->m_target.get());
        output->m_target.reset();
        if (output->m_gbmSurface)
            gbm_surface_destroy(output->m_gbmSurface);
        output->m_gbmSurface = std::exchange(item->surface, nullptr);
        output->m_target = std::move(item->target);
        output->m_modeBlobId = std::exchange(item->blob, 0);
        output->m_config.mode = item->mode;
        output->m_scanout->m_config.modeSize = output->size();
        output->m_scanout->m_activePlanes = {output->m_config.planeId};
        // The modeset carried the colour pipeline.
        output->m_scanout->m_colorPending = false;
        output->m_scanoutFrame.planes = {item->buffer->primary};
        output->m_scanoutFrame.composited = std::move(item->buffer);
        // The modeset commits blocking, so this frame is already on screen.
        output->m_scanoutFrame.secure =
                output->m_secureScanout && isSecureConfiguration(output->m_scanoutFrame.planes);
        output->m_lastPresentedSecure = output->m_scanoutFrame.secure;
        output->m_scheduler.setPeriod(framePeriod(output->pacingRate()));
        if (committed.succeeded && !item->blanked) {
            output->m_scheduler.resume();
            output->m_forceFrame = true;
            output->scheduleFrame(Reason::Scene);
        }
    }
    // Observers must see every route updated before any completion notification.
    for (const auto &[output, frame] : discarded)
        output->frameDiscarded(frame);
    return report(outcome);
}

bool DrmOutput::fillModesetBuffer(render::EglRenderTarget *target, const base::Size &size,
                                  bool copyScanout)
{
    // What the CRTC shows now, another DRM master's last frame or this output's
    // own, carries over into the new mode instead of a black frame between the
    // two. A blocked session stays black: a frame it did not compose must not
    // count as its secure presentation.
    render::DmaBufAttributes current;
    const bool copy = copyScanout && (!m_secureScanout || m_carryOverWhileSecure)
            && m_device->exportCrtcFramebuffer(m_config.crtcId, &current) && current.size == size;
    if (copy)
        m_renderer->setOutputContext({m_config.name, base::Rect(size), size, 1, 0, 0});
    if (!m_renderer->beginFrame(target))
        return false;
    m_renderer->clear({0, 0, 0, 1});
    // Imported once the target is current, and kept until the frame has ended.
    std::shared_ptr<render::Texture> texture = copy ? m_renderer->importDmaBuf(current) : nullptr;
    const base::Rect viewport(size);
    if (texture && m_keepCarriedFrame) {
        // A copy, not the import: the other master may still draw into its buffer.
        m_renderer->renderToTexture(m_carriedFrame, viewport, [&] {
            m_renderer->drawTexture(texture.get(), viewport, 1.0f);
        });
    }
    if (texture)
        m_renderer->drawTexture(texture.get(), viewport, 1.0f);
    return m_renderer->endFrame();
}

bool DrmOutput::createModeBlob()
{
    destroyModeBlob();
    if (!m_device->createPropertyBlob(&m_config.mode, sizeof(m_config.mode), &m_modeBlobId)) {
        LM_CRITICAL(lingmoDrm) << "Failed to create the mode property blob";
        return false;
    }
    return true;
}

void DrmOutput::destroyModeBlob()
{
    if (m_modeBlobId) {
        m_device->destroyPropertyBlob(m_modeBlobId);
        m_modeBlobId = 0;
    }
}

bool DrmOutput::enable()
{
    if (m_enabled)
        return true;
    if (!m_target || !m_renderer || !m_scanout || !m_acquireBuffer || !createModeBlob())
        return false;
    if (!m_pendingFrame.planes.empty() || !m_scanoutFrame.planes.empty()) {
        // Resume without requiring a third GBM buffer while both old frames may
        // still be held. A blocking modeset first establishes known scanout.
        const bool keepPending = !m_pendingFrame.planes.empty();
        SubmittedFrame &retained = keepPending ? m_pendingFrame : m_scanoutFrame;
        m_scheduler.resume();
        const auto frame = m_scheduler.begin();
        if (!frame || !commit(retained.planes, true, frame)) {
            m_scheduler.suspend();
            return false;
        }
        m_presentations.submitted(frame, retained.presentation);
        if (keepPending) {
            retire(m_scanoutFrame, render::SyncFence());
            m_scanoutFrame = std::move(m_pendingFrame);
            m_pendingFrame = SubmittedFrame();
        }
        m_bufferFrame = 0;
        m_enabled = true;
        m_outstanding.push_back(frame);
        m_scheduler.submitted(frame);
        scheduleFrame(Reason::Resume);
        LM_INFO(lingmoDrm) << "Output resumed with retained scanout:" << m_config.name;
        return true;
    }
    m_enabled = true;
    m_scheduler.resume();
    const auto frame = m_scheduler.begin();
    if (!frame || !renderFrame(frame, true)) {
        if (frame)
            frameDiscarded(frame);
        std::erase(m_outstanding, frame);
        m_enabled = false;
        m_scheduler.suspend();
        return false;
    }
    m_scheduler.submitted(frame);
    frameDrawn(frame, aimedNs());
    LM_INFO(lingmoDrm) << "Output enabled:" << m_config.name;
    return true;
}

bool DrmOutput::retireForReprobe(bool connectorExists, bool keepRoute)
{
    m_retiring = true;
    m_wakeup.stop();
    m_flipTimeout.stop();
    m_scheduler.suspend();
    hideCursor();
    if (!m_sessionActive || !m_sessionActive())
        return false;
    const auto showsClientBuffer = [this] {
        std::vector<const SubmittedFrame *> frames {&m_scanoutFrame, &m_pendingFrame};
        if (m_queuedFrame)
            frames.push_back(&m_queuedFrame->submitted);
        return std::any_of(frames.begin(), frames.end(), [](const SubmittedFrame *frame) {
            return std::any_of(frame->planes.begin(), frame->planes.end(),
                               [](const PlaneContent &content) {
                                   return content.isDirectScanout();
                               });
        });
    };
    if (m_scanout && !(keepRoute && connectorExists && !showsClientBuffer())) {
        auto request = m_scanout->buildRequest({}, true, 0, true);
        if (!connectorExists) {
            std::erase_if(request.properties, [this](const auto &property) {
                return property.objectId == m_config.connectorId;
            });
        }
        if (!m_device->atomicCommit(request, DRM_MODE_ATOMIC_ALLOW_MODESET, nullptr))
            return false;
    }
    m_enabled = false;
    for (const uint64_t frame : dropOutstandingFrames())
        frameDiscarded(frame);
    releaseScanoutBuffers();
    m_bufferFrame = 0;
    destroyModeBlob();
    m_presentations.abandon();
    m_prepareCallback = {};
    m_renderCallback = {};
    m_beginFrameCallback = {};
    m_scanoutPlanner = {};
    presentationSubmitted = {};
    return true;
}

void DrmOutput::disable()
{
    m_wakeup.stop();
    m_flipTimeout.stop();
    m_scheduler.suspend();
    for (const uint64_t frame : dropOutstandingFrames())
        frameDiscarded(frame);
    // An inactive session may still have old scanout references. Keep both
    // frames until their flip retires or a successful modeset replaces them.
    if (m_enabled && m_sessionActive && m_sessionActive() && m_scanout && m_scanout->disable()) {
        releaseScanoutBuffers();
        m_bufferFrame = 0;
    }
    if (m_cursor)
        m_cursor->hide();
    m_enabled = false;
    // Waking up has to earn secure presentation again: what was on screen
    // before the output went dark says nothing about the frame it comes back
    // with.
    m_lastPresentedSecure = false;
    destroyModeBlob();
}

void DrmOutput::retire(SubmittedFrame &frame, const render::SyncFence &fence)
{
    for (PlaneContent &content : frame.planes) {
        if (content.buffer)
            content.buffer->onScanoutReleased(fence.duplicate());
    }
    frame.composited.reset();
    frame = SubmittedFrame();
}

void DrmOutput::releaseScanoutBuffers()
{
    if (m_queuedFrame) {
        retire(m_queuedFrame->submitted, render::SyncFence());
        m_queuedFrame.reset();
    }
    retire(m_pendingFrame, render::SyncFence());
    retire(m_scanoutFrame, render::SyncFence());
}

std::vector<uint64_t> DrmOutput::dropOutstandingFrames()
{
    if (m_queuedFrame) {
        retire(m_queuedFrame->submitted, render::SyncFence());
        m_queuedFrame.reset();
    }
    return std::exchange(m_outstanding, {});
}

void DrmOutput::commitQueued()
{
    if (!m_queuedFrame)
        return;
    QueuedFrame queued = std::move(*m_queuedFrame);
    m_queuedFrame.reset();
    if (!m_enabled || m_blanked || !commit(queued.submitted.planes, false, queued.frame)) {
        retire(queued.submitted, render::SyncFence());
        std::erase(m_outstanding, queued.frame);
        frameDiscarded(queued.frame);
        m_scheduler.failed(queued.frame);
        return;
    }
    m_presentations.submitted(queued.frame, queued.submitted.presentation);
    queued.submitted.releaseFence = std::move(m_lastReleaseFence);
    retire(m_pendingFrame, render::SyncFence());
    m_pendingFrame = std::move(queued.submitted);
    m_bufferFrame = queued.frame;
}

void DrmOutput::clearPromotions()
{
    for (scene::SurfaceNode *node : m_promoted)
        node->setPromotedToOverlay(false);
    m_promoted.clear();
}

void DrmOutput::scheduleFrame(Reason reason)
{
    if (m_retiring || m_blanked)
        return;
    if (reason == Reason::Startup || reason == Reason::Resume)
        m_forceFrame = true;
    m_scheduler.request(reason, m_fullRepaintQuery && m_fullRepaintQuery());
    armWakeup();
}

void DrmOutput::warmUp()
{
    if (m_retiring || m_blanked)
        return;
    m_forceFrame = true;
    scheduleFrame(Reason::Scene);
}

void DrmOutput::armWakeup()
{
    const auto deadline = m_scheduler.deadline();
    if (!deadline) {
        m_wakeup.stop();
        return;
    }
    // Damage cannot push a deadline back; a flip that moved the slots can
    // bring it forward.
    if (m_wakeup.isActive() && *deadline >= m_armedDeadline)
        return;
    m_armedDeadline = *deadline;
    // Rounded down: OutputScheduler::begin() takes a wakeup within its slack
    // of the deadline, where rounding up would start the frame late.
    const auto remaining = *deadline - m_now();
    const auto ms = std::chrono::floor<std::chrono::milliseconds>(remaining).count();
    m_wakeup.start(std::chrono::milliseconds(std::max<int64_t>(0, ms)));
}

uint64_t DrmOutput::aimedNs() const
{
    const auto aimed = m_scheduler.frameTime() + m_scheduler.period();
    return uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(aimed.time_since_epoch())
                           .count());
}

void DrmOutput::wakeup()
{
    const auto frame = m_scheduler.begin();
    const bool drawn = frame
            && (!m_beginFrameCallback
                || m_beginFrameCallback(std::chrono::duration<double, std::milli>(
                                                m_scheduler.sampleTime().time_since_epoch())
                                                .count())
                || m_forceFrame);
    if (frame && !drawn) {
        m_scheduler.skipped(frame);
    } else if (frame) {
        const auto started = m_now();
        if (renderFrame(frame, false)) {
            m_forceFrame = false;
            m_scheduler.submitted(frame);
            frameDrawn(frame, aimedNs());
            const auto took = m_now() - started;
            if (took > m_scheduler.period()) {
                LM_DEBUG(lingmoDrm) << "Composited frame" << frame << "for" << m_config.name << "in"
                                << std::chrono::duration<double, std::milli>(took).count()
                                << "ms, longer than its refresh period";
            }
        } else {
            std::erase(m_outstanding, frame);
            frameDiscarded(frame);
            m_scheduler.failed(frame);
        }
    }
    armWakeup();
}

bool DrmOutput::renderFrame(uint64_t frame, bool modeset)
{
    bool prepared = false;
    if (renderAttempt(frame, modeset, m_directScanoutEnabled, prepared))
        return true;
    if (!m_directScanoutEnabled || !m_enabled)
        return false;

    // A plane configuration that passed TEST_ONLY can still be rejected by the
    // real commit. Composing the same frame without promotions is always
    // possible, and it is what keeps a failed optimization from dropping a
    // frame or leaving a stale buffer on screen.
    LM_WARNING(lingmoDrm) << "Hardware plane commit failed; compositing this frame instead";
    return renderAttempt(frame, modeset, false, prepared, true);
}

bool DrmOutput::renderAttempt(uint64_t frame, bool modeset, bool allowPromotion, bool &prepared,
                              bool retry)
{
    if (!m_enabled || m_blanked || !m_renderer->beginFrame(m_target.get()))
        return false;

    render::PresentationSubmission presentation;

    struct ResetCapture {
        render::Renderer *renderer;

        ~ResetCapture()
        {
            renderer->presentationSubmission = nullptr;
            renderer->frameTime = -1;
            renderer->framePeriod = 0;
        }
    } resetCapture {m_renderer};

    m_renderer->presentationSubmission = &presentation;
    m_renderer->frameTime = std::chrono::duration<double, std::milli>(
                                    m_scheduler.sampleTime().time_since_epoch())
                                    .count();
    m_renderer->framePeriod =
            std::chrono::duration<double, std::milli>(m_scheduler.period()).count();
    ++m_frameCount;
    if (std::find(m_outstanding.begin(), m_outstanding.end(), frame) == m_outstanding.end())
        m_outstanding.push_back(frame);
    if (!prepared && m_prepareCallback)
        m_prepareCallback(*m_renderer, frame);
    prepared = true;

    // The framebuffer currently on the primary plane stands in for the one this
    // frame will produce: same size, format and modifier, which is what makes
    // the atomic test meaningful. Upstream allocates a throwaway buffer for
    // this; reusing the real one is both cheaper and more accurate.
    PlaneContent composited;
    if (!m_scanoutFrame.planes.empty() && !m_scanoutFrame.planes.front().isDirectScanout())
        composited = m_scanoutFrame.planes.front();

    scene::OverlayPlan plan;
    std::vector<PlaneContent> planes;
    // A plane the blend cannot reach would show untinted; the cursor's image
    // carries the tint instead.
    const bool blended = blendsColorTransform();
    if (m_cursor)
        m_cursor->setTint(blended ? m_colorTransform : render::ColorTransform());
    if (allowPromotion && !blended && !m_secureScanout && !modeset && m_scanout
        && m_scanoutPlanner) {
        plan = m_scanoutPlanner(size(), m_scanout->maxOverlays());
        planes = m_scanout->validate(plan, composited, m_modeBlobId);
    }

    const bool direct = !planes.empty() && planes.front().isDirectScanout();
    for (PlaneContent &content : planes) {
        if (!content.node)
            continue;
        content.node->setPromotedToOverlay(true);
        m_promoted.push_back(content.node);
        presentation.add(content.node->presentationContent(), true);
    }

    std::unique_ptr<CompositedBuffer> compositedBuffer;
    render::GpuTimer::Mark drawnMark;
    render::SyncFence drawnFence;
    if (direct) {
        // Nothing to composite: the client's own buffer goes on the primary
        // plane and the GBM front buffer is left exactly as it was, ready for
        // the moment scanout stops being possible.
        m_renderer->cancelFrame();
    } else {
        // Plane promotion changes pixels without changing scene revisions.
        if (retry || planes.size() > 1 || m_scanoutFrame.planes.size() > 1) {
            const auto viewport = m_renderer->outputContext().logicalViewport;
            m_renderer->prepareDamage(viewport.isEmpty() ? base::Rect(size()) : viewport,
                                      {});
        }
        // A buffer kept from an earlier frame holds another tint, or none.
        if (blended || m_untintedRepaints > 0)
            m_renderer->invalidateFrame();
        if (m_renderCallback)
            m_renderCallback(*m_renderer, frame);
        // After the scene and the captures that copy it, which stay untinted.
        if (blended)
            m_renderer->tintFrame(m_colorTransform);
        // Tells the scheduler how long composition really takes on this GPU.
        // A fence's export waits for the driver to submit the frame; a
        // timestamp query rides the swap's own submission.
        if (m_renderer->marksDrawn())
            drawnMark = m_renderer->markDrawn();
        else
            drawnFence = m_renderer->createReleaseFence();
        if (!m_enabled || !m_renderer->endFrame()) {
            clearPromotions();
            return false;
        }
        if (blended)
            m_untintedRepaints = 0;
        else if (m_untintedRepaints > 0)
            --m_untintedRepaints;
        compositedBuffer = m_acquireBuffer();
        if (!compositedBuffer) {
            LM_WARNING(lingmoDrm) << "Failed to obtain a framebuffer for the composited frame";
            clearPromotions();
            return false;
        }
        PlaneContent primary = compositedBuffer->primary;
        primary.planeId = planes.empty() ? m_config.planeId : planes.front().planeId;
        primary.displayRect = base::Rect(size());
        if (planes.empty())
            planes.push_back(std::move(primary));
        else
            planes.front() = std::move(primary);
    }

    // The previous flip has not landed, so this frame waits its turn at KMS;
    // its rendering has already overlapped that wait, which is the point.
    const bool queue = !modeset && m_bufferFrame != 0;
    if (!queue && !commit(planes, modeset, frame)) {
        clearPromotions();
        return false;
    }

    if (direct)
        ++m_directScanoutFrames;
    if (planes.size() > 1)
        ++m_overlayFrames;
    if (presentationSubmitted)
        presentationSubmitted(presentation);
    SubmittedFrame submitted;
    submitted.presentation = std::move(presentation);
    if (!queue)
        m_presentations.submitted(frame, submitted.presentation);
    submitted.planes = std::move(planes);
    submitted.composited = std::move(compositedBuffer);
    submitted.drawnMark = std::move(drawnMark);
    submitted.drawnFence = std::move(drawnFence);
    if (!queue)
        submitted.releaseFence = std::move(m_lastReleaseFence);
    submitted.secure = m_secureScanout && isSecureConfiguration(submitted.planes)
            && (!m_secureContentCheck || m_secureContentCheck(m_renderer->frameTime));
    for (PlaneContent &content : submitted.planes) {
        if (content.buffer)
            content.buffer->onScanoutAcquired();
    }

    if (queue) {
        // The scheduler holds at most one frame behind the flip; a stale one
        // here would be a bookkeeping slip, and the newer frame wins.
        if (m_queuedFrame) {
            const uint64_t stale = m_queuedFrame->frame;
            retire(m_queuedFrame->submitted, render::SyncFence());
            m_queuedFrame.reset();
            frameDiscarded(stale);
            m_scheduler.failed(stale);
        }
        m_queuedFrame = QueuedFrame {frame, std::move(submitted)};
    } else if (modeset) {
        // A blocking modeset has already taken effect, so the previous frame is
        // done being read the moment it returns.
        releaseScanoutBuffers();
        m_scanoutFrame = std::move(submitted);
        m_lastPresentedSecure = m_scanoutFrame.secure;
        m_bufferFrame = 0;
    } else {
        retire(m_pendingFrame, render::SyncFence());
        m_pendingFrame = std::move(submitted);
        m_bufferFrame = frame;
    }

    // Promotion lasts exactly one frame; the next one re-decides from scratch.
    clearPromotions();
    return true;
}

void DrmOutput::dispatchPageFlip(FlipEvent event, uint32_t seconds, uint32_t microseconds,
                                 uint32_t sequence)
{
    if (event.output)
        event.output->onPageFlip(event.frame, seconds, microseconds, sequence);
}

void DrmOutput::onPageFlip(uint64_t frame, uint32_t seconds, uint32_t microseconds,
                           uint32_t sequence)
{
    if (!m_events.erase(frame))
        return; // One unique kernel event per token.
    // A driver without vblank (virtio-gpu) stamps every flip with sequence 0,
    // and its mode's refresh is made up: QEMU's EDID says 75 Hz. The frames
    // reach a host display running at its own rate, 60 or 120 Hz, where 75 fps
    // cannot be paced evenly and 60 can.
    if (!m_unsynced && sequence == 0 && m_lastSequence == 0 && m_completions > 0) {
        m_unsynced = true;
        m_scheduler.setPeriod(framePeriod(pacingRate()));
        LM_INFO(lingmoDrm) << m_config.name << "has no vblank; pacing at" << pacingRate()
                       << "Hz instead of the mode's" << refreshRate();
    }
    m_presentations.complete(frame, seconds, microseconds, sequence, m_monotonicClock,
                             pacingRate(), presentationOutput);
    const uint32_t timeMs = seconds * 1000u + microseconds / 1000u;
    ++m_completions;
    m_lastSequence = sequence;
    m_lastTimeMs = timeMs;
    // std::steady_clock is CLOCK_MONOTONIC on Linux, the clock a monotonic
    // DRM device stamps its events with.
    std::optional<render::OutputScheduler::Time> presented;
    if (m_monotonicClock && microseconds < 1000000) {
        presented = render::OutputScheduler::Time(std::chrono::seconds(seconds)
                                                  + std::chrono::microseconds(microseconds));
    }
    finishFlip(frame, timeMs, presented);
}

void DrmOutput::finishFlip(uint64_t frame, uint32_t timeMs,
                           std::optional<render::OutputScheduler::Time> presented)
{
    if (m_events.empty())
        m_flipTimeout.stop();
    else
        m_flipTimeout.start();
    if (frame == m_bufferFrame && !m_pendingFrame.planes.empty()) {
        if (const auto done = m_renderer ? m_renderer->drawnAt(m_pendingFrame.drawnMark)
                                         : std::nullopt) {
            m_scheduler.drawn(frame, *done);
        } else if (const auto done = m_pendingFrame.drawnFence.signalTimeNs()) {
            m_scheduler.drawn(frame,
                              render::OutputScheduler::Time(std::chrono::nanoseconds(*done)));
        }
        m_pendingFrame.drawnMark = {};
        m_pendingFrame.drawnFence.reset();
        // The commit that just landed is what stopped the previous frame's
        // buffers being read, so its out-fence is their release fence.
        retire(m_scanoutFrame, m_pendingFrame.releaseFence);
        m_scanoutFrame = std::move(m_pendingFrame);
        m_lastPresentedSecure = m_scanoutFrame.secure;
        m_pendingFrame = SubmittedFrame();
        m_bufferFrame = 0;
        // Before anyone hears of this completion: the next frame is already
        // drawn, and every microsecond it waits here is latency.
        commitQueued();
    }
    // A device without vblank stamps when the host finished, which is no
    // refresh to lay the slots on.
    if (m_scheduler.complete(frame, m_unsynced ? std::nullopt : presented)) {
        std::erase(m_outstanding, frame);
        imagePresented(frame,
                              presented ? uint64_t(std::chrono::nanoseconds(
                                                           presented->time_since_epoch())
                                                           .count())
                                        : 0);
        framePresented(timeMs, frame);
    }
    armWakeup();
}

void DrmOutput::onFlipTimeout()
{
    if (m_events.empty())
        return;
    const auto oldest = m_events.begin();
    const uint64_t frame = oldest->first;
    LM_WARNING(lingmoDrm) << "No page flip event on" << m_config.name << "for frame" << frame
                      << "within"
                      << std::chrono::duration_cast<std::chrono::milliseconds>(m_flipTimeout.interval())
                                .count()
                     << "ms; no longer waiting for it";
    retireEvent(std::move(oldest->second));
    m_events.erase(oldest);
    // Nothing says the frame reached the screen, so its feedback is discarded,
    // while frame callbacks and scheduling move on as if it had.
    m_presentations.abandon(frame);
    const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
            m_now().time_since_epoch());
    finishFlip(frame, uint32_t(now.count()), std::nullopt);
}

void DrmOutput::retireEvent(std::unique_ptr<FlipEvent> event)
{
    event->output = nullptr;
    if (m_backend)
        m_backend->retainFlipEvent(std::move(event));
    else if (m_retireEvent)
        m_retireEvent(std::move(event));
}

void DrmOutput::reportStats() const
{
    LM_INFO(base::LogCategory::defaultCategory()) << statsLine();
}

std::string DrmOutput::statsLine() const
{
    const auto &s = m_scheduler.stats();
    const auto ms = [](std::chrono::steady_clock::duration duration) {
        return std::chrono::duration<double, std::milli>(duration).count();
    };
    std::string line;
    const auto add = [&line](const auto &value) {
        if (!line.empty())
            line += ' ';
        using Value = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<Value, bool>)
            line += value ? "true" : "false";
        else if constexpr (std::is_floating_point_v<Value>)
            line += base::strings::number(double(value));
        else if constexpr (std::is_arithmetic_v<Value>)
            line += std::to_string(value);
        else
            line += value;
    };
    add("Output scheduling");
    add('"' + m_config.name + '"');
    add("duration-ms");
    add(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()
                                                              - m_statsStart)
                .count());
    add("process-CPU-ms");
    add(1000.0 * (std::clock() - m_cpuStart) / CLOCKS_PER_SEC);
    add("refresh-Hz");
    add(refreshRate());
    add("pacing-Hz");
    add(pacingRate());
    add("period-ns");
    add(std::chrono::duration_cast<std::chrono::nanoseconds>(m_scheduler.period()).count());
    for (const auto &[label, value] : {std::pair<const char *, uint64_t> {"renders", m_frameCount},
                                       {"KMS-commits", m_commits},
                                       {"normal-commits", m_flipCommits},
                                       {"unique-completions", m_completions},
                                       {"missed-slots", s.missed},
                                       {"late-demand", s.lateDemand},
                                       {"requests", s.requests},
                                       {"coalesced", s.coalesced},
                                       {"failures", s.failures},
                                       {"stale-completions", s.staleCompletions},
                                       {"skipped", s.skips},
                                       {"direct-scanout-frames", m_directScanoutFrames},
                                       {"overlay-frames", m_overlayFrames},
                                       {"plane-test-failures",
                                        m_scanout ? m_scanout->testFailures() : 0},
                                       {"scanout-import-failures",
                                        m_scanout ? m_scanout->importFailures() : 0}}) {
        add(label);
        add(value);
    }
    add("hardware-cursor");
    add(m_cursor != nullptr);
    add("last-kernel-sequence");
    add(m_lastSequence);
    add("last-kernel-time-ms");
    add(m_lastTimeMs);
    add("draw-budget-ms");
    add(ms(m_scheduler.drawBudget()));
    add("slowest-draw-ms");
    add(ms(m_scheduler.slowestDrawTime()));
    add("reasons startup/resume/scene/surface/decoration");
    for (int i = 0; i < 5; ++i)
        add(s.reasons[i]);
    return line;
}

bool DrmOutput::commit(const std::vector<PlaneContent> &planes, bool modeset, uint64_t frame)
{
    if (!m_scanout)
        return false;

    FlipEvent *token = nullptr;
    if (frame) {
        auto event = std::make_unique<FlipEvent>(FlipEvent {weakPtr(), frame});
        token = event.get();
        m_events.emplace(frame, std::move(event));
    }

    render::SyncFence releaseFence;
    const bool ok = m_scanout->commit(planes, modeset, token, m_modeBlobId,
                                      modeset ? nullptr : &releaseFence, m_secureScanout);
    if (!ok) {
        if (frame)
            m_events.erase(frame);
        LM_WARNING(lingmoDrm).printf("Atomic commit failed (%s): %s", modeset ? "modeset" : "flip",
                                 strerror(errno));
        return false;
    }

    if (frame && !m_flipTimeout.isActive())
        m_flipTimeout.start();
    if (modeset)
        m_presentations.discontinuity();
    m_lastReleaseFence = std::move(releaseFence);
    ++m_commits;
    if (!modeset)
        ++m_flipCommits;
    return true;
}

std::map<uint32_t, std::vector<uint64_t>> DrmOutput::primaryPlaneFormats() const
{
    for (const DrmPlane &plane : m_config.planes) {
        if (plane.id == m_config.planeId)
            return plane.formats;
    }
    return {};
}

std::optional<base::Point> DrmOutput::probedCursor() const
{
    for (const DrmPlane &plane : m_config.planes) {
        if (plane.type != DRM_PLANE_TYPE_CURSOR || plane.properties.value("FB_ID") == 0
            || plane.properties.value("CRTC_ID") != m_config.crtcId)
            continue;
        // Signed properties: an image may hang off the top or left edge.
        return base::Point(int(int64_t(plane.properties.value("CRTC_X"))),
                      int(int64_t(plane.properties.value("CRTC_Y"))));
    }
    return std::nullopt;
}

bool DrmOutput::setCursorImage(const render::CursorImage &image)
{
    if (m_blanked || !m_enabled)
        return true;
    if (!m_cursor)
        return false;
    return m_cursor->setImage(image.pixels, image.size, image.stride, image.hotspot);
}

void DrmOutput::hideCursor()
{
    if (m_cursor)
        m_cursor->hide();
}

void DrmOutput::moveCursor(const base::PointF &position)
{
    if (m_cursor)
        m_cursor->move(position.toPoint());
}

} // namespace Lingmo::Backend
