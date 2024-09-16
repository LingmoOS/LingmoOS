/*
    SPDX-FileCopyrightText: 2022 JccKevin <luochaojiang@uniontech.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "outputdevice_v2.h"
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
class Q_DECL_HIDDEN OutputDeviceV2::Private : public QObject
{
    Q_OBJECT
public:
    Private(OutputDeviceV2 *q);
    void setup(kde_output_device_v2 *o);

    WaylandPointer<kde_output_device_v2, kde_output_device_v2_destroy> output;
    EventQueue *queue = nullptr;
    QSize physicalSize;
    QPoint globalPosition;
    QString manufacturer;
    QString model;
    qreal scale = 1.0;
    QString serialNumber;
    QString eisaId;
    SubPixel subPixel = SubPixel::Unknown;
    Transform transform = Transform::Normal;

    QByteArray edid;
    OutputDeviceV2::Enablement enabled = OutputDeviceV2::Enablement::Enabled;
    QByteArray uuid;

    Capabilities capabilities;
    uint32_t overscan = 0;
    VrrPolicy vrrPolicy = VrrPolicy::Automatic;

    uint32_t rgbRange;
    QString outputName;

    bool done = false;

    DeviceModeV2 *m_mode;
    QList<DeviceModeV2 *> m_modes;

private:
    static void geometryCallback(void *data,
                                 kde_output_device_v2 *output,
                                 int32_t x,
                                 int32_t y,
                                 int32_t physicalWidth,
                                 int32_t physicalHeight,
                                 int32_t subPixel,
                                 const char *make,
                                 const char *model,
                                 int32_t transform);
    static void currentModeCallback(void *data, kde_output_device_v2 *output, kde_output_device_mode_v2 *mode);
    static void modeCallback(void *data, kde_output_device_v2 *output, kde_output_device_mode_v2 *mode);
    static void doneCallback(void *data, kde_output_device_v2 *output);
    static void scaleCallback(void *data, kde_output_device_v2 *output, wl_fixed_t scale);

    static void edidCallback(void *data, kde_output_device_v2 *output, const char *raw);
    static void enabledCallback(void *data, kde_output_device_v2 *output, int32_t enabled);
    static void uuidCallback(void *data, kde_output_device_v2 *output, const char *uuid);

    static void serialNumberCallback(void *data, kde_output_device_v2 *output, const char *serialNumber);
    static void eisaIdCallback(void *data, kde_output_device_v2 *output, const char *eisa);

    static void capabilitiesCallback(void *data, kde_output_device_v2 *output, uint32_t capabilities);
    static void overscanCallback(void *data, kde_output_device_v2 *output, uint32_t overscan);
    static void vrrPolicyCallback(void *data, kde_output_device_v2 *output, uint32_t vrrPolicy);

    static void colorRgbRangeCallback(void *data, kde_output_device_v2 *output, uint32_t rgb_range);
    static void nameCallback(void *data, kde_output_device_v2 *output, const char *name);

    void setPhysicalSize(const QSize &size);
    void setGlobalPosition(const QPoint &pos);
    void setManufacturer(const QString &manufacturer);
    void setModel(const QString &model);
    void setScale(qreal scale);
    void setSerialNumber(const QString &serialNumber);
    void setEisaId(const QString &eisaId);
    void setSubPixel(SubPixel subPixel);
    void setTransform(Transform transform);
    void handleCurrentMode(kde_output_device_mode_v2 *mode);
    void addMode(kde_output_device_mode_v2 *mode);

    OutputDeviceV2 *q;
    static struct kde_output_device_v2_listener s_outputListener;
};

OutputDeviceV2::Private::Private(OutputDeviceV2 *q)
    : QObject(q), q(q)
{
}

void OutputDeviceV2::Private::setup(kde_output_device_v2 *o)
{
    Q_ASSERT(o);
    Q_ASSERT(!output);
    output.setup(o);
    kde_output_device_v2_add_listener(output, &s_outputListener, this);
}

OutputDeviceV2::OutputDeviceV2(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

OutputDeviceV2::~OutputDeviceV2()
{
    d->output.release();
}

kde_output_device_v2_listener OutputDeviceV2::Private::s_outputListener = {
    geometryCallback,
    currentModeCallback,
    modeCallback,
    doneCallback,
    scaleCallback,
    edidCallback,
    enabledCallback,
    uuidCallback,
    serialNumberCallback,
    eisaIdCallback,
    capabilitiesCallback,
    overscanCallback,
    vrrPolicyCallback,
    colorRgbRangeCallback,
    nameCallback
};

void OutputDeviceV2::Private::geometryCallback(void *data,
                                             kde_output_device_v2 *output,
                                             int32_t x,
                                             int32_t y,
                                             int32_t physicalWidth,
                                             int32_t physicalHeight,
                                             int32_t subPixel,
                                             const char *make,
                                             const char *model,
                                             int32_t transform)
{
    Q_UNUSED(transform)
    auto o = reinterpret_cast<OutputDeviceV2::Private *>(data);
    Q_ASSERT(o->output == output);
    o->setGlobalPosition(QPoint(x, y));
    o->setManufacturer(make);
    o->setModel(model);
    o->setPhysicalSize(QSize(physicalWidth, physicalHeight));
    auto toSubPixel = [subPixel]() {
        switch (subPixel) {
        case WL_OUTPUT_SUBPIXEL_NONE:
            return SubPixel::None;
        case WL_OUTPUT_SUBPIXEL_HORIZONTAL_RGB:
            return SubPixel::HorizontalRGB;
        case WL_OUTPUT_SUBPIXEL_HORIZONTAL_BGR:
            return SubPixel::HorizontalBGR;
        case WL_OUTPUT_SUBPIXEL_VERTICAL_RGB:
            return SubPixel::VerticalRGB;
        case WL_OUTPUT_SUBPIXEL_VERTICAL_BGR:
            return SubPixel::VerticalBGR;
        case WL_OUTPUT_SUBPIXEL_UNKNOWN:
        default:
            return SubPixel::Unknown;
        }
    };
    o->setSubPixel(toSubPixel());
    auto toTransform = [transform]() {
        switch (transform) {
        case WL_OUTPUT_TRANSFORM_90:
            return Transform::Rotated90;
        case WL_OUTPUT_TRANSFORM_180:
            return Transform::Rotated180;
        case WL_OUTPUT_TRANSFORM_270:
            return Transform::Rotated270;
        case WL_OUTPUT_TRANSFORM_FLIPPED:
            return Transform::Flipped;
        case WL_OUTPUT_TRANSFORM_FLIPPED_90:
            return Transform::Flipped90;
        case WL_OUTPUT_TRANSFORM_FLIPPED_180:
            return Transform::Flipped180;
        case WL_OUTPUT_TRANSFORM_FLIPPED_270:
            return Transform::Flipped270;
        case WL_OUTPUT_TRANSFORM_NORMAL:
        default:
            return Transform::Normal;
        }
    };
    o->setTransform(toTransform());
}

void OutputDeviceV2::Private::currentModeCallback(void *data, kde_output_device_v2 *output, kde_output_device_mode_v2 *mode)
{
    auto o = reinterpret_cast<OutputDeviceV2::Private *>(data);
    Q_ASSERT(o->output == output);
    o->handleCurrentMode(mode);
}

void OutputDeviceV2::Private::handleCurrentMode(kde_output_device_mode_v2 *mode)
{
    auto m = DeviceModeV2::get(mode);

    if (*m == *m_mode) {
        // unchanged
        return;
    }
    m_mode = m;

    Q_EMIT q->currentModeChanged(m);
}

void OutputDeviceV2::Private::modeCallback(void *data, kde_output_device_v2 *output, kde_output_device_mode_v2 *mode)
{
    auto o = reinterpret_cast<OutputDeviceV2::Private *>(data);
    Q_ASSERT(o->output == output);
    o->addMode(mode);
}

void OutputDeviceV2::Private::addMode(kde_output_device_mode_v2 *mode)
{
    DeviceModeV2 *m = new DeviceModeV2(this, mode);
    // last mode sent is the current one
    m_mode = m;
    m_modes.append(m);

    connect(m, &DeviceModeV2::removed, this, [this, m]() {
        m_modes.removeOne(m);
        if (m_mode == m) {
            if (!m_modes.isEmpty()) {
                m_mode = m_modes.first();
            } else {
                // was last mode
                qFatal("KWaylandBackend: no output modes available anymore, this seems like a compositor bug");
            }
        }

        delete m;
    });
    Q_EMIT q->modeAdded(m);
}

DeviceModeV2* OutputDeviceV2::currentMode() const
{
    return d->m_mode;
}

void OutputDeviceV2::Private::scaleCallback(void *data, kde_output_device_v2 *output, int32_t scale)
{
    auto o = reinterpret_cast<OutputDeviceV2::Private *>(data);
    Q_ASSERT(o->output == output);
    o->setScale(scale);
}

void OutputDeviceV2::Private::doneCallback(void *data, kde_output_device_v2 *output)
{
    auto o = reinterpret_cast<OutputDeviceV2::Private *>(data);
    Q_ASSERT(o->output == output);
    o->done = true;
    Q_EMIT o->q->changed();
    Q_EMIT o->q->done();
}

void OutputDeviceV2::Private::edidCallback(void *data, kde_output_device_v2 *output, const char *raw)
{
    Q_UNUSED(output);
    auto o = reinterpret_cast<OutputDeviceV2::Private *>(data);
    o->edid = QByteArray::fromBase64(raw);
}

void OutputDeviceV2::Private::enabledCallback(void *data, kde_output_device_v2 *output, int32_t enabled)
{
    Q_UNUSED(output);
    auto o = reinterpret_cast<OutputDeviceV2::Private *>(data);

    OutputDeviceV2::Enablement _enabled = OutputDeviceV2::Enablement::Disabled;
    if (enabled) {
        _enabled = OutputDeviceV2::Enablement::Enabled;
    }
    if (o->enabled != _enabled) {
        o->enabled = _enabled;
        Q_EMIT o->q->enabledChanged(o->enabled);
        if (o->done) {
            Q_EMIT o->q->changed();
        }
    }
}

void OutputDeviceV2::Private::uuidCallback(void *data, kde_output_device_v2 *output, const char *uuid)
{
    Q_UNUSED(output);
    auto o = reinterpret_cast<OutputDeviceV2::Private *>(data);
    if (o->uuid != uuid) {
        o->uuid = uuid;
        Q_EMIT o->q->uuidChanged(o->uuid);
        if (o->done) {
            Q_EMIT o->q->changed();
        }
    }
}

void OutputDeviceV2::Private::serialNumberCallback(void *data, kde_output_device_v2 *output, const char *raw)
{
    auto o = reinterpret_cast<OutputDeviceV2::Private *>(data);
    Q_UNUSED(output);
    o->setSerialNumber(raw);
}

void OutputDeviceV2::Private::eisaIdCallback(void *data, kde_output_device_v2 *output, const char *raw)
{
    auto o = reinterpret_cast<OutputDeviceV2::Private *>(data);
    Q_UNUSED(output);
    o->setEisaId(raw);
}

void OutputDeviceV2::Private::capabilitiesCallback(void *data, kde_output_device_v2 *output, uint32_t capabilities)
{
    auto o = reinterpret_cast<OutputDeviceV2::Private *>(data);
    Q_UNUSED(output);
    auto caps = static_cast<Capabilities>(capabilities);
    if (o->capabilities != caps) {
        o->capabilities = caps;
        Q_EMIT o->q->capabilitiesChanged(caps);
        if (o->done) {
            Q_EMIT o->q->changed();
        }
    }
}

void OutputDeviceV2::Private::overscanCallback(void *data, kde_output_device_v2 *output, uint32_t overscan)
{
    auto o = reinterpret_cast<OutputDeviceV2::Private *>(data);
    Q_UNUSED(output);
    if (o->overscan != overscan) {
        o->overscan = overscan;
        Q_EMIT o->q->overscanChanged(overscan);
        if (o->done) {
            Q_EMIT o->q->changed();
        }
    }
}

void OutputDeviceV2::Private::vrrPolicyCallback(void *data, kde_output_device_v2 *output, uint32_t vrr_policy)
{
    auto o = reinterpret_cast<OutputDeviceV2::Private*>(data);
    Q_UNUSED(output);
    auto vrrPolicy = static_cast<VrrPolicy>(vrr_policy);
    if (o->vrrPolicy != vrrPolicy) {
        o->vrrPolicy = vrrPolicy;
        Q_EMIT o->q->vrrPolicyChanged(vrrPolicy);
        if (o->done) {
            Q_EMIT o->q->changed();
        }
    }
}

void OutputDeviceV2::Private::colorRgbRangeCallback(void *data, kde_output_device_v2 *output, uint32_t rgb_range)
{
    auto o = reinterpret_cast<OutputDeviceV2::Private *>(data);
    Q_UNUSED(output);
    o->rgbRange = rgb_range;
}

void OutputDeviceV2::Private::nameCallback(void *data, kde_output_device_v2 *output, const char *name)
{
    auto o = reinterpret_cast<OutputDeviceV2::Private *>(data);
    Q_UNUSED(output);
    o->outputName = name;
}

void OutputDeviceV2::setup(kde_output_device_v2 *output)
{
    d->setup(output);
}

EventQueue *OutputDeviceV2::eventQueue() const
{
    return d->queue;
}

void OutputDeviceV2::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

void OutputDeviceV2::Private::setGlobalPosition(const QPoint &pos)
{
    globalPosition = pos;
}

void OutputDeviceV2::Private::setManufacturer(const QString &m)
{
    manufacturer = m;
}

void OutputDeviceV2::Private::setModel(const QString &m)
{
    model = m;
}

void OutputDeviceV2::Private::setSerialNumber(const QString &sn)
{
    serialNumber = sn;
}

void OutputDeviceV2::Private::setEisaId(const QString &e)
{
    eisaId = e;
}

void OutputDeviceV2::Private::setPhysicalSize(const QSize &size)
{
    physicalSize = size;
}

void OutputDeviceV2::Private::setScale(qreal s)
{
    scale = s;
}

QRect OutputDeviceV2::geometry() const
{
    if (!currentMode()) {
        return QRect();
    }
    return QRect(d->globalPosition, currentMode()->size());
}

void OutputDeviceV2::Private::setSubPixel(OutputDeviceV2::SubPixel s)
{
    subPixel = s;
}

void OutputDeviceV2::Private::setTransform(OutputDeviceV2::Transform t)
{
    transform = t;
}

QPoint OutputDeviceV2::globalPosition() const
{
    return d->globalPosition;
}

QString OutputDeviceV2::manufacturer() const
{
    return d->manufacturer;
}

QString OutputDeviceV2::model() const
{
    return d->model;
}

QString OutputDeviceV2::serialNumber() const
{
    return d->serialNumber;
}

QString OutputDeviceV2::eisaId() const
{
    return d->eisaId;
}

kde_output_device_v2 *OutputDeviceV2::output()
{
    return d->output;
}

QSize OutputDeviceV2::physicalSize() const
{
    return d->physicalSize;
}

QSize OutputDeviceV2::pixelSize() const
{
    return currentMode()->size();
}

int OutputDeviceV2::refreshRate() const
{
    return currentMode()->refreshRate();
}

int OutputDeviceV2::scale() const
{
    return qRound(d->scale);
}

qreal OutputDeviceV2::scaleF() const
{
    return d->scale;
}

bool OutputDeviceV2::isValid() const
{
    return d->output.isValid();
}

OutputDeviceV2::SubPixel OutputDeviceV2::subPixel() const
{
    return d->subPixel;
}

OutputDeviceV2::Transform OutputDeviceV2::transform() const
{
    return d->transform;
}

QList<DeviceModeV2*> OutputDeviceV2::modes() const
{
    return d->m_modes;
}

OutputDeviceV2::operator kde_output_device_v2 *()
{
    return d->output;
}

OutputDeviceV2::operator kde_output_device_v2 *() const
{
    return d->output;
}

QByteArray OutputDeviceV2::edid() const
{
    return d->edid;
}

OutputDeviceV2::Enablement OutputDeviceV2::enabled() const
{
    return d->enabled;
}

QByteArray OutputDeviceV2::uuid() const
{
    return d->uuid;
}

OutputDeviceV2::Capabilities OutputDeviceV2::capabilities() const
{
    return d->capabilities;
}

uint32_t OutputDeviceV2::overscan() const
{
    return d->overscan;
}

OutputDeviceV2::VrrPolicy OutputDeviceV2::vrrPolicy() const
{
    return d->vrrPolicy;
}

uint32_t OutputDeviceV2::rgbRange() const
{
    return d->rgbRange;
}

QString OutputDeviceV2::outputName() const
{
    return d->outputName;
}

void OutputDeviceV2::destroy()
{
    d->output.destroy();
}

DeviceModeV2 *OutputDeviceV2::deviceModeFromId(const int modeId) const
{
    return d->m_modes.at(modeId);
}

}
}

#include "outputdevice_v2.moc"
