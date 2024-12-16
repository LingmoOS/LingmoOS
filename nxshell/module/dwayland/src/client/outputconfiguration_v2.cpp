/*
    SPDX-FileCopyrightText: 2022 JccKevin <luochaojiang@uniontech.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "outputconfiguration_v2.h"
#include "event_queue.h"
#include "outputmanagement_v2.h"
#include "wayland_pointer_p.h"

#include "wayland-kde-output-device-v2-client-protocol.h"
#include "wayland-kde-output-management-v2-client-protocol.h"

namespace KWayland
{
namespace Client
{
class Q_DECL_HIDDEN OutputConfigurationV2::Private
{
public:
    Private() = default;

    void setup(kde_output_configuration_v2 *outputconfiguration);

    WaylandPointer<kde_output_configuration_v2, kde_output_configuration_v2_destroy> outputconfiguration;
    static struct kde_output_configuration_v2_listener s_outputconfigurationListener;
    EventQueue *queue = nullptr;

    OutputConfigurationV2 *q;

private:
    static void appliedCallback(void *data, kde_output_configuration_v2 *config);
    static void failedCallback(void *data, kde_output_configuration_v2 *config);
};

OutputConfigurationV2::OutputConfigurationV2(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->q = this;
}

OutputConfigurationV2::~OutputConfigurationV2()
{
    release();
}

void OutputConfigurationV2::setup(kde_output_configuration_v2 *outputconfiguration)
{
    Q_ASSERT(outputconfiguration);
    Q_ASSERT(!d->outputconfiguration);
    d->outputconfiguration.setup(outputconfiguration);
    d->setup(outputconfiguration);
}

void OutputConfigurationV2::Private::setup(kde_output_configuration_v2 *outputconfiguration)
{
    kde_output_configuration_v2_add_listener(outputconfiguration, &s_outputconfigurationListener, this);
}

void OutputConfigurationV2::release()
{
    d->outputconfiguration.release();
}

void OutputConfigurationV2::destroy()
{
    d->outputconfiguration.destroy();
}

void OutputConfigurationV2::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

EventQueue *OutputConfigurationV2::eventQueue()
{
    return d->queue;
}

OutputConfigurationV2::operator kde_output_configuration_v2 *()
{
    return d->outputconfiguration;
}

OutputConfigurationV2::operator kde_output_configuration_v2 *() const
{
    return d->outputconfiguration;
}

bool OutputConfigurationV2::isValid() const
{
    return d->outputconfiguration.isValid();
}

// Requests

void OutputConfigurationV2::setEnabled(OutputDeviceV2 *outputdevice, OutputDeviceV2::Enablement enable)
{
    qint32 _enable = 0;
    if (enable == OutputDeviceV2::Enablement::Enabled) {
        _enable = 1;
    }
    kde_output_device_v2 *od = outputdevice->output();
    kde_output_configuration_v2_enable(d->outputconfiguration, od, _enable);
}

void OutputConfigurationV2::setMode(OutputDeviceV2 *outputdevice, const int modeId)
{
    kde_output_device_v2 *od = outputdevice->output();
    kde_output_configuration_v2_mode(d->outputconfiguration, od, *outputdevice->deviceModeFromId(modeId));
}

void OutputConfigurationV2::setBrightness(OutputDeviceV2 *outputdevice, const int brightness)
{
    kde_output_device_v2 *od = outputdevice->output();
    kde_output_configuration_v2_brightness(d->outputconfiguration, od,
                                          brightness);
}

void OutputConfigurationV2::setTransform(OutputDeviceV2 *outputdevice, KWayland::Client::OutputDeviceV2::Transform transform)
{
    auto toTransform = [transform]() {
        switch (transform) {
            using KWayland::Client::OutputDeviceV2;
        case KWayland::Client::OutputDeviceV2::Transform::Normal:
            return WL_OUTPUT_TRANSFORM_NORMAL;
        case KWayland::Client::OutputDeviceV2::Transform::Rotated90:
            return WL_OUTPUT_TRANSFORM_90;
        case KWayland::Client::OutputDeviceV2::Transform::Rotated180:
            return WL_OUTPUT_TRANSFORM_180;
        case KWayland::Client::OutputDeviceV2::Transform::Rotated270:
            return WL_OUTPUT_TRANSFORM_270;
        case KWayland::Client::OutputDeviceV2::Transform::Flipped:
            return WL_OUTPUT_TRANSFORM_FLIPPED;
        case KWayland::Client::OutputDeviceV2::Transform::Flipped90:
            return WL_OUTPUT_TRANSFORM_FLIPPED_90;
        case KWayland::Client::OutputDeviceV2::Transform::Flipped180:
            return WL_OUTPUT_TRANSFORM_FLIPPED_180;
        case KWayland::Client::OutputDeviceV2::Transform::Flipped270:
            return WL_OUTPUT_TRANSFORM_FLIPPED_270;
        }
        abort();
    };
    kde_output_device_v2 *od = outputdevice->output();
    kde_output_configuration_v2_transform(d->outputconfiguration, od, toTransform());
}

void OutputConfigurationV2::setPosition(OutputDeviceV2 *outputdevice, const QPoint &pos)
{
    kde_output_device_v2 *od = outputdevice->output();
    kde_output_configuration_v2_position(d->outputconfiguration, od, pos.x(), pos.y());
}

void OutputConfigurationV2::setScale(OutputDeviceV2 *outputdevice, qint32 scale)
{
    setScaleF(outputdevice, scale);
}

void OutputConfigurationV2::setScaleF(OutputDeviceV2 *outputdevice, qreal scale)
{
    kde_output_device_v2 *od = outputdevice->output();
    kde_output_configuration_v2_scale(d->outputconfiguration, od, wl_fixed_from_double(scale));
}

void OutputConfigurationV2::setColorCurves(OutputDeviceV2 *outputdevice, QVector<quint16> red, QVector<quint16> green, QVector<quint16> blue)
{
    kde_output_device_v2 *od = outputdevice->output();

    wl_array wlRed;
    wl_array wlGreen;
    wl_array wlBlue;

    auto fillArray = [](QVector<quint16> &origin, wl_array *dest) {
        wl_array_init(dest);
        const size_t memLength = sizeof(uint16_t) * origin.size();
        void *s = wl_array_add(dest, memLength);
        memcpy(s, origin.data(), memLength);
    };
    fillArray(red, &wlRed);
    fillArray(green, &wlGreen);
    fillArray(blue, &wlBlue);

    //kde_output_configuration_v2_colorcurves(d->outputconfiguration, od, &wlRed, &wlGreen, &wlBlue);

    wl_array_release(&wlRed);
    wl_array_release(&wlGreen);
    wl_array_release(&wlBlue);
}

void OutputConfigurationV2::setOverscan(OutputDeviceV2 *outputdevice, uint32_t overscan)
{
    kde_output_device_v2 *od = outputdevice->output();
    if (wl_proxy_get_version(d->outputconfiguration) >= KDE_OUTPUT_CONFIGURATION_V2_OVERSCAN_SINCE_VERSION) {
        kde_output_configuration_v2_overscan(d->outputconfiguration, od, overscan);
    }
}

void OutputConfigurationV2::setVrrPolicy(OutputDeviceV2 *outputdevice, OutputDeviceV2::VrrPolicy policy)
{
    if (wl_proxy_get_version(d->outputconfiguration) >= KDE_OUTPUT_CONFIGURATION_V2_SET_VRR_POLICY_SINCE_VERSION) {
        kde_output_device_v2 *od = outputdevice->output();
        kde_output_configuration_v2_set_vrr_policy(d->outputconfiguration, od, static_cast<uint32_t>(policy));
    }
}

void OutputConfigurationV2::setRgbRange(OutputDeviceV2 *outputdevice, uint32_t rgbRange)
{
    if (wl_proxy_get_version(d->outputconfiguration) >= KDE_OUTPUT_CONFIGURATION_V2_SET_RGB_RANGE_SINCE_VERSION) {
        kde_output_configuration_v2_set_rgb_range(d->outputconfiguration, outputdevice->output(), rgbRange);
    }
}

void OutputConfigurationV2::setPrimaryOutput(OutputDeviceV2 *outputdevice)
{
    if (wl_proxy_get_version(d->outputconfiguration) >= KDE_OUTPUT_CONFIGURATION_V2_SET_PRIMARY_OUTPUT_SINCE_VERSION) {
        kde_output_configuration_v2_set_primary_output(d->outputconfiguration, outputdevice->output());
    }
}

void OutputConfigurationV2::apply()
{
    kde_output_configuration_v2_apply(d->outputconfiguration);
}

// Callbacks
kde_output_configuration_v2_listener OutputConfigurationV2::Private::s_outputconfigurationListener = {appliedCallback, failedCallback};

void OutputConfigurationV2::Private::appliedCallback(void *data, kde_output_configuration_v2 *config)
{
    Q_UNUSED(config);
    auto o = reinterpret_cast<OutputConfigurationV2::Private *>(data);
    Q_EMIT o->q->applied();
}

void OutputConfigurationV2::Private::failedCallback(void *data, kde_output_configuration_v2 *config)
{
    Q_UNUSED(config);
    auto o = reinterpret_cast<OutputConfigurationV2::Private *>(data);
    Q_EMIT o->q->failed();
}

}
}
