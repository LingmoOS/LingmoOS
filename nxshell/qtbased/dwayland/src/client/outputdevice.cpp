/*
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "outputdevice.h"
#include "logging.h"
#include "wayland_pointer_p.h"
// Qt
#include <QDebug>
#include <QPoint>
#include <QRect>
// wayland
#include "wayland-org_kde_kwin_outputdevice-client-protocol.h"
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{
typedef QList<OutputDevice::Mode> Modes;

class Q_DECL_HIDDEN OutputDevice::Private
{
public:
    Private(OutputDevice *q);
    void setup(org_kde_kwin_outputdevice *o);

    WaylandPointer<org_kde_kwin_outputdevice, org_kde_kwin_outputdevice_destroy> output;
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
    Modes modes;

    QByteArray edid;
    OutputDevice::Enablement enabled = OutputDevice::Enablement::Enabled;
    QByteArray uuid;

    ColorCurves colorCurves;

    Capabilities capabilities;
    uint32_t overscan = 0;
    VrrPolicy vrrPolicy = VrrPolicy::Automatic;

    bool done = false;

private:
    static void geometryCallback(void *data,
                                 org_kde_kwin_outputdevice *output,
                                 int32_t x,
                                 int32_t y,
                                 int32_t physicalWidth,
                                 int32_t physicalHeight,
                                 int32_t subPixel,
                                 const char *make,
                                 const char *model,
                                 int32_t transform);
    static void modeCallback(void *data, org_kde_kwin_outputdevice *output, uint32_t flags, int32_t width, int32_t height, int32_t refresh, int32_t mode_id);
    static void doneCallback(void *data, org_kde_kwin_outputdevice *output);
    static void scaleCallback(void *data, org_kde_kwin_outputdevice *output, int32_t scale);
    static void scaleFCallback(void *data, org_kde_kwin_outputdevice *output, wl_fixed_t scale);

    static void edidCallback(void *data, org_kde_kwin_outputdevice *output, const char *raw);
    static void enabledCallback(void *data, org_kde_kwin_outputdevice *output, int32_t enabled);
    static void uuidCallback(void *data, org_kde_kwin_outputdevice *output, const char *uuid);

    static void colorcurvesCallback(void *data, org_kde_kwin_outputdevice *output, wl_array *red, wl_array *green, wl_array *blue);

    static void serialNumberCallback(void *data, org_kde_kwin_outputdevice *output, const char *serialNumber);
    static void eisaIdCallback(void *data, org_kde_kwin_outputdevice *output, const char *eisa);
    static void capabilitiesCallback(void *data, org_kde_kwin_outputdevice *output, uint32_t capabilities);
    static void overscanCallback(void *data, org_kde_kwin_outputdevice *output, uint32_t overscan);
    static void vrrPolicyCallback(void *data, org_kde_kwin_outputdevice *output, uint32_t vrrPolicy);

    void setPhysicalSize(const QSize &size);
    void setGlobalPosition(const QPoint &pos);
    void setManufacturer(const QString &manufacturer);
    void setModel(const QString &model);
    void setScale(qreal scale);
    void setSerialNumber(const QString &serialNumber);
    void setEisaId(const QString &eisaId);
    void setSubPixel(SubPixel subPixel);
    void setTransform(Transform transform);
    void addMode(uint32_t flags, int32_t width, int32_t height, int32_t refresh, int32_t mode_id);

    OutputDevice *q;
    static struct org_kde_kwin_outputdevice_listener s_outputListener;
};

OutputDevice::Private::Private(OutputDevice *q)
    : q(q)
{
}

void OutputDevice::Private::setup(org_kde_kwin_outputdevice *o)
{
    Q_ASSERT(o);
    Q_ASSERT(!output);
    output.setup(o);
    org_kde_kwin_outputdevice_add_listener(output, &s_outputListener, this);
}

bool OutputDevice::Mode::operator==(const OutputDevice::Mode &m) const
{
    return size == m.size && refreshRate == m.refreshRate && flags == m.flags && output == m.output;
}

bool OutputDevice::ColorCurves::operator==(const OutputDevice::ColorCurves &cc) const
{
    return red == cc.red && green == cc.green && blue == cc.blue;
}
bool OutputDevice::ColorCurves::operator!=(const ColorCurves &cc) const
{
    return !operator==(cc);
}

OutputDevice::OutputDevice(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

OutputDevice::~OutputDevice()
{
    d->output.release();
}

org_kde_kwin_outputdevice_listener OutputDevice::Private::s_outputListener = {
    geometryCallback,
    modeCallback,
    doneCallback,
    scaleCallback,
    edidCallback,
    enabledCallback,
    uuidCallback,
    scaleFCallback,
    colorcurvesCallback,
    serialNumberCallback,
    eisaIdCallback,
    capabilitiesCallback,
    overscanCallback,
    vrrPolicyCallback
};

void OutputDevice::Private::geometryCallback(void *data,
                                             org_kde_kwin_outputdevice *output,
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
    auto o = reinterpret_cast<OutputDevice::Private *>(data);
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

void OutputDevice::Private::modeCallback(void *data,
                                         org_kde_kwin_outputdevice *output,
                                         uint32_t flags,
                                         int32_t width,
                                         int32_t height,
                                         int32_t refresh,
                                         int32_t mode_id)
{
    auto o = reinterpret_cast<OutputDevice::Private *>(data);
    Q_ASSERT(o->output == output);
    o->addMode(flags, width, height, refresh, mode_id);
}

void OutputDevice::Private::addMode(uint32_t flags, int32_t width, int32_t height, int32_t refresh, int32_t mode_id)
{
    Mode mode;
    mode.output = QPointer<OutputDevice>(q);
    mode.refreshRate = refresh;
    mode.size = QSize(width, height);
    mode.id = mode_id;
    if (flags & WL_OUTPUT_MODE_CURRENT) {
        mode.flags |= Mode::Flag::Current;
    }
    if (flags & WL_OUTPUT_MODE_PREFERRED) {
        mode.flags |= Mode::Flag::Preferred;
    }

    bool existing = false;
    if (flags & WL_OUTPUT_MODE_CURRENT) {
        auto it = modes.begin();
        while (it != modes.end()) {
            auto &m = (*it);
            if (m.flags.testFlag(Mode::Flag::Current)) {
                m.flags &= ~Mode::Flags(Mode::Flag::Current);
                Q_EMIT q->modeChanged(m);
            }
            if (m.refreshRate == mode.refreshRate && m.size == mode.size) {
                it = modes.erase(it);
                existing = true;
            } else {
                it++;
            }
        }
    }

    // insert new mode after erase all repeat old mode
    modes.insert(modes.end(), mode);

    if (existing) {
        Q_EMIT q->modeChanged(mode);
    } else {
        Q_EMIT q->modeAdded(mode);
    }
}

KWayland::Client::OutputDevice::Mode OutputDevice::currentMode() const
{
    for (const auto &m : modes()) {
        if (m.flags.testFlag(KWayland::Client::OutputDevice::Mode::Flag::Current)) {
            return m;
        }
    }
    qCWarning(KWAYLAND_CLIENT) << "current mode not found";
    return Mode();
}

void OutputDevice::Private::scaleCallback(void *data, org_kde_kwin_outputdevice *output, int32_t scale)
{
    auto o = reinterpret_cast<OutputDevice::Private *>(data);
    Q_ASSERT(o->output == output);
    o->setScale(scale);
}

void OutputDevice::Private::scaleFCallback(void *data, org_kde_kwin_outputdevice *output, wl_fixed_t scale_fixed)
{
    auto o = reinterpret_cast<OutputDevice::Private *>(data);
    Q_ASSERT(o->output == output);
    o->setScale(wl_fixed_to_double(scale_fixed));
}

void OutputDevice::Private::doneCallback(void *data, org_kde_kwin_outputdevice *output)
{
    auto o = reinterpret_cast<OutputDevice::Private *>(data);
    Q_ASSERT(o->output == output);
    o->done = true;
    Q_EMIT o->q->changed();
    Q_EMIT o->q->done();
}

void OutputDevice::Private::edidCallback(void *data, org_kde_kwin_outputdevice *output, const char *raw)
{
    Q_UNUSED(output);
    auto o = reinterpret_cast<OutputDevice::Private *>(data);
    o->edid = QByteArray::fromBase64(raw);
}

void OutputDevice::Private::enabledCallback(void *data, org_kde_kwin_outputdevice *output, int32_t enabled)
{
    Q_UNUSED(output);
    auto o = reinterpret_cast<OutputDevice::Private *>(data);

    OutputDevice::Enablement _enabled = OutputDevice::Enablement::Disabled;
    if (enabled == ORG_KDE_KWIN_OUTPUTDEVICE_ENABLEMENT_ENABLED) {
        _enabled = OutputDevice::Enablement::Enabled;
    }
    if (o->enabled != _enabled) {
        o->enabled = _enabled;
        Q_EMIT o->q->enabledChanged(o->enabled);
        if (o->done) {
            Q_EMIT o->q->changed();
        }
    }
}

void OutputDevice::Private::uuidCallback(void *data, org_kde_kwin_outputdevice *output, const char *uuid)
{
    Q_UNUSED(output);
    auto o = reinterpret_cast<OutputDevice::Private *>(data);
    if (o->uuid != uuid) {
        o->uuid = uuid;
        Q_EMIT o->q->uuidChanged(o->uuid);
        if (o->done) {
            Q_EMIT o->q->changed();
        }
    }
}

void OutputDevice::Private::colorcurvesCallback(void *data, org_kde_kwin_outputdevice *output, wl_array *red, wl_array *green, wl_array *blue)
{
    Q_UNUSED(output);
    auto o = reinterpret_cast<OutputDevice::Private *>(data);

    auto cc = ColorCurves();

    auto setCurve = [](const wl_array *curve, QVector<quint16> *destination) {
        destination->resize(curve->size / sizeof(uint16_t));
        memcpy(destination->data(), curve->data, curve->size);
    };
    setCurve(red, &cc.red);
    setCurve(green, &cc.green);
    setCurve(blue, &cc.blue);

    if (o->colorCurves != cc) {
        o->colorCurves = cc;
        Q_EMIT o->q->colorCurvesChanged();
        if (o->done) {
            Q_EMIT o->q->changed();
        }
    }
}

void OutputDevice::Private::serialNumberCallback(void *data, org_kde_kwin_outputdevice *output, const char *raw)
{
    auto o = reinterpret_cast<OutputDevice::Private *>(data);
    Q_UNUSED(output);
    o->setSerialNumber(raw);
}

void OutputDevice::Private::eisaIdCallback(void *data, org_kde_kwin_outputdevice *output, const char *raw)
{
    auto o = reinterpret_cast<OutputDevice::Private *>(data);
    Q_UNUSED(output);
    o->setEisaId(raw);
}

void OutputDevice::Private::capabilitiesCallback(void *data, org_kde_kwin_outputdevice *output, uint32_t capabilities)
{
    auto o = reinterpret_cast<OutputDevice::Private *>(data);
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

void OutputDevice::Private::overscanCallback(void *data, org_kde_kwin_outputdevice *output, uint32_t overscan)
{
    auto o = reinterpret_cast<OutputDevice::Private *>(data);
    Q_UNUSED(output);
    if (o->overscan != overscan) {
        o->overscan = overscan;
        Q_EMIT o->q->overscanChanged(overscan);
        if (o->done) {
            Q_EMIT o->q->changed();
        }
    }
}

void OutputDevice::Private::vrrPolicyCallback(void *data, org_kde_kwin_outputdevice *output, uint32_t vrr_policy)
{
    auto o = reinterpret_cast<OutputDevice::Private*>(data);
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

void OutputDevice::setup(org_kde_kwin_outputdevice *output)
{
    d->setup(output);
}

EventQueue *OutputDevice::eventQueue() const
{
    return d->queue;
}

void OutputDevice::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

void OutputDevice::Private::setGlobalPosition(const QPoint &pos)
{
    globalPosition = pos;
}

void OutputDevice::Private::setManufacturer(const QString &m)
{
    manufacturer = m;
}

void OutputDevice::Private::setModel(const QString &m)
{
    model = m;
}

void OutputDevice::Private::setSerialNumber(const QString &sn)
{
    serialNumber = sn;
}

void OutputDevice::Private::setEisaId(const QString &e)
{
    eisaId = e;
}

void OutputDevice::Private::setPhysicalSize(const QSize &size)
{
    physicalSize = size;
}

void OutputDevice::Private::setScale(qreal s)
{
    scale = s;
}

QRect OutputDevice::geometry() const
{
    if (currentMode() == Mode()) {
        return QRect();
    }
    return QRect(d->globalPosition, currentMode().size);
}

void OutputDevice::Private::setSubPixel(OutputDevice::SubPixel s)
{
    subPixel = s;
}

void OutputDevice::Private::setTransform(OutputDevice::Transform t)
{
    transform = t;
}

QPoint OutputDevice::globalPosition() const
{
    return d->globalPosition;
}

QString OutputDevice::manufacturer() const
{
    return d->manufacturer;
}

QString OutputDevice::model() const
{
    return d->model;
}

QString OutputDevice::serialNumber() const
{
    return d->serialNumber;
}

QString OutputDevice::eisaId() const
{
    return d->eisaId;
}

org_kde_kwin_outputdevice *OutputDevice::output()
{
    return d->output;
}

QSize OutputDevice::physicalSize() const
{
    return d->physicalSize;
}

QSize OutputDevice::pixelSize() const
{
    return currentMode().size;
}

int OutputDevice::refreshRate() const
{
    return currentMode().refreshRate;
}

int OutputDevice::scale() const
{
    return qRound(d->scale);
}

qreal OutputDevice::scaleF() const
{
    return d->scale;
}

bool OutputDevice::isValid() const
{
    return d->output.isValid();
}

OutputDevice::SubPixel OutputDevice::subPixel() const
{
    return d->subPixel;
}

OutputDevice::Transform OutputDevice::transform() const
{
    return d->transform;
}

QList<OutputDevice::Mode> OutputDevice::modes() const
{
    return d->modes;
}

OutputDevice::operator org_kde_kwin_outputdevice *()
{
    return d->output;
}

OutputDevice::operator org_kde_kwin_outputdevice *() const
{
    return d->output;
}

QByteArray OutputDevice::edid() const
{
    return d->edid;
}

OutputDevice::Enablement OutputDevice::enabled() const
{
    return d->enabled;
}

QByteArray OutputDevice::uuid() const
{
    return d->uuid;
}

OutputDevice::ColorCurves OutputDevice::colorCurves() const
{
    return d->colorCurves;
}

OutputDevice::Capabilities OutputDevice::capabilities() const
{
    return d->capabilities;
}

uint32_t OutputDevice::overscan() const
{
    return d->overscan;
}

OutputDevice::VrrPolicy OutputDevice::vrrPolicy() const
{
    return d->vrrPolicy;
}

void OutputDevice::destroy()
{
    d->output.destroy();
}

}
}
