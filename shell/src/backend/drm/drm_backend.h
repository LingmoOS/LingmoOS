// Copyright 2026 LingmoOS Contributors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINGMO_BACKEND_DRM_DRM_BACKEND_H_
#define LINGMO_BACKEND_DRM_DRM_BACKEND_H_

#include "backend/drm/drm_output.h"
#include "base/signal.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

struct gbm_device;

namespace Lingmo::Render {
class EglRenderer;
}

namespace Lingmo::Backend {

class DrmDevice;
class DrmDeviceApi;
class DrmOutput;
class DrmHotplugMonitor;
class Session;

// Owns the DRM device: discovery, KMS resources and the page-flip event source.
// The rendering card also owns secondary scanout devices and their lifetimes.
class DrmBackend
{
public:
    explicit DrmBackend(Session *session);
    ~DrmBackend();

    DrmBackend(const DrmBackend &) = delete;
    DrmBackend &operator=(const DrmBackend &) = delete;

    // Opens `devicePath`, or the first card supporting atomic modesetting when it
    // is empty. Enables universal planes and atomic modesetting.
    bool open(const std::string &devicePath = {});

    // Builds every connected output after solving shared CRTC and plane use.
    bool createOutputs();
    bool tryNextConfigurationResources();
    bool reprobe();
    void retainFlipEvent(std::unique_ptr<DrmOutput::FlipEvent> event);

    // Creates the EGL renderer on this device's GBM device and initializes
    // every output with it.
    bool initRenderer();
    bool refreshDevices();
    std::vector<DrmOutput *> allOutputs(bool retiring = false) const;

    bool hasMonotonicPresentationClock() const { return m_monotonicPresentationClock; }

    int fd() const { return m_fd; }

    const std::string &devicePath() const { return m_devicePath; }

    std::string deviceKey() const;

    const std::string &driverName() const { return m_driverName; }

    uint64_t topologyEpoch() const { return m_topologyEpoch; }

    gbm_device *gbmDevice() const { return m_renderBackend ? m_renderBackend->gbmDevice() : m_gbm; }

    bool usesForeignRenderer() const { return m_renderBackend != nullptr; }

    uint32_t format() const { return m_format; }

    Session *session() const { return m_session; }

    render::EglRenderer *renderer() const
    {
        return m_renderBackend ? m_renderBackend->renderer() : m_renderer.get();
    }

    // The ioctl surface the display path uses. Valid between open() and
    // destruction.
    DrmDeviceApi *device() const;

    DrmOutput *primaryOutput() const
    {
        const auto outputs = allOutputs();
        return outputs.empty() ? nullptr : outputs.front();
    }

    const std::vector<std::unique_ptr<DrmOutput>> &outputs() const { return m_outputs; }

    const std::vector<std::unique_ptr<DrmOutput>> &retiringOutputs() const
    {
        return m_retiringOutputs;
    }

    // The outputs no longer match the hardware and need a reprobe.
    base::Signal<> outputsInvalidated;
    // Emits outputsInvalidated from the loop, unless this is gone by then.
    void invalidateOutputsLater();

private:
    void invalidateOutputs();
    void stopWatching();
    void onDrmEvent();
    void onSessionActiveChanged(bool active);
    void closeDevice();
    void retireDevice();

    static void pageFlipHandler(int fd, unsigned int sequence, unsigned int tvSec,
                                unsigned int tvUsec, unsigned int crtcId, void *userData);

    Session *m_session = nullptr;
    DrmBackend *m_renderBackend = nullptr;
    bool m_autoDevices = false;
    int m_routeAlternative = 0;
    bool m_linearAttempt = false;
    std::vector<std::unique_ptr<DrmBackend>> m_devices;
    std::vector<std::unique_ptr<DrmBackend>> m_retiredDevices;

    bool m_monotonicPresentationClock = false;
    int m_fd = -1;
    int m_deviceId = -1;
    std::string m_devicePath;
    std::string m_driverName;

    std::unique_ptr<DrmDevice> m_device;
    gbm_device *m_gbm = nullptr;
    uint32_t m_format = 0;

    int m_watch = 0;
    base::Subscription m_sessionActive;
    // A secondary device's outputsInvalidated, forwarded to the rendering card.
    base::Subscription m_forwardInvalidation;
    std::vector<base::Subscription> m_hotplugSubscriptions;
    std::shared_ptr<bool> m_alive = std::make_shared<bool>(true);
    std::unique_ptr<render::EglRenderer> m_renderer;
    std::vector<std::unique_ptr<DrmOutput>> m_outputs;
    std::vector<std::unique_ptr<DrmOutput>> m_retiringOutputs;
    std::unique_ptr<DrmHotplugMonitor> m_hotplugMonitor;
    uint64_t m_topologyEpoch = 0;
    std::vector<std::unique_ptr<DrmOutput::FlipEvent>> m_retiredEvents;
};

} // namespace Lingmo::Backend

#endif // LINGMO_BACKEND_DRM_DRM_BACKEND_H_
