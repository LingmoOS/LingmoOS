/*
    SPDX-FileCopyrightText: 2022 JccKevin <luochaojiang@uniontech.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "outputdevicemode_v2.h"
#include "logging.h"
#include "wayland_pointer_p.h"
// Qt
#include <QDebug>
#include <QPoint>
#include <QRect>
// wayland
#include <wayland-kde-output-device-v2-client-protocol.h>
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{

class Q_DECL_HIDDEN DeviceModeV2::Private
{
public:
    Private(DeviceModeV2 *q, kde_output_device_mode_v2 *m);
    void setup(kde_output_device_mode_v2 *m);

    static DeviceModeV2 *get(kde_output_device_mode_v2 *object);

    WaylandPointer<kde_output_device_mode_v2, kde_output_device_mode_v2_destroy> mode;

    int refreshRate = 60000;
    QSize size;
    bool preferred = false;

private:
    static void handleSizeCallback(void *data, kde_output_device_mode_v2 *object, int32_t width, int32_t height);
    static void handleRefreshCallback(void *data, kde_output_device_mode_v2 *object, int32_t refresh);
    static void handlePreferredCallback(void *data, kde_output_device_mode_v2 *object);
    static void handleRemovedCallback(void *data, kde_output_device_mode_v2 *object);

    void kde_output_device_mode_v2_size(int32_t width, int32_t height);
    void kde_output_device_mode_v2_refresh(int32_t refresh);
    void kde_output_device_mode_v2_preferred();
    void kde_output_device_mode_v2_removed();

    DeviceModeV2 *q;
    static struct kde_output_device_mode_v2_listener s_modeListener;
};

DeviceModeV2::Private::Private(DeviceModeV2 *q, kde_output_device_mode_v2 *m)
    : q(q)
{
    setup(m);
}

void DeviceModeV2::Private::setup(kde_output_device_mode_v2 *m)
{
    Q_ASSERT(m);
    Q_ASSERT(!mode);
    mode.setup(m);
    kde_output_device_mode_v2_add_listener(m, &s_modeListener, this);
}

kde_output_device_mode_v2_listener DeviceModeV2::Private::s_modeListener = {
    handleSizeCallback,
    handleRefreshCallback,
    handlePreferredCallback,
    handleRemovedCallback
};

void DeviceModeV2::Private::handleSizeCallback(void *data, kde_output_device_mode_v2 *object, int32_t width, int32_t height)
{
    Q_UNUSED(object);
    auto m = reinterpret_cast<DeviceModeV2::Private *>(data);
    m->kde_output_device_mode_v2_size(width, height);
}

void DeviceModeV2::Private::handleRefreshCallback(void *data, kde_output_device_mode_v2 *object, int32_t refresh)
{
    Q_UNUSED(object);
    auto m = reinterpret_cast<DeviceModeV2::Private *>(data);
    m->kde_output_device_mode_v2_refresh(refresh);
}

void DeviceModeV2::Private::handlePreferredCallback(void *data, kde_output_device_mode_v2 *object)
{
    Q_UNUSED(object);
    auto m = reinterpret_cast<DeviceModeV2::Private *>(data);
    m->kde_output_device_mode_v2_preferred();
}

void DeviceModeV2::Private::handleRemovedCallback(void *data, kde_output_device_mode_v2 *object)
{
    Q_UNUSED(object);
    auto m = reinterpret_cast<DeviceModeV2::Private *>(data);
    m->kde_output_device_mode_v2_removed();
}

void DeviceModeV2::Private::kde_output_device_mode_v2_size(int32_t width, int32_t height)
{
    size = QSize(width, height);
}

void DeviceModeV2::Private::kde_output_device_mode_v2_refresh(int32_t refresh)
{
    refreshRate = refresh;
}

void DeviceModeV2::Private::kde_output_device_mode_v2_preferred()
{
    preferred = true;
}

void DeviceModeV2::Private::kde_output_device_mode_v2_removed()
{
    Q_EMIT q->removed();
}

DeviceModeV2 *DeviceModeV2::Private::get(kde_output_device_mode_v2 *object)
{
    return reinterpret_cast<Private *>(kde_output_device_mode_v2_get_user_data(object))->q;
}

DeviceModeV2::DeviceModeV2(QObject *parent, kde_output_device_mode_v2 *m)
    : QObject(parent)
    , d(new Private(this, m))
{
}

DeviceModeV2::~DeviceModeV2()
{
    d->mode.release();
}

int DeviceModeV2::refreshRate() const
{
    return d->refreshRate;
}

QSize DeviceModeV2::size() const
{
    return d->size;
}

bool DeviceModeV2::preferred() const
{
    return d->preferred;
}

bool DeviceModeV2::operator==(const DeviceModeV2 &other)
{
    return d->size == other.d->size && d->refreshRate == other.d->refreshRate && d->preferred == other.d->preferred;
}

DeviceModeV2::operator kde_output_device_mode_v2 *()
{
    return d->mode;
}

DeviceModeV2::operator kde_output_device_mode_v2 *() const
{
    return d->mode;
}

DeviceModeV2 *DeviceModeV2::get(kde_output_device_mode_v2 *object)
{
    return Private::get(object);
}

}
}