// Copyright 2014 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/
//               hardware_display_plane_manager_atomic.cc (Commit
//                   AddOutFencePtrProperties AddAllPendingCrtcProperties)
//               hardware_display_plane_manager.cc (UpdatePendingCrtcState)
//               hardware_display_plane_atomic.cc (AssignPlaneProps, SetPlaneProps)
//               drm_overlay_validator.cc (TestPageFlip)
//               drm_gpu_util.cc (CreateCTMBlob CreateLutBlob)
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a

#include "backend/drm/scanout_controller.h"

#include <drm_fourcc.h>
#include <drm_mode.h>
#include <xf86drmMode.h>

#include <algorithm>
#include <cmath>
#include <utility>

#include "backend/drm/drm_framebuffer.h"
#include "backend/drm/plane_assignment.h"
#include "core/logging.h"
#include "scene/surface_node.h"
namespace scene = Lingmo::Scene;

namespace Lingmo::Backend {

namespace {

// SRC_* are 16.16 fixed point.
uint64_t fixed16(int value)
{
    return static_cast<uint64_t>(value) << 16;
}

// A CTM entry is S31.32 sign-magnitude; gains are never negative.
uint64_t ctmValue(float gain)
{
    return static_cast<uint64_t>(std::max(gain, 0.0f) * static_cast<float>(1ull << 32))
            & ~(1ull << 63);
}

uint16_t lutValue(double value)
{
    return static_cast<uint16_t>(std::lround(std::clamp(value, 0.0, 1.0) * 65535.0));
}

} // namespace

ScanoutController::ScanoutController(DrmDeviceApi *device, Config config)
    : m_device(device)
    , m_config(std::move(config))
{
    // Planes are kept in enumeration order, which is ascending id, because the
    // assignment pass relies on that to preserve z ordering.
    for (const DrmPlane &plane : m_config.planes) {
        if (!plane.canUseForCrtc(m_config.crtcIndex))
            continue;
        if (plane.type == DRM_PLANE_TYPE_PRIMARY && !m_primaryPlane)
            m_primaryPlane = &plane;
        else if (plane.type == DRM_PLANE_TYPE_OVERLAY)
            m_overlayPlanes.push_back(&plane);
        else if (plane.type == DRM_PLANE_TYPE_CURSOR)
            m_cursorPlanes.push_back(&plane);
    }
}

ScanoutController::~ScanoutController()
{
    destroyBlobs(m_color);
}

bool ScanoutController::supportsColorTransform() const
{
    const DrmPropertyMap &properties = m_config.crtcProperties;
    return !m_colorRejected
            && (properties.id("CTM")
                || (properties.id("GAMMA_LUT") && properties.value("GAMMA_LUT_SIZE") > 1));
}

bool ScanoutController::setColorTransform(const render::ColorTransform &transform)
{
    if (transform == m_color.transform || !supportsColorTransform())
        return false;

    ColorPipeline color {transform, 0, 0};
    if (!transform.isIdentity()) {
        const DrmPropertyMap &properties = m_config.crtcProperties;
        bool created = false;
        if (properties.id("CTM")) {
            drm_color_ctm ctm = {};
            ctm.matrix[0] = ctmValue(transform.red);
            ctm.matrix[4] = ctmValue(transform.green);
            ctm.matrix[8] = ctmValue(transform.blue);
            created = m_device->createPropertyBlob(&ctm, sizeof(ctm), &color.ctmBlob);
        } else {
            // The gains apply to encoded values, so each channel's curve is a
            // straight line: exactly what the diagonal CTM would do.
            const size_t size = properties.value("GAMMA_LUT_SIZE");
            std::vector<drm_color_lut> lut(size);
            for (size_t i = 0; i < size; ++i) {
                const double x = double(i) / double(size - 1);
                lut[i].red = lutValue(x * transform.red);
                lut[i].green = lutValue(x * transform.green);
                lut[i].blue = lutValue(x * transform.blue);
            }
            created = m_device->createPropertyBlob(lut.data(), sizeof(drm_color_lut) * size,
                                                   &color.gammaBlob);
        }
        if (!created) {
            LM_WARNING(lingmoDrm).printf("Failed to create the colour transform blob for CRTC %u",
                                     m_config.crtcId);
            return false;
        }
    }

    // The kernel holds its own reference to a blob a CRTC uses.
    destroyBlobs(m_color);
    m_color = color;
    m_colorPending = true;
    return true;
}

void ScanoutController::destroyBlobs(const ColorPipeline &color)
{
    for (const uint32_t blob : {color.ctmBlob, color.gammaBlob}) {
        if (blob)
            m_device->destroyPropertyBlob(blob);
    }
}

const DrmPlane *ScanoutController::planeById(uint32_t id) const
{
    const auto plane = std::find_if(m_config.planes.begin(), m_config.planes.end(),
                                    [id](const DrmPlane &plane) {
                                        return plane.id == id;
                                    });
    return plane == m_config.planes.end() ? nullptr : &*plane;
}

uint32_t ScanoutController::framebufferFor(render::ScanoutBuffer &buffer, uint32_t *format,
                                           uint64_t *modifier)
{
    // A buffer this device already failed to import is not retried: the failure
    // is a property of the buffer, and retrying it costs an ioctl per frame.
    if (buffer.scanoutImportFailed(m_device))
        return 0;

    auto *framebuffer = static_cast<DrmFramebuffer *>(buffer.scanoutImport(m_device));
    if (!framebuffer) {
        std::unique_ptr<DrmFramebuffer> imported =
                DrmFramebuffer::fromDmaBuf(m_device, buffer.scanoutAttributes());
        if (!imported)
            ++m_importFailures;
        framebuffer = static_cast<DrmFramebuffer *>(
                buffer.setScanoutImport(m_device, std::move(imported)));
        if (!framebuffer)
            return 0;
    }

    *format = framebuffer->format();
    *modifier = framebuffer->modifier();
    return framebuffer->id();
}

std::vector<PlaneContent> ScanoutController::validate(const scene::OverlayPlan &plan,
                                                      const PlaneContent &composited,
                                                      uint32_t modeBlobId)
{
    std::vector<PlaneContent> fallback;
    if (composited.framebufferId)
        fallback.push_back(composited);

    if (!m_primaryPlane || plan.isEmpty())
        return fallback;

    // Resolve every candidate to a framebuffer first. A candidate this device
    // cannot import is simply not promoted.
    std::vector<PlaneContent> contents;

    const auto resolve = [&](const scene::OverlayCandidate &candidate) {
        if (!candidate.buffer || !candidate.opaque)
            return false;
        // YUV and higher-depth formats need an explicit color-conversion contract.
        switch (candidate.buffer->scanoutAttributes().format) {
        case DRM_FORMAT_XRGB8888:
        case DRM_FORMAT_ARGB8888:
        case DRM_FORMAT_XBGR8888:
        case DRM_FORMAT_ABGR8888:
            break;
        default:
            return false;
        }
        PlaneContent content;
        uint32_t format = 0;
        uint64_t modifier = 0;
        content.framebufferId = framebufferFor(*candidate.buffer, &format, &modifier);
        if (!content.framebufferId)
            return false;
        content.format = format;
        content.modifier = modifier;
        content.displayRect = candidate.displayRect;
        content.sourceRect = candidate.sourceRect;
        if (candidate.acquireFence.isValid()) {
            content.acquireFence = std::make_shared<render::SyncFence>(candidate.acquireFence.duplicate());
            if (!content.acquireFence->isValid())
                return false;
        }
        content.buffer = candidate.buffer;
        content.node = candidate.node;
        contents.push_back(std::move(content));
        return true;
    };

    if (plan.hasDirectScanout()) {
        if (!resolve(plan.primary))
            return fallback;
    } else {
        if (!composited.framebufferId)
            return fallback;
        contents.push_back(composited);
    }

    for (const scene::OverlayCandidate &candidate : plan.overlays)
        resolve(candidate);

    // Then ask the hardware, shrinking the configuration until it agrees.
    // Upstream drops the last plane and retests; the same loop here.
    while (contents.size() > 1 || (contents.size() == 1 && contents.front().isDirectScanout())) {
        std::vector<PlaneRequest> requests;
        requests.reserve(contents.size());
        for (size_t i = 0; i < contents.size(); ++i)
            requests.push_back(PlaneRequest {contents[i].format, contents[i].modifier, i == 0});

        const std::vector<PlaneAssignment> assignments =
                assignPlanes(m_config.planes, m_config.crtcIndex, requests);
        if (!assignments.empty()) {
            for (const PlaneAssignment &assignment : assignments)
                contents[assignment.requestIndex].planeId = assignment.planeId;

            bool compatible = true;
            uint64_t previousZ = 0;
            for (size_t i = 0; i < contents.size(); ++i) {
                const auto *plane = planeById(contents[i].planeId);
                const auto &properties = plane->properties;
                // Explicit acquire fences cannot silently become implicit synchronization.
                if (contents[i].acquireFenceFd() >= 0 && !properties.id("IN_FENCE_FD"))
                    compatible = false;
                if (properties.value("rotation", DRM_MODE_ROTATE_0) != DRM_MODE_ROTATE_0
                    || properties.value("alpha", 0xffff) != 0xffff)
                    compatible = false;
                // Plane IDs do not establish stacking; require a known current z position.
                if (contents.size() > 1) {
                    const auto z = properties.find("zpos");
                    if (!z || (i && z->value <= previousZ))
                        compatible = false;
                    if (z)
                        previousZ = z->value;
                }
            }
            const AtomicRequest request = buildRequest(contents, false, modeBlobId, false);
            if (compatible && m_device->atomicCommit(request, DRM_MODE_ATOMIC_TEST_ONLY, nullptr))
                return contents;
            ++m_testFailures;
        }

        // The topmost plane is the one to give up on: dropping a lower one
        // would leave content composited underneath something on a plane.
        // Direct scanout on its own is the last configuration tried, and when
        // that fails too the list empties and everything is composited.
        contents.pop_back();
    }

    return fallback;
}

void ScanoutController::addPlaneProperties(AtomicRequest &request, const DrmPlane &plane,
                                           const PlaneContent &content, bool off) const
{
    const DrmPropertyMap &properties = plane.properties;
    const uint32_t id = plane.id;

    request.add(id, properties.id("FB_ID"), off ? 0 : content.framebufferId);
    request.add(id, properties.id("CRTC_ID"), off ? 0 : m_config.crtcId);
    request.add(id, properties.id("SRC_X"), off ? 0 : fixed16(content.sourceRect.x()));
    request.add(id, properties.id("SRC_Y"), off ? 0 : fixed16(content.sourceRect.y()));
    request.add(id, properties.id("SRC_W"), off ? 0 : fixed16(content.sourceRect.width()));
    request.add(id, properties.id("SRC_H"), off ? 0 : fixed16(content.sourceRect.height()));
    request.add(id, properties.id("CRTC_X"), off ? 0 : content.displayRect.x());
    request.add(id, properties.id("CRTC_Y"), off ? 0 : content.displayRect.y());
    request.add(id, properties.id("CRTC_W"), off ? 0 : content.displayRect.width());
    request.add(id, properties.id("CRTC_H"), off ? 0 : content.displayRect.height());
    // A previous DRM master may have left the plane turned or mirrored.
    if (!off)
        request.add(id, properties.id("rotation"), DRM_MODE_ROTATE_0);

    // Validation rejects explicit-sync promotion without IN_FENCE_FD.
    if (!off && content.acquireFenceFd() >= 0)
        request.add(id, properties.id("IN_FENCE_FD"),
                    static_cast<uint64_t>(content.acquireFenceFd()));
}

std::vector<const DrmPlane *> ScanoutController::allPlanes() const
{
    std::vector<const DrmPlane *> planes;
    if (m_primaryPlane)
        planes.push_back(m_primaryPlane);
    planes.insert(planes.end(), m_overlayPlanes.begin(), m_overlayPlanes.end());
    return planes;
}

AtomicRequest ScanoutController::buildRequest(const std::vector<PlaneContent> &planes, bool modeset,
                                              uint32_t modeBlobId, bool off, bool secure) const
{
    AtomicRequest request;

    if (modeset) {
        request.add(m_config.connectorId, m_config.connectorProperties.id("CRTC_ID"),
                    off ? 0 : m_config.crtcId);
        request.add(m_config.crtcId, m_config.crtcProperties.id("MODE_ID"), off ? 0 : modeBlobId);
        request.add(m_config.crtcId, m_config.crtcProperties.id("ACTIVE"), off ? 0 : 1);
    }
    // A modeset replaces whatever colour pipeline a previous DRM master left,
    // its night light for instance; a flip carries ours only when it changed.
    // Ozone commits all three with the CRTC, zero meaning unset
    // (AddAllPendingCrtcProperties).
    if (!off && (modeset || m_colorPending)) {
        request.add(m_config.crtcId, m_config.crtcProperties.id("CTM"), m_color.ctmBlob);
        request.add(m_config.crtcId, m_config.crtcProperties.id("GAMMA_LUT"), m_color.gammaBlob);
    }
    if (modeset && !off)
        request.add(m_config.crtcId, m_config.crtcProperties.id("DEGAMMA_LUT"), 0);

    std::vector<uint32_t> used;
    for (const PlaneContent &content : planes) {
        const DrmPlane *plane = planeById(content.planeId);
        if (!plane)
            continue;
        addPlaneProperties(request, *plane, content, off);
        used.push_back(plane->id);
    }

    // A plane that was showing something last frame and is not in this
    // configuration has to be switched off explicitly, or it keeps scanning out
    // a buffer nothing owns any more. A secure commit does not trust that list
    // and disables every plane it is not programming, and neither does a
    // modeset: a previous DRM master may have left any of them showing its own.
    if (secure || (modeset && !off)) {
        std::vector<const DrmPlane *> planes = allPlanes();
        if (!m_hardwareCursor)
            planes.insert(planes.end(), m_cursorPlanes.begin(), m_cursorPlanes.end());
        for (const DrmPlane *plane : planes) {
            if (std::find(used.begin(), used.end(), plane->id) == used.end())
                addPlaneProperties(request, *plane, PlaneContent(), true);
        }
        return request;
    }

    for (uint32_t planeId : m_activePlanes) {
        if (std::find(used.begin(), used.end(), planeId) != used.end())
            continue;
        if (const DrmPlane *plane = planeById(planeId))
            addPlaneProperties(request, *plane, PlaneContent(), true);
    }

    return request;
}

bool ScanoutController::commit(const std::vector<PlaneContent> &planes, bool modeset,
                               void *userData, uint32_t modeBlobId, render::SyncFence *releaseFence,
                               bool secure)
{
    const uint32_t outFenceProperty =
            releaseFence ? m_config.crtcProperties.id("OUT_FENCE_PTR") : 0;

    uint32_t flags = 0;
    if (modeset) {
        flags = DRM_MODE_ATOMIC_ALLOW_MODESET | (userData ? DRM_MODE_PAGE_FLIP_EVENT : 0);
    } else {
        flags = DRM_MODE_PAGE_FLIP_EVENT | DRM_MODE_ATOMIC_NONBLOCK;
    }

    const auto attempt = [&] {
        AtomicRequest request = buildRequest(planes, modeset, modeBlobId, false, secure);
        // OUT_FENCE_PTR writes the fd straight into this variable during the
        // commit, so it has to outlive the ioctl.
        int outFenceFd = -1;
        if (outFenceProperty) {
            request.add(m_config.crtcId, outFenceProperty,
                        reinterpret_cast<uint64_t>(&outFenceFd));
        }
        if (!m_device->atomicCommit(request, flags, userData)) {
            const ScopedFd failedFence(outFenceFd);
            return false;
        }
        if (releaseFence && outFenceFd >= 0)
            *releaseFence = render::SyncFence(ScopedFd(outFenceFd));
        return true;
    };

    if (!attempt()) {
        // A driver that lists a colour property can still refuse a value for
        // it. Losing the tint beats losing every frame from here on.
        if (!(modeset || m_colorPending) || m_color.transform.isIdentity())
            return false;
        const ColorPipeline color = std::exchange(m_color, ColorPipeline());
        const bool pending = std::exchange(m_colorPending, true);
        if (!attempt()) {
            m_color = color;
            m_colorPending = pending;
            return false;
        }
        LM_WARNING(lingmoDrm).printf(
                "The driver refused the colour transform for CRTC %u; blending it instead",
                m_config.crtcId);
        destroyBlobs(color);
        m_colorRejected = true;
    }
    m_colorPending = false;

    m_activePlanes.clear();
    for (const PlaneContent &content : planes) {
        if (content.planeId)
            m_activePlanes.push_back(content.planeId);
    }
    return true;
}

bool ScanoutController::disable()
{
    std::vector<PlaneContent> planes;
    if (m_primaryPlane) {
        PlaneContent content;
        content.planeId = m_primaryPlane->id;
        planes.push_back(std::move(content));
    }

    const AtomicRequest request = buildRequest(planes, true, 0, true);
    if (!m_device->atomicCommit(request, DRM_MODE_ATOMIC_ALLOW_MODESET, nullptr))
        return false;
    m_activePlanes.clear();
    return true;
}

} // namespace Lingmo::Backend
