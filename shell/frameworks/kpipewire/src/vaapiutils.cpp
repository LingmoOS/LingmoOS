/*
    SPDX-FileCopyrightText: 2023 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "vaapiutils_p.h"
#include <logging_vaapi.h>

#include <QDir>

extern "C" {
#include <drm_fourcc.h>
#include <fcntl.h>
#include <unistd.h>
#include <va/va_drm.h>
#include <xf86drm.h>
}

VaapiUtils::VaapiUtils(VaapiUtils::Private)
{
    int max_devices = drmGetDevices2(0, nullptr, 0);
    if (max_devices <= 0) {
        qCWarning(PIPEWIREVAAPI_LOGGING) << "drmGetDevices2() has not found any devices (errno=" << -max_devices << ")";
        return;
    }

    std::vector<drmDevicePtr> devices(max_devices);
    int ret = drmGetDevices2(0, devices.data(), max_devices);
    if (ret < 0) {
        qCWarning(PIPEWIREVAAPI_LOGGING) << "drmGetDevices2() returned an error " << ret;
        return;
    }

    for (const drmDevicePtr &device : devices) {
        if (device->available_nodes & (1 << DRM_NODE_RENDER)) {
            QByteArray fullPath = device->nodes[DRM_NODE_RENDER];
            if (supportsH264(fullPath)) {
                m_devicePath = fullPath;
                break;
            }
        }
    }

    drmFreeDevices(devices.data(), ret);

    if (m_devicePath.isEmpty()) {
        qCWarning(PIPEWIREVAAPI_LOGGING) << "DRM device not found";
    }
}

VaapiUtils::~VaapiUtils()
{
}

bool VaapiUtils::supportsProfile(VAProfile profile)
{
    if (m_devicePath.isEmpty()) {
        return false;
    }
    bool ret = false;

    int drmFd = -1;

    VADisplay vaDpy = openDevice(&drmFd, m_devicePath);
    if (!vaDpy) {
        return false;
    }

    ret = supportsProfile(profile, vaDpy, m_devicePath);

    closeDevice(&drmFd, vaDpy);

    return ret;
}

bool VaapiUtils::supportsH264(const QByteArray &path) const
{
    if (path.isEmpty()) {
        return false;
    }
    bool ret = false;

    int drmFd = -1;

    VADisplay vaDpy = openDevice(&drmFd, path);
    if (!vaDpy) {
        return false;
    }

    const char *driver = vaQueryVendorString(vaDpy);
    qCWarning(PIPEWIREVAAPI_LOGGING) << "VAAPI:" << driver << "in use for device" << path;

    ret = supportsProfile(VAProfileH264ConstrainedBaseline, vaDpy, path) || supportsProfile(VAProfileH264Main, vaDpy, path)
        || supportsProfile(VAProfileH264High, vaDpy, path);

    querySizeConstraints(vaDpy);

    closeDevice(&drmFd, vaDpy);

    return ret;
}

QByteArray VaapiUtils::devicePath()
{
    return m_devicePath;
}

QSize VaapiUtils::minimumSize() const
{
    return m_minSize;
}

QSize VaapiUtils::maximumSize() const
{
    return m_maxSize;
}

bool VaapiUtils::supportsModifier(uint32_t /*format*/, uint64_t modifier)
{
    // For now, we have no way of querying VAAPI for what modifiers are
    // actually supported for encoding. So assume we can only support linear
    // buffers for now, even though some cards may actually also support
    // other formats.
    //
    // As of 8/4/24, we know that on AMD chips using the RadeonSI driver, frames
    // using "Delta Color Compression" modifier will be rejected by the driver.
    // Also, Intel chips using the Interl iHD Media driver will accept any
    // modifier but internally force using LINEAR so any modifier other than
    // LINEAR should be rejected.
    //
    // See https://github.com/intel/libva/pull/589 for discussion surrounding
    // API in LibVA for querying supported modifiers.
    return modifier == DRM_FORMAT_MOD_LINEAR;
}

std::shared_ptr<VaapiUtils> VaapiUtils::instance()
{
    static std::shared_ptr<VaapiUtils> instance = std::make_shared<VaapiUtils>(VaapiUtils::Private{});
    return instance;
}

VADisplay VaapiUtils::openDevice(int *fd, const QByteArray &path)
{
    VADisplay vaDpy;

    if (path.isEmpty()) {
        return NULL;
    }

    *fd = open(path.data(), O_RDWR);
    if (*fd < 0) {
        qCWarning(PIPEWIREVAAPI_LOGGING) << "VAAPI: Failed to open device" << path;
        return NULL;
    }

    vaDpy = vaGetDisplayDRM(*fd);
    if (!vaDpy) {
        qCWarning(PIPEWIREVAAPI_LOGGING) << "VAAPI: Failed to initialize DRM display";
        return NULL;
    }

    if (vaDisplayIsValid(vaDpy) == 0) {
        qCWarning(PIPEWIREVAAPI_LOGGING) << "Invalid VA display";
        vaTerminate(vaDpy);
        return NULL;
    }

    int major, minor;
    VAStatus va_status = vaInitialize(vaDpy, &major, &minor);

    if (va_status != VA_STATUS_SUCCESS) {
        qCWarning(PIPEWIREVAAPI_LOGGING) << "VAAPI: Failed to initialize display";
        return NULL;
    }

    qCInfo(PIPEWIREVAAPI_LOGGING) << "VAAPI: API version" << major << "." << minor;
    qCInfo(PIPEWIREVAAPI_LOGGING) << "VAAPI: Display initialized";

    return vaDpy;
}

void VaapiUtils::closeDevice(int *fd, VADisplay dpy)
{
    vaTerminate(dpy);
    if (*fd < 0) {
        return;
    }

    close(*fd);
    *fd = -1;
}

bool VaapiUtils::supportsProfile(VAProfile profile, VADisplay dpy, const QByteArray &path)
{
    uint32_t ret = rateControlForProfile(profile, VAEntrypointEncSlice, dpy, path);

    if (ret & VA_RC_CBR || ret & VA_RC_CQP || ret & VA_RC_VBR) {
        return true;
    } else {
        ret = rateControlForProfile(profile, VAEntrypointEncSliceLP, dpy, path);

        if (ret & VA_RC_CBR || ret & VA_RC_CQP || ret & VA_RC_VBR) {
            return true;
        }
    }

    return false;
}

uint32_t VaapiUtils::rateControlForProfile(VAProfile profile, VAEntrypoint entrypoint, VADisplay dpy, const QByteArray &path)
{
    VAStatus va_status;
    VAConfigAttrib attrib[1];
    attrib->type = VAConfigAttribRateControl;

    va_status = vaGetConfigAttributes(dpy, profile, entrypoint, attrib, 1);

    switch (va_status) {
    case VA_STATUS_SUCCESS:
        return attrib->value;
    case VA_STATUS_ERROR_UNSUPPORTED_PROFILE:
        qCWarning(PIPEWIREVAAPI_LOGGING) << "VAAPI: profile" << profile << "is not supported by the device" << path;
        return 0;
    case VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT:
        qCWarning(PIPEWIREVAAPI_LOGGING) << "VAAPI: entrypoint" << entrypoint << "of profile" << profile << "is not supported by the device" << path;
        return 0;
    default:
        qCWarning(PIPEWIREVAAPI_LOGGING) << "VAAPI: Fail to get RC attribute from the" << profile << entrypoint << "of the device" << path;
        return 0;
    }
}

void VaapiUtils::querySizeConstraints(VADisplay dpy) const
{
    VAConfigID config;
    if (auto status = vaCreateConfig(dpy, VAProfileH264ConstrainedBaseline, VAEntrypointEncSlice, nullptr, 0, &config); status != VA_STATUS_SUCCESS) {
        return;
    }

    VASurfaceAttrib attrib[8];
    uint32_t attribCount = 8;

    auto status = vaQuerySurfaceAttributes(dpy, config, attrib, &attribCount);
    if (status == VA_STATUS_SUCCESS) {
        for (uint32_t i = 0; i < attribCount; ++i) {
            switch (attrib[i].type) {
            case VASurfaceAttribMinWidth:
                m_minSize.setWidth(attrib[i].value.value.i);
                break;
            case VASurfaceAttribMinHeight:
                m_minSize.setHeight(attrib[i].value.value.i);
                break;
            case VASurfaceAttribMaxWidth:
                m_maxSize.setWidth(attrib[i].value.value.i);
                break;
            case VASurfaceAttribMaxHeight:
                m_maxSize.setHeight(attrib[i].value.value.i);
                break;
            default:
                break;
            }
        }
    }

    vaDestroyConfig(dpy, config);
}
