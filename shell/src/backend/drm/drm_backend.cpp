// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "backend/drm/drm_backend.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <drm_fourcc.h>
#include <filesystem>
#include <gbm.h>
#include <map>
#include <set>
#include <sys/epoll.h>
#include <utility>
#include <vector>
#include <xf86drm.h>
#include <xf86drmMode.h>

#include "base/event_loop.h"
#include "base/file.h"
#include "base/strings.h"

#include "backend/drm/drm_configuration_planner.h"
#include "backend/drm/drm_device.h"
#include "backend/drm/drm_framebuffer.h"
#include "backend/drm/drm_hotplug_monitor.h"
#include "backend/drm/drm_output.h"
#include "backend/drm/drm_plane.h"
#include "backend/drm/drm_properties.h"
#include "backend/drm/drm_selection.h"
#include "backend/session/session.h"
#include "core/logging.h"
#include "core/main_loop.h"
#include "render/egl_renderer.h"

namespace Lingmo::Backend {

namespace {

// XRGB8888 is the one format every KMS driver can scan out and every EGL
// implementation can render to.
constexpr uint32_t kScanoutFormat = DRM_FORMAT_XRGB8888;

std::string readDriverName(int fd)
{
    drmVersion *version = drmGetVersion(fd);
    if (!version)
        return {};
    std::string name = base::strings::sanitizedUtf8(
            std::string_view(version->name, size_t(std::max(version->name_len, 0))));
    drmFreeVersion(version);
    return name;
}

std::string fileName(const std::string &path)
{
    return std::filesystem::path(path).filename().string();
}

// Whether the firmware showed its boot console on the primary node at `path`.
bool isBootVga(const std::string &path)
{
    const auto contents = base::file::read("/sys/class/drm/" + fileName(path) + "/device/boot_vga");
    return contents && base::strings::trimmed(*contents) == "1";
}

bool drivesDisplays(int fd)
{
    drmModeRes *resources = drmModeGetResources(fd);
    const bool crtcs = resources && resources->count_crtcs > 0;
    drmModeFreeResources(resources);
    return crtcs;
}

} // namespace

DrmBackend::DrmBackend(Session *session)
    : m_session(session)
    , m_format(kScanoutFormat)
{
    m_sessionActive = m_session->activeChanged.connect(
            [this](bool active) { onSessionActiveChanged(active); });
}

void DrmBackend::invalidateOutputs()
{
    ++m_topologyEpoch;
    outputsInvalidated();
}

void DrmBackend::invalidateOutputsLater()
{
    core::mainLoop()->post([this, alive = std::weak_ptr<bool>(m_alive)] {
        if (!alive.expired())
            invalidateOutputs();
    });
}

void DrmBackend::stopWatching()
{
    if (m_watch)
        core::mainLoop()->unwatch(std::exchange(m_watch, 0));
}

DrmBackend::~DrmBackend()
{
    m_devices.clear();
    m_retiredDevices.clear();
    stopWatching();
    for (const auto &output : m_outputs)
        output->retireForReprobe(true);
    for (const auto &output : m_retiringOutputs)
        output->retireForReprobe(false);
    // The device stays open until the framebuffers are gone: closed through it,
    // one still on screen stays there for the next DRM master. Freed by the
    // kernel when the file closes, it would be removed and its plane turned off.
    m_outputs.clear();
    m_retiringOutputs.clear();
    m_renderer.reset();
    closeDevice();
}

void DrmBackend::closeDevice()
{
    stopWatching();

    if (m_gbm) {
        gbm_device_destroy(m_gbm);
        m_gbm = nullptr;
    }
    if (m_deviceId >= 0) {
        m_session->closeDevice(m_deviceId);
        m_deviceId = -1;
    }
    m_fd = -1;
}

bool DrmBackend::open(const std::string &devicePath)
{
    m_autoDevices = devicePath.empty();
    std::vector<std::string> candidates;
    if (!devicePath.empty()) {
        candidates.push_back(devicePath);
    } else {
        drmDevicePtr devices[16] = {};
        const int count = drmGetDevices2(0, devices, 16);
        if (count < 0) {
            LM_CRITICAL(lingmoDrm) << "drmGetDevices2 failed";
            return false;
        }
        for (int i = 0; i < count; ++i) {
            if (devices[i]->available_nodes & (1 << DRM_NODE_PRIMARY))
                candidates.push_back(devices[i]->nodes[DRM_NODE_PRIMARY]);
        }
        drmFreeDevices(devices, count);
    }

    if (candidates.empty()) {
        LM_CRITICAL(lingmoDrm) << "No DRM devices found";
        return false;
    }

    // Every atomic device is opened for its driver name, which sysfs gives
    // only for the bus (virtio-pci for a virtio GPU), as ozone reads it from
    // the fd. A render-only GPU, beside an ARM system's display controller for
    // one, has no CRTC to show anything on; ozone's GetValidDisplayCards leaves
    // it out, and it is taken here only when nothing else is there.
    struct Opened {
        int fd;
        int deviceId;
        bool drivesDisplays;
    };
    std::map<std::string, Opened> opened;
    std::vector<DrmDeviceCandidate> displays, others;
    for (const std::string &path : candidates) {
        int deviceId = -1;
        const int fd = m_session->openDevice(path, &deviceId);
        if (fd < 0)
            continue;
        if (drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1) != 0
            || drmSetClientCap(fd, DRM_CLIENT_CAP_ATOMIC, 1) != 0) {
            m_session->closeDevice(deviceId);
            continue;
        }
        const Opened device {fd, deviceId, drivesDisplays(fd)};
        opened.insert_or_assign(path, device);
        (device.drivesDisplays ? displays : others)
                .push_back({path, readDriverName(fd), isBootVga(path)});
    }
    const std::vector<std::string> preferred =
            orderDisplayDevices(displays.empty() ? others : displays);
    for (const auto &[path, device] : opened) {
        if (preferred.empty() || path != preferred.front())
            m_session->closeDevice(device.deviceId);
    }
    if (!preferred.empty()) {
        const Opened &device = opened.at(preferred.front());
        uint64_t monotonic = 0;
        m_monotonicPresentationClock =
                drmGetCap(device.fd, DRM_CAP_TIMESTAMP_MONOTONIC, &monotonic) == 0
                && monotonic == 1;
        m_fd = device.fd;
        m_deviceId = device.deviceId;
        m_devicePath = preferred.front();
    }

    if (m_fd < 0) {
        LM_CRITICAL(lingmoDrm) << "Failed to open a usable DRM device";
        return false;
    }

    m_driverName = readDriverName(m_fd);
    LM_INFO(lingmoDrm) << "DRM device:" << m_devicePath;
    LM_INFO(lingmoDrm) << "DRM driver:" << m_driverName;

    if (drmSetClientCap(m_fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1) != 0) {
        LM_CRITICAL(lingmoDrm) << "The driver does not support universal planes";
        closeDevice();
        return false;
    }
    if (drmSetClientCap(m_fd, DRM_CLIENT_CAP_ATOMIC, 1) != 0) {
        LM_CRITICAL(lingmoDrm) << "The driver does not support atomic modesetting";
        closeDevice();
        return false;
    }
    LM_INFO(lingmoDrm) << "Atomic modesetting: yes";

    // A secondary device allocates from the rendering card's GBM device, which
    // gbmDevice() hands out; one of its own would be a driver screen never used.
    if (!m_renderBackend) {
        m_gbm = gbm_create_device(m_fd);
        if (!m_gbm) {
            LM_CRITICAL(lingmoDrm) << "Failed to create a GBM device";
            closeDevice();
            return false;
        }
    }

    m_device = std::make_unique<DrmDevice>(m_fd, m_gbm);

    m_watch = core::mainLoop()->watch(m_fd, EPOLLIN, [this](uint32_t) { onDrmEvent(); });

    m_hotplugMonitor = std::make_unique<DrmHotplugMonitor>();
    if (m_hotplugMonitor->start()) {
        m_hotplugSubscriptions.push_back(
                m_hotplugMonitor->snapshotsInvalidated.connect([this] { invalidateOutputs(); }));
        m_hotplugSubscriptions.push_back(
                m_hotplugMonitor->reprobeRequested.connect([this] { invalidateOutputs(); }));
    }

    return true;
}

std::string DrmBackend::deviceKey() const
{
    std::error_code error;
    const auto key = std::filesystem::canonical(
            "/sys/class/drm/" + fileName(m_devicePath) + "/device", error);
    return error || key.empty() ? m_devicePath : key.string();
}

DrmDeviceApi *DrmBackend::device() const
{
    return m_device.get();
}

namespace {

// Reads a plane's IN_FORMATS blob, which is the only place the modifiers a
// plane accepts are published. Without it only the legacy format list is
// available and every modifier is implicit.
//
// Upstream: chromium/src/ui/ozone/platform/drm/gpu/hardware_display_plane.cc
// (HardwareDisplayPlane::Initialize)
void readPlaneFormats(int fd, drmModePlane *plane, const DrmPropertyMap &properties,
                      DrmPlane *target)
{
    for (uint32_t i = 0; i < plane->count_formats; ++i)
        target->formats.emplace(plane->formats[i], std::vector<uint64_t> {});

    const auto inFormats = properties.find("IN_FORMATS");
    if (!inFormats || !inFormats->value)
        return;

    drmModePropertyBlobRes *blob = drmModeGetPropertyBlob(fd, inFormats->value);
    if (!blob)
        return;

    const auto *header = static_cast<const drm_format_modifier_blob *>(blob->data);
    if (blob->length >= sizeof(drm_format_modifier_blob)) {
        const auto *base = static_cast<const uint8_t *>(blob->data);
        const auto *formats = reinterpret_cast<const uint32_t *>(base + header->formats_offset);
        const auto *modifiers =
                reinterpret_cast<const drm_format_modifier *>(base + header->modifiers_offset);

        for (uint32_t m = 0; m < header->count_modifiers; ++m) {
            for (uint32_t bit = 0; bit < 64; ++bit) {
                if (!(modifiers[m].formats & (uint64_t {1} << bit)))
                    continue;
                const uint32_t index = modifiers[m].offset + bit;
                if (index >= header->count_formats)
                    continue;
                target->formats[formats[index]].push_back(modifiers[m].modifier);
            }
        }
    }
    drmModeFreePropertyBlob(blob);
}

// Every usable plane on the device, in ascending id: the plane assignment pass
// relies on that order to keep the requested z order.
std::vector<DrmPlane> enumeratePlanes(int fd)
{
    std::vector<DrmPlane> planes;
    drmModePlaneRes *resources = drmModeGetPlaneResources(fd);
    if (!resources)
        return planes;

    for (uint32_t i = 0; i < resources->count_planes; ++i) {
        drmModePlane *plane = drmModeGetPlane(fd, resources->planes[i]);
        if (!plane)
            continue;

        DrmPropertyMap properties =
                DrmPropertyMap::fromObject(fd, plane->plane_id, DRM_MODE_OBJECT_PLANE);
        if (properties.missing(DrmPlane::requiredProperties()).empty()) {
            DrmPlane entry;
            entry.id = plane->plane_id;
            entry.type = static_cast<uint32_t>(properties.value("type"));
            entry.possibleCrtcs = plane->possible_crtcs;
            entry.properties = properties;
            readPlaneFormats(fd, plane, properties, &entry);
            planes.push_back(std::move(entry));
        }
        drmModeFreePlane(plane);
    }
    drmModeFreePlaneResources(resources);

    std::sort(planes.begin(), planes.end(), [](const DrmPlane &a, const DrmPlane &b) {
        return a.id < b.id;
    });
    return planes;
}

} // namespace

bool DrmBackend::createOutputs()
{
    drmModeRes *resources = drmModeGetResources(m_fd);
    if (!resources) {
        LM_CRITICAL(lingmoDrm) << "Failed to read DRM resources";
        return false;
    }

    std::vector<drmModeConnector *> connectors;
    for (int i = 0; i < resources->count_connectors; ++i) {
        drmModeConnector *connector = drmModeGetConnector(m_fd, resources->connectors[i]);
        if (!connector)
            continue;
        connectors.push_back(connector);
    }

    struct ConnectedConnector {
        drmModeConnector *connector = nullptr;
        std::string name;
        drmModeModeInfo mode = {};
        std::vector<drmModeModeInfo> modes;
        uint32_t possibleCrtcs = 0;
        uint32_t currentCrtc = 0;
    };

    std::vector<ConnectedConnector> connected;
    for (drmModeConnector *connector : connectors) {
        ConnectorInfo info {connector->connector_id, connector->connector_type,
                            connector->connector_type_id, connector->connection,
                            connector->count_modes};
        if (info.connection == DRM_MODE_CONNECTED)
            info.nonDesktop = DrmPropertyMap::fromObject(m_fd, connector->connector_id,
                                                         DRM_MODE_OBJECT_CONNECTOR)
                                      .value("non-desktop");
        if (!isDesktopOutput(info))
            continue;
        auto modes = distinctModes(connector->modes, connector->count_modes);
        const auto mode = selectMode(modes.data(), int(modes.size()));
        if (!mode)
            continue;
        ConnectedConnector entry;
        entry.connector = connector;
        entry.name = connectorName(connector->connector_type, connector->connector_type_id);
        entry.mode = *mode;
        entry.modes = std::move(modes);
        for (int i = 0; i < connector->count_encoders; ++i) {
            drmModeEncoder *encoder = drmModeGetEncoder(m_fd, connector->encoders[i]);
            if (!encoder)
                continue;
            entry.possibleCrtcs |= encoder->possible_crtcs;
            if (encoder->encoder_id == connector->encoder_id)
                entry.currentCrtc = encoder->crtc_id;
            drmModeFreeEncoder(encoder);
        }
        connected.push_back(entry);
    }

    bool ok = false;
    if (connected.empty()) {
        ok = true;
    } else {
        std::vector<DrmPlane> planes = enumeratePlanes(m_fd);
        std::vector<DrmCrtcCandidate> crtcCandidates;
        for (int i = 0; i < resources->count_crtcs; ++i)
            crtcCandidates.push_back({resources->crtcs[i], i, {}});
        for (const DrmPlane &plane : planes) {
            if (plane.type != DRM_PLANE_TYPE_PRIMARY)
                continue;
            for (DrmCrtcCandidate &crtc : crtcCandidates) {
                if (plane.canUseForCrtc(crtc.index))
                    crtc.primaryPlanes.push_back(plane.id);
            }
        }
        std::vector<DrmRouteCandidate> routeCandidates;
        for (const ConnectedConnector &connector : connected)
            routeCandidates.push_back({connector.connector->connector_id, connector.possibleCrtcs,
                                    connector.currentCrtc});
        const auto assignments = DrmConfigurationPlanner::assign(routeCandidates, crtcCandidates,
                                                                 m_routeAlternative);
        if (!assignments) {
            LM_CRITICAL(lingmoDrm) << "No device-wide CRTC and primary-plane assignment";
        } else {
            m_outputs.clear();
            std::set<uint32_t> claimedPlanes;
            for (size_t index = 0; index < connected.size(); ++index) {
                const ConnectedConnector &connector = connected[index];
                const DrmRouteAssignment &assignment = assignments->at(index);
                DrmOutput::Config config;
                config.connectorId = connector.connector->connector_id;
                config.crtcId = assignment.crtcId;
                config.planeId = assignment.primaryPlaneId;
                config.mode = connector.mode;
                config.modes = connector.modes;
                config.name = connector.name;
                config.physicalSizeMm = base::Size(int(connector.connector->mmWidth),
                                                   int(connector.connector->mmHeight));
                config.subpixel = connector.connector->subpixel;
                config.connectorProperties = DrmPropertyMap::fromObject(
                        m_fd, connector.connector->connector_id, DRM_MODE_OBJECT_CONNECTOR);
                const auto edidBlob = config.connectorProperties.value("EDID");
                if (edidBlob) {
                    if (auto *blob = drmModeGetPropertyBlob(m_fd, edidBlob)) {
                        config.edid =
                                std::string(static_cast<const char *>(blob->data), blob->length);
                        drmModeFreePropertyBlob(blob);
                    }
                }
                config.portPath = connector.name;
                if (const auto pathBlob = config.connectorProperties.value("PATH")) {
                    if (auto *blob = drmModeGetPropertyBlob(m_fd, pathBlob)) {
                        std::string path(static_cast<const char *>(blob->data), blob->length);
                        path = base::strings::latin1ToUtf8(path.substr(0, path.find('\0')));
                        drmModeFreePropertyBlob(blob);
                        const size_t separator = path.find('-', 4);
                        if (path.starts_with("mst:") && separator != std::string::npos
                            && separator > 4) {
                            const uint32_t baseId =
                                    base::strings::toNumber<uint32_t>(path.substr(4, separator - 4))
                                            .value_or(0);
                            for (const auto *base : connectors) {
                                if (base->connector_id == baseId)
                                    config.portPath = connectorName(base->connector_type,
                                                                    base->connector_type_id)
                                            + path.substr(separator);
                            }
                        }
                    }
                }
                config.crtcProperties =
                        DrmPropertyMap::fromObject(m_fd, assignment.crtcId, DRM_MODE_OBJECT_CRTC);
                config.crtcIndex = assignment.crtcIndex;
                for (const DrmPlane &plane : planes) {
                    if (plane.id == assignment.primaryPlaneId) {
                        config.planeProperties = plane.properties;
                        config.planes.push_back(plane);
                        claimedPlanes.insert(plane.id);
                    }
                }
                for (const DrmPlane &plane : planes) {
                    if (plane.type == DRM_PLANE_TYPE_PRIMARY || claimedPlanes.contains(plane.id)
                        || !plane.canUseForCrtc(assignment.crtcIndex)) {
                        continue;
                    }
                    config.planes.push_back(plane);
                    claimedPlanes.insert(plane.id);
                }

                std::vector<std::string> missing;
                for (const auto &names :
                     {config.connectorProperties.missing(DrmOutput::requiredConnectorProperties()),
                      config.crtcProperties.missing(DrmOutput::requiredCrtcProperties()),
                      config.planeProperties.missing(DrmOutput::requiredPlaneProperties())}) {
                    missing.insert(missing.end(), names.begin(), names.end());
                }

                if (refreshRate(connector.mode) <= 0.0) {
                    LM_CRITICAL(lingmoDrm) << "Selected mode has invalid refresh timing";
                } else if (!missing.empty()) {
                    LM_CRITICAL(lingmoDrm) << "Missing atomic properties:"
                                       << base::strings::join(missing, ", ");
                } else {
                    LM_INFO(lingmoDrm) << "Connector:" << connector.name;
                    LM_INFO(lingmoDrm).printf("Mode: %dx%d@%.0f", connector.mode.hdisplay,
                                          connector.mode.vdisplay, refreshRate(connector.mode));
                    m_outputs.push_back(std::make_unique<DrmOutput>(this, std::move(config)));
                }
            }
            ok = m_outputs.size() == connected.size();
            if (!ok)
                m_outputs.clear();
        }
    }

    for (drmModeConnector *connector : connectors)
        drmModeFreeConnector(connector);
    drmModeFreeResources(resources);
    return ok;
}

void DrmBackend::retainFlipEvent(std::unique_ptr<DrmOutput::FlipEvent> event)
{
    m_retiredEvents.push_back(std::move(event));
}

bool DrmBackend::reprobe()
{
    if (!m_session->isActive())
        return false;
    m_routeAlternative = 0;
    m_linearAttempt = false;
    if (!m_renderBackend && !refreshDevices())
        return false;
    for (const auto &device : m_devices)
        if (!device->reprobe())
            return false;
    if (!m_session->isActive())
        return false;
    auto *resources = drmModeGetResources(m_fd);
    if (!resources)
        return false;
    std::set<uint32_t> connectors;
    for (int i = 0; i < resources->count_connectors; ++i)
        connectors.insert(resources->connectors[i]);
    drmModeFreeResources(resources);
    for (auto &output : m_outputs)
        m_retiringOutputs.push_back(std::move(output));
    m_outputs.clear();
    // Routes are planned while the old ones are still lit, so the planner keeps
    // each connector on its CRTC and a route that survives is never turned off:
    // no black frame and no panel power cycle on seat activation.
    const bool planned = createOutputs();
    for (const auto &output : m_retiringOutputs) {
        const bool keepRoute =
                planned && std::any_of(m_outputs.begin(), m_outputs.end(), [&](const auto &next) {
                    return next->connectorId() == output->connectorId()
                            && next->crtcId() == output->crtcId();
                });
        if (!output->retireForReprobe(connectors.contains(output->connectorId()), keepRoute)) {
            m_outputs.clear();
            return false;
        }
    }
    m_retiringOutputs.clear();
    if (!planned)
        return false;
    for (const auto &output : m_outputs) {
        if (!output->init(renderer()))
            return false;
    }
    return true;
}

bool DrmBackend::initRenderer()
{
    if (!m_renderBackend)
        m_renderer = render::EglRenderer::create(m_gbm, m_format);
    if (!renderer())
        return false;
    for (const auto &output : m_outputs) {
        if (!output->init(renderer()))
            return false;
    }
    return true;
}

bool DrmBackend::tryNextConfigurationResources()
{
    const auto outputs = allOutputs();
    if (outputs.empty() || std::any_of(outputs.begin(), outputs.end(), [](auto *output) {
            return output->isEnabled();
        }))
        return false;
    if (!m_linearAttempt) {
        m_linearAttempt = true;
        for (auto *output : outputs)
            output->setLinearBuffers(true);
        return true;
    }
    std::vector<DrmBackend *> devices {this};
    for (const auto &device : m_devices)
        devices.push_back(device.get());
    for (auto *device : devices) {
        if (device->m_routeAlternative >= 7)
            continue;
        std::vector<std::string> previous;
        std::vector<DrmOutput *> previousObjects;
        for (const auto &output : device->m_outputs) {
            previous.push_back(output->publicName());
            previousObjects.push_back(output.get());
        }
        ++device->m_routeAlternative;
        if (!device->createOutputs()) {
            --device->m_routeAlternative;
            std::vector<DrmOutput *> remaining;
            for (const auto &output : device->m_outputs)
                remaining.push_back(output.get());
            if (remaining != previousObjects) {
                invalidateOutputs();
                return false;
            }
            continue;
        }
        std::vector<std::string> current;
        for (const auto &output : device->m_outputs) {
            current.push_back(output->publicName());
            if (!output->init(renderer())) {
                invalidateOutputs();
                return false;
            }
        }
        if (current != previous) {
            invalidateOutputs();
            return false;
        }
        m_linearAttempt = false;
        for (auto *output : allOutputs())
            output->setLinearBuffers(false);
        return true;
    }
    return false;
}

std::vector<DrmOutput *> DrmBackend::allOutputs(bool retiring) const
{
    std::vector<DrmOutput *> result;
    for (const auto &output : retiring ? m_retiringOutputs : m_outputs)
        result.push_back(output.get());
    for (const auto &device : m_devices) {
        const auto outputs = device->allOutputs(retiring);
        result.insert(result.end(), outputs.begin(), outputs.end());
    }
    return result;
}

void DrmBackend::retireDevice()
{
    stopWatching();
    m_hotplugSubscriptions.clear();
    m_hotplugMonitor.reset();
    m_sessionActive.reset();
    m_forwardInvalidation.reset();
    if (m_deviceId >= 0) {
        m_session->closeDevice(m_deviceId);
        m_deviceId = -1;
    }
    m_fd = -1;
    if (m_device)
        m_device->invalidate();
    m_outputs.clear();
    m_retiringOutputs.clear();
    m_retiredEvents.clear();
}

bool DrmBackend::refreshDevices()
{
    if (!m_autoDevices || m_renderBackend)
        return true;
    drmDevicePtr enumerated[16] = {};
    const int count = drmGetDevices2(0, enumerated, 16);
    if (count < 0)
        return false;
    std::vector<std::string> paths;
    for (int i = 0; i < count; ++i)
        if (enumerated[i]->available_nodes & (1 << DRM_NODE_PRIMARY))
            paths.push_back(enumerated[i]->nodes[DRM_NODE_PRIMARY]);
    drmFreeDevices(enumerated, count);
    std::sort(paths.begin(), paths.end());
    for (auto it = m_devices.begin(); it != m_devices.end();) {
        if (std::find(paths.begin(), paths.end(), (*it)->devicePath()) != paths.end()) {
            ++it;
            continue;
        }
        (*it)->retireDevice();
        m_retiredDevices.push_back(std::move(*it));
        it = m_devices.erase(it);
    }
    for (const auto &path : paths) {
        if (path == m_devicePath)
            continue;
        const bool known = std::any_of(m_devices.begin(), m_devices.end(), [&](const auto &device) {
            return device->devicePath() == path;
        });
        if (known)
            continue;
        auto device = std::make_unique<DrmBackend>(m_session);
        device->m_renderBackend = this;
        if (!device->open(path) || !device->createOutputs() || !device->initRenderer()) {
            LM_WARNING(lingmoDrm) << "Could not initialize secondary DRM device" << path;
            continue;
        }
        device->m_forwardInvalidation =
                device->outputsInvalidated.connect([this] { invalidateOutputs(); });
        m_devices.push_back(std::move(device));
    }
    return true;
}

void DrmBackend::pageFlipHandler(int, unsigned int sequence, unsigned int tvSec,
                                 unsigned int tvUsec, unsigned int, void *userData)
{
    if (auto *event = static_cast<DrmOutput::FlipEvent *>(userData)) {
        if (event->output)
            DrmOutput::dispatchPageFlip(*event, tvSec, tvUsec, sequence);
        else
            event->delivered = true;
    }
}

void DrmBackend::onDrmEvent()
{
    drmEventContext context = {};
    context.version = 3;
    context.page_flip_handler2 = &DrmBackend::pageFlipHandler;

    errno = 0;
    if (drmHandleEvent(m_fd, &context) != 0 && errno != EINTR && errno != EAGAIN) {
        // As Ozone's DrmDevice::IOWatcher does, stop watching events that cannot
        // be read, rather than spinning on an fd that stays readable.
        LM_WARNING(lingmoDrm).printf("Cannot read DRM events from %s: %s", m_devicePath.c_str(),
                                 strerror(errno));
        stopWatching();
    }
    std::erase_if(m_retiredEvents, [](const auto &event) {
        return event->delivered;
    });
}

void DrmBackend::onSessionActiveChanged(bool active)
{
    if (!active) {
        // The seat is about to go away; stop touching KMS before acknowledging.
        for (const auto &output : m_outputs)
            output->disable();
        return;
    }

    invalidateOutputs();
}

} // namespace Lingmo::Backend
