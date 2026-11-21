// Copyright 2022 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE.chromium file.
//
// Upstream: chromium/src/components/exo/wayland/wayland_dmabuf_feedback_manager.cc
// Revision: 7acd40ddefff49c9d36144999aafdbb674dbd07a
//
// Adapted for Lingmo: integrates the upstream behavior with native
// compositor types and ownership.

#include "wayland/protocols/dmabuf_feedback_manager.h"

#include <drm_fourcc.h>
#include <linux-dmabuf-v1-server-protocol.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <set>
#include <utility>
#include <vector>

#include "compositor/surface.h"
#include "compositor/surface_observer.h"
#include "core/logging.h"
#include "render/dmabuf.h"
#include "wayland/server_util.h"

namespace Lingmo::Wayland {

namespace {

enum TrancheFlags : uint32_t { kNone = 0, kScanout = 1 };

// The wire layout of one format table entry.
struct DmaBufFeedbackFormat {
    uint32_t format;
    uint32_t padding;
    uint64_t modifier;
};

void feedback_destroy(wl_client*, wl_resource* resource)
{
    wl_resource_destroy(resource);
}

const struct zwp_linux_dmabuf_feedback_v1_interface feedback_implementation = {
    feedback_destroy,
};

// Intel CCS modifiers leak memory on gbm to gl buffer import. Block these
// modifiers for now. See crbug.com/1445252, crbug.com/1458575.
// Also blocking |DRM_FORMAT_YVU420| for a specific modifier based on a test
// failing in minigbm. See b/289714323.
const std::set<std::pair<uint64_t, uint64_t>>& modifierBlockList()
{
    static const std::set<std::pair<uint64_t, uint64_t>> list = {
        { DRM_FORMAT_INVALID, I915_FORMAT_MOD_Y_TILED_CCS },
        { DRM_FORMAT_INVALID, I915_FORMAT_MOD_Yf_TILED_CCS },
        { DRM_FORMAT_INVALID, I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS },
        { DRM_FORMAT_INVALID, I915_FORMAT_MOD_Y_TILED_GEN12_MC_CCS },
        { DRM_FORMAT_INVALID, I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS_CC },
        { DRM_FORMAT_INVALID, I915_FORMAT_MOD_4_TILED_DG2_RC_CCS },
        { DRM_FORMAT_INVALID, I915_FORMAT_MOD_4_TILED_DG2_MC_CCS },
        { DRM_FORMAT_INVALID, I915_FORMAT_MOD_4_TILED_DG2_RC_CCS_CC },
        { DRM_FORMAT_YVU420, DRM_FORMAT_MOD_QCOM_COMPRESSED },
    };
    return list;
}

// A sealed memfd holding `table`. Clients map it read-only, so it must not be
// growable or writable once handed out.
ScopedFd createFormatTable(const std::vector<DmaBufFeedbackFormat>& table)
{
    const size_t size = table.size() * sizeof(DmaBufFeedbackFormat);
    if (size == 0)
        return ScopedFd();

    ScopedFd fd(memfd_create("lingmo-dmabuf-format-table",
                             MFD_CLOEXEC | MFD_ALLOW_SEALING));
    if (!fd.isValid()) {
        LM_WARNING(lingmoWayland) << "memfd_create failed for the dmabuf format table";
        return ScopedFd();
    }

    if (ftruncate(fd.get(), static_cast<off_t>(size)) != 0) {
        LM_WARNING(lingmoWayland) << "ftruncate failed for the dmabuf format table";
        return ScopedFd();
    }

    void* data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd.get(), 0);
    if (data == MAP_FAILED) {
        LM_WARNING(lingmoWayland) << "mmap failed for the dmabuf format table";
        return ScopedFd();
    }
    std::memcpy(data, table.data(), size);
    munmap(data, size);

    if (fcntl(fd.get(), F_ADD_SEALS,
              F_SEAL_SEAL | F_SEAL_SHRINK | F_SEAL_GROW | F_SEAL_WRITE)
        != 0) {
        LM_WARNING(lingmoWayland) << "Failed to seal the dmabuf format table";
        return ScopedFd();
    }

    return fd;
}

} // namespace

// A tranche links to a subset of formats and modifiers which are supported on a
// specific device for a specific task. The most common use cases are the
// "default" tranche (targeting the main device, advertising all formats and
// modifiers supported by the compositor, for non-scanout cases) and "scanout"
// tranches (advertising the formats and modifiers on a specific device that,
// if used, allow the compositor to put the buffer straight on a plane).
class DmaBufFeedbackTranche
{
public:
    DmaBufFeedbackTranche(dev_t targetDeviceId, TrancheFlags flags,
                          IndexedDrmFormatsAndModifiers formatsAndModifiers)
        : m_targetDeviceId(targetDeviceId)
        , m_flags(flags)
        , m_formatsAndModifiers(std::move(formatsAndModifiers))
    {
    }

    DmaBufFeedbackTranche(const DmaBufFeedbackTranche&) = delete;
    DmaBufFeedbackTranche& operator=(const DmaBufFeedbackTranche&) = delete;

    dev_t targetDeviceId() const { return m_targetDeviceId; }

    TrancheFlags flags() const { return m_flags; }

    const IndexedDrmFormatsAndModifiers& formatsAndModifiers() const
    {
        return m_formatsAndModifiers;
    }

private:
    const dev_t m_targetDeviceId;
    const TrancheFlags m_flags;
    const IndexedDrmFormatsAndModifiers m_formatsAndModifiers;
};

// A feedback is the set of information sent to a client: a main device (the
// device the compositor renders on), a format table shared through an fd, and
// one or more tranches, of which the default one must always be present. A
// scanout tranche, when present, goes first: clients prefer earlier tranches.
class DmaBufFeedback
{
public:
    DmaBufFeedback(dev_t deviceId, std::unique_ptr<DmaBufFeedbackTranche> defaultTranche)
        : m_mainDeviceId(deviceId)
        , m_defaultTranche(std::move(defaultTranche))
    {
    }

    DmaBufFeedback(const DmaBufFeedback&) = delete;
    DmaBufFeedback& operator=(const DmaBufFeedback&) = delete;

    dev_t mainDeviceId() const { return m_mainDeviceId; }

    const DmaBufFeedbackTranche* defaultTranche() const { return m_defaultTranche.get(); }
    const DmaBufFeedbackTranche* scanoutTranche() const { return m_scanoutTranche.get(); }

    // Returns whether the tranche the client sees changed.
    bool setScanoutTranche(std::unique_ptr<DmaBufFeedbackTranche> tranche)
    {
        const bool changed = (tranche != nullptr) != (m_scanoutTranche != nullptr)
            || (tranche && tranche->formatsAndModifiers() != m_scanoutTranche->formatsAndModifiers());
        m_scanoutTranche = std::move(tranche);
        return changed;
    }

private:
    const dev_t m_mainDeviceId;
    const std::unique_ptr<DmaBufFeedbackTranche> m_defaultTranche;
    std::unique_ptr<DmaBufFeedbackTranche> m_scanoutTranche;
};

// One zwp_linux_dmabuf_feedback_v1 resource of a surface feedback.
class DmaBufSurfaceFeedbackRef
{
public:
    DmaBufSurfaceFeedbackRef(DmaBufSurfaceFeedback* surfaceFeedback, wl_resource* resource);
    DmaBufSurfaceFeedbackRef(const DmaBufSurfaceFeedbackRef&) = delete;
    DmaBufSurfaceFeedbackRef& operator=(const DmaBufSurfaceFeedbackRef&) = delete;
    ~DmaBufSurfaceFeedbackRef();

    wl_resource* resource() const { return m_resource; }
    void setInert() { m_surfaceFeedback = nullptr; }

private:
    DmaBufSurfaceFeedback* m_surfaceFeedback;
    wl_resource* const m_resource;
};

// A client may request the feedback of one surface several times (Mesa does
// implicitly, the application may explicitly), so one surface feedback serves
// any number of resources. It goes with its last resource or with the surface.
class DmaBufSurfaceFeedback : public compositor::SurfaceObserver
{
public:
    DmaBufSurfaceFeedback(DmaBufFeedbackManager* manager, compositor::Surface* surface,
                          std::unique_ptr<DmaBufFeedback> feedback)
        : m_manager(manager)
        , m_surface(surface)
        , m_feedback(std::move(feedback))
    {
        m_surface->addSurfaceObserver(this);
    }

    ~DmaBufSurfaceFeedback() override
    {
        for (DmaBufSurfaceFeedbackRef* ref : m_refs)
            ref->setInert();
        m_surface->removeSurfaceObserver(this);
    }

    void onSurfaceDestroying(compositor::Surface* surface) override
    {
        // Deletes this; the manager drops the candidate through its own
        // observer, so nothing is re-sent for a surface on its way out.
        m_manager->removeSurfaceFeedback(surface);
    }

    void addRef(DmaBufSurfaceFeedbackRef* ref) { m_refs.insert(ref); }

    void removeRef(DmaBufSurfaceFeedbackRef* ref)
    {
        m_refs.erase(ref);
        if (m_refs.empty())
            m_manager->removeSurfaceFeedback(m_surface);
    }

    compositor::Surface* surface() const { return m_surface; }
    DmaBufFeedback* feedback() const { return m_feedback.get(); }
    const std::set<DmaBufSurfaceFeedbackRef*>& refs() const { return m_refs; }

private:
    DmaBufFeedbackManager* const m_manager;
    compositor::Surface* const m_surface;
    const std::unique_ptr<DmaBufFeedback> m_feedback;
    std::set<DmaBufSurfaceFeedbackRef*> m_refs;
};

DmaBufSurfaceFeedbackRef::DmaBufSurfaceFeedbackRef(DmaBufSurfaceFeedback* surfaceFeedback,
                                                   wl_resource* resource)
    : m_surfaceFeedback(surfaceFeedback)
    , m_resource(resource)
{
    m_surfaceFeedback->addRef(this);
}

DmaBufSurfaceFeedbackRef::~DmaBufSurfaceFeedbackRef()
{
    if (m_surfaceFeedback)
        m_surfaceFeedback->removeRef(this);
}

DmaBufFeedbackManager::DmaBufFeedbackManager(compositor::Compositor* compositor,
                                             render::DmaBufDevice* device)
    : m_compositor(compositor)
{
    std::vector<DmaBufFeedbackFormat> table;
    size_t formatTableIndex = 0;

    for (const auto& [drmFormat, modifiers] : device->dmaBufFormats()) {
        std::map<size_t, uint64_t> modifierEntries;
        // The implicit modifier is always importable and always comes first.
        modifierEntries.emplace(formatTableIndex++, DRM_FORMAT_MOD_INVALID);
        table.push_back({ drmFormat, 0, DRM_FORMAT_MOD_INVALID });

        for (uint64_t modifier : modifiers) {
            if (modifier == DRM_FORMAT_MOD_INVALID)
                continue;
            // Check for generic blocking first then format specific blocking.
            if (modifierBlockList().contains({ DRM_FORMAT_INVALID, modifier })
                || modifierBlockList().contains({ drmFormat, modifier })) {
                continue;
            }
            modifierEntries.emplace(formatTableIndex++, modifier);
            table.push_back({ drmFormat, 0, modifier });
        }

        m_formatsAndModifiers.emplace(drmFormat, std::move(modifierEntries));
    }

    const dev_t mainDevice = device->dmaBufMainDevice();
    if (!mainDevice) {
        m_version = ZWP_LINUX_DMABUF_V1_MODIFIER_SINCE_VERSION;
        return;
    }

    m_formatTableFd = createFormatTable(table);
    if (!m_formatTableFd.isValid()) {
        m_version = ZWP_LINUX_DMABUF_V1_MODIFIER_SINCE_VERSION;
        return;
    }
    m_formatTableSize = table.size() * sizeof(DmaBufFeedbackFormat);

    auto tranche = std::make_unique<DmaBufFeedbackTranche>(mainDevice, TrancheFlags::kNone,
                                                           m_formatsAndModifiers);
    m_defaultFeedback = std::make_unique<DmaBufFeedback>(mainDevice, std::move(tranche));

    m_version = ZWP_LINUX_DMABUF_V1_GET_DEFAULT_FEEDBACK_SINCE_VERSION;
}

DmaBufFeedbackManager::~DmaBufFeedbackManager()
{
    // Each surface feedback inerts its resources on the way out.
    m_surfaceFeedbacks.clear();
    for (compositor::Surface* surface : m_scanoutCandidates)
        surface->removeSurfaceObserver(this);
}

bool DmaBufFeedbackManager::isFormatSupported(uint32_t format) const
{
    return m_formatsAndModifiers.contains(format);
}

void DmaBufFeedbackManager::sendFormatsAndModifiers(wl_resource* resource) const
{
    for (const auto& [format, modifierEntries] : m_formatsAndModifiers) {
        zwp_linux_dmabuf_v1_send_format(resource, format);
        if (wl_resource_get_version(resource) >= ZWP_LINUX_DMABUF_V1_MODIFIER_SINCE_VERSION) {
            for (const auto& [tableIndex, modifier] : modifierEntries) {
                zwp_linux_dmabuf_v1_send_modifier(resource, format, modifier >> 32,
                                                  modifier & 0xffffffff);
            }
        }
    }
}

void DmaBufFeedbackManager::getDefaultFeedback(wl_client* client, wl_resource* dmaBufResource,
                                               uint32_t feedbackId)
{
    wl_resource* feedbackResource =
        wl_resource_create(client, &zwp_linux_dmabuf_feedback_v1_interface,
                           wl_resource_get_version(dmaBufResource), feedbackId);
    if (!feedbackResource) {
        wl_client_post_no_memory(client);
        return;
    }

    wl_resource_set_implementation(feedbackResource, &feedback_implementation, nullptr, nullptr);

    sendFeedback(m_defaultFeedback.get(), feedbackResource);
}

void DmaBufFeedbackManager::getSurfaceFeedback(wl_client* client, wl_resource* dmaBufResource,
                                               uint32_t feedbackId, wl_resource* surfaceResource)
{
    compositor::Surface* surface = surfaceFromResource(surfaceResource);
    if (!surface) {
        getDefaultFeedback(client, dmaBufResource, feedbackId);
        return;
    }

    auto it = m_surfaceFeedbacks.find(surface);
    if (it == m_surfaceFeedbacks.end()) {
        auto tranche = std::make_unique<DmaBufFeedbackTranche>(
            m_defaultFeedback->mainDeviceId(), TrancheFlags::kNone, m_formatsAndModifiers);
        auto feedback = std::make_unique<DmaBufFeedback>(m_defaultFeedback->mainDeviceId(),
                                                         std::move(tranche));
        if (m_scanoutCandidates.contains(surface))
            feedback->setScanoutTranche(makeScanoutTranche());
        it = m_surfaceFeedbacks.emplace_hint(
            it, surface,
            std::make_unique<DmaBufSurfaceFeedback>(this, surface, std::move(feedback)));
    }
    DmaBufSurfaceFeedback* surfaceFeedback = it->second.get();

    wl_resource* feedbackResource =
        wl_resource_create(client, &zwp_linux_dmabuf_feedback_v1_interface,
                           wl_resource_get_version(dmaBufResource), feedbackId);
    if (!feedbackResource) {
        wl_client_post_no_memory(client);
        return;
    }

    // The ref removes itself from the surface feedback when the resource dies,
    // which may in turn retire the surface feedback.
    setImplementation(feedbackResource, &feedback_implementation,
                      std::make_unique<DmaBufSurfaceFeedbackRef>(surfaceFeedback,
                                                                 feedbackResource));

    sendFeedback(surfaceFeedback->feedback(), feedbackResource);
}

void DmaBufFeedbackManager::setScanoutFormats(std::function<ScanoutFormats()> provider)
{
    m_scanoutFormats = std::move(provider);
    refreshScanoutTranches();
}

void DmaBufFeedbackManager::setScanoutCandidate(compositor::Surface* surface, bool candidate)
{
    if (!surface)
        return;
    if (candidate ? !m_scanoutCandidates.insert(surface).second
                  : m_scanoutCandidates.erase(surface) == 0) {
        return;
    }
    // A surface that is no candidate any more may already be gone; it is only
    // touched while the set says it is alive.
    if (candidate)
        surface->addSurfaceObserver(this);
    else
        surface->removeSurfaceObserver(this);

    const auto it = m_surfaceFeedbacks.find(surface);
    if (it == m_surfaceFeedbacks.end())
        return;
    if (it->second->feedback()->setScanoutTranche(candidate ? makeScanoutTranche() : nullptr))
        sendSurfaceFeedback(it->second.get());
}

bool DmaBufFeedbackManager::isScanoutCandidate(const compositor::Surface* surface) const
{
    return m_scanoutCandidates.contains(const_cast<compositor::Surface*>(surface));
}

void DmaBufFeedbackManager::onSurfaceDestroying(compositor::Surface* surface)
{
    m_scanoutCandidates.erase(surface);
}

void DmaBufFeedbackManager::refreshScanoutTranches()
{
    for (const auto& [surface, surfaceFeedback] : m_surfaceFeedbacks) {
        if (!m_scanoutCandidates.contains(surface))
            continue;
        if (surfaceFeedback->feedback()->setScanoutTranche(makeScanoutTranche()))
            sendSurfaceFeedback(surfaceFeedback.get());
    }
}

void DmaBufFeedbackManager::removeSurfaceFeedback(compositor::Surface* surface)
{
    m_surfaceFeedbacks.erase(surface);
}

void DmaBufFeedbackManager::sendSurfaceFeedback(DmaBufSurfaceFeedback* surfaceFeedback) const
{
    for (DmaBufSurfaceFeedbackRef* ref : surfaceFeedback->refs())
        sendFeedback(surfaceFeedback->feedback(), ref->resource());
}

std::unique_ptr<DmaBufFeedbackTranche> DmaBufFeedbackManager::makeScanoutTranche() const
{
    if (!m_defaultFeedback || !m_scanoutFormats)
        return nullptr;

    const ScanoutFormats scanoutFormats = m_scanoutFormats();
    IndexedDrmFormatsAndModifiers scanout;
    for (const auto& [format, modifierEntries] : m_formatsAndModifiers) {
        const auto planeIt = scanoutFormats.find(format);
        if (planeIt == scanoutFormats.end())
            continue;

        std::map<size_t, uint64_t> entries;
        if (modifierEntries.size() == 1 || planeIt->second.empty()) {
            // Only the implicit modifier on one side or the other: the driver
            // picks the layout, and a plane without IN_FORMATS takes what its
            // legacy format list implies.
            const auto it = modifierEntries.begin();
            entries.emplace(it->first, it->second);
        } else {
            // With explicit modifiers on both sides the implicit entry is left
            // out: a client picking it would get the driver's default layout,
            // which is the one that cannot be scanned out.
            for (const auto& [tableIndex, modifier] : modifierEntries) {
                if (modifier != DRM_FORMAT_MOD_INVALID
                    && std::find(planeIt->second.begin(), planeIt->second.end(), modifier)
                           != planeIt->second.end()) {
                    entries.emplace(tableIndex, modifier);
                }
            }
        }
        if (!entries.empty())
            scanout.emplace(format, std::move(entries));
    }
    if (scanout.empty())
        return nullptr;

    return std::make_unique<DmaBufFeedbackTranche>(m_defaultFeedback->mainDeviceId(),
                                                   TrancheFlags::kScanout, std::move(scanout));
}

void DmaBufFeedbackManager::sendFeedback(const DmaBufFeedback* feedback,
                                         wl_resource* resource) const
{
    if (!feedback)
        return;

    wl_array mainDeviceBuf;
    wl_array_init(&mainDeviceBuf);
    auto* deviceIdPtr = static_cast<dev_t*>(wl_array_add(&mainDeviceBuf, sizeof(dev_t)));
    *deviceIdPtr = feedback->mainDeviceId();
    zwp_linux_dmabuf_feedback_v1_send_main_device(resource, &mainDeviceBuf);
    wl_array_release(&mainDeviceBuf);

    zwp_linux_dmabuf_feedback_v1_send_format_table(resource, m_formatTableFd.get(),
                                                   m_formatTableSize);

    if (const DmaBufFeedbackTranche* scanout = feedback->scanoutTranche())
        sendTranche(scanout, resource);
    sendTranche(feedback->defaultTranche(), resource);

    zwp_linux_dmabuf_feedback_v1_send_done(resource);
}

void DmaBufFeedbackManager::sendTranche(const DmaBufFeedbackTranche* tranche,
                                        wl_resource* resource) const
{
    wl_array targetDeviceBuf;
    wl_array_init(&targetDeviceBuf);
    auto* deviceIdPtr = static_cast<dev_t*>(wl_array_add(&targetDeviceBuf, sizeof(dev_t)));
    *deviceIdPtr = tranche->targetDeviceId();
    zwp_linux_dmabuf_feedback_v1_send_tranche_target_device(resource, &targetDeviceBuf);
    wl_array_release(&targetDeviceBuf);

    zwp_linux_dmabuf_feedback_v1_send_tranche_flags(resource, tranche->flags());

    wl_array formatsArray;
    wl_array_init(&formatsArray);
    for (const auto& [format, modifierEntries] : tranche->formatsAndModifiers()) {
        for (const auto& [tableIndex, modifier] : modifierEntries) {
            auto* formatIndexPtr =
                static_cast<uint16_t*>(wl_array_add(&formatsArray, sizeof(uint16_t)));
            *formatIndexPtr = static_cast<uint16_t>(tableIndex);
        }
    }
    zwp_linux_dmabuf_feedback_v1_send_tranche_formats(resource, &formatsArray);
    wl_array_release(&formatsArray);

    zwp_linux_dmabuf_feedback_v1_send_tranche_done(resource);
}

} // namespace Lingmo::Wayland
