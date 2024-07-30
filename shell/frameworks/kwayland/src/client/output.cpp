/*
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "output.h"
#include "wayland_pointer_p.h"
// Qt
#include <QList>
#include <QPoint>
#include <QRect>
// wayland
#include <wayland-client-protocol.h>

namespace KWayland
{
namespace Client
{
namespace
{
typedef QList<Output::Mode> Modes;
}

class Q_DECL_HIDDEN Output::Private
{
public:
    Private(Output *q);
    ~Private();
    void setup(wl_output *o);

    WaylandPointer<wl_output, wl_output_release> output;
    EventQueue *queue = nullptr;
    QSize physicalSize;
    QPoint globalPosition;
    QString manufacturer;
    QString model;
    int scale = 1;
    SubPixel subPixel = SubPixel::Unknown;
    Transform transform = Transform::Normal;
    Modes modes;
    Modes::iterator currentMode = modes.end();
    QString name;
    QString description;

    static Output *get(wl_output *o);

private:
    static void geometryCallback(void *data,
                                 wl_output *output,
                                 int32_t x,
                                 int32_t y,
                                 int32_t physicalWidth,
                                 int32_t physicalHeight,
                                 int32_t subPixel,
                                 const char *make,
                                 const char *model,
                                 int32_t transform);
    static void modeCallback(void *data, wl_output *output, uint32_t flags, int32_t width, int32_t height, int32_t refresh);
    static void doneCallback(void *data, wl_output *output);
    static void scaleCallback(void *data, wl_output *output, int32_t scale);
    static void nameCallback(void *data, struct wl_output *wl_output, const char *name);
    static void descriptionCallback(void *data, struct wl_output *wl_output, const char *description);
    void setPhysicalSize(const QSize &size);
    void setGlobalPosition(const QPoint &pos);
    void setManufacturer(const QString &manufacturer);
    void setModel(const QString &model);
    void setScale(int scale);
    void setSubPixel(SubPixel subPixel);
    void setTransform(Transform transform);
    void addMode(uint32_t flags, int32_t width, int32_t height, int32_t refresh);

    Output *q;
    static struct wl_output_listener s_outputListener;

    static QList<Private *> s_allOutputs;
};

QList<Output::Private *> Output::Private::s_allOutputs;

Output::Private::Private(Output *q)
    : q(q)
{
    s_allOutputs << this;
}

Output::Private::~Private()
{
    s_allOutputs.removeOne(this);
}

Output *Output::Private::get(wl_output *o)
{
    auto it = std::find_if(s_allOutputs.constBegin(), s_allOutputs.constEnd(), [o](Private *p) {
        const wl_output *reference = p->output;
        return reference == o;
    });
    if (it != s_allOutputs.constEnd()) {
        return (*it)->q;
    }
    return nullptr;
}

void Output::Private::setup(wl_output *o)
{
    Q_ASSERT(o);
    Q_ASSERT(!output);
    output.setup(o);
    wl_output_add_listener(output, &s_outputListener, this);
}

bool Output::Mode::operator==(const Output::Mode &m) const
{
    return size == m.size && refreshRate == m.refreshRate && flags == m.flags && output == m.output;
}

Output::Output(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
}

Output::~Output()
{
    d->output.release();
}

wl_output_listener Output::Private::s_outputListener = {
    geometryCallback, modeCallback, doneCallback, scaleCallback,
#ifdef WL_OUTPUT_NAME_SINCE_VERSION
    nameCallback,
#endif
#ifdef WL_OUTPUT_DESCRIPTION_SINCE_VERSION
    descriptionCallback,
#endif
};

void Output::Private::geometryCallback(void *data,
                                       wl_output *output,
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
    auto o = reinterpret_cast<Output::Private *>(data);
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

void Output::Private::modeCallback(void *data, wl_output *output, uint32_t flags, int32_t width, int32_t height, int32_t refresh)
{
    auto o = reinterpret_cast<Output::Private *>(data);
    Q_ASSERT(o->output == output);
    o->addMode(flags, width, height, refresh);
}

void Output::Private::addMode(uint32_t flags, int32_t width, int32_t height, int32_t refresh)
{
    Mode mode;
    mode.output = QPointer<Output>(q);
    mode.refreshRate = refresh;
    mode.size = QSize(width, height);
    if (flags & WL_OUTPUT_MODE_CURRENT) {
        mode.flags |= Mode::Flag::Current;
    }
    if (flags & WL_OUTPUT_MODE_PREFERRED) {
        mode.flags |= Mode::Flag::Preferred;
    }
    auto currentIt = modes.insert(modes.end(), mode);
    bool existing = false;
    if (flags & WL_OUTPUT_MODE_CURRENT) {
        auto it = modes.begin();
        while (it != currentIt) {
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
        currentMode = currentIt;
    }
    if (existing) {
        Q_EMIT q->modeChanged(mode);
    } else {
        Q_EMIT q->modeAdded(mode);
    }
}

void Output::Private::scaleCallback(void *data, wl_output *output, int32_t scale)
{
    auto o = reinterpret_cast<Output::Private *>(data);
    Q_ASSERT(o->output == output);
    o->setScale(scale);
}

void Output::Private::nameCallback(void *data, struct wl_output *wl_output, const char *name)
{
    auto o = reinterpret_cast<Output::Private *>(data);
    Q_ASSERT(o->output == wl_output);
    o->name = QString::fromUtf8(name);
}

void Output::Private::descriptionCallback(void *data, struct wl_output *wl_output, const char *description)
{
    auto o = reinterpret_cast<Output::Private *>(data);
    Q_ASSERT(o->output == wl_output);
    o->description = QString::fromUtf8(description);
}

void Output::Private::doneCallback(void *data, wl_output *output)
{
    auto o = reinterpret_cast<Output::Private *>(data);
    Q_ASSERT(o->output == output);
    Q_EMIT o->q->changed();
}

void Output::setup(wl_output *output)
{
    d->setup(output);
}

EventQueue *Output::eventQueue() const
{
    return d->queue;
}

void Output::setEventQueue(EventQueue *queue)
{
    d->queue = queue;
}

void Output::Private::setGlobalPosition(const QPoint &pos)
{
    globalPosition = pos;
}

void Output::Private::setManufacturer(const QString &m)
{
    manufacturer = m;
}

void Output::Private::setModel(const QString &m)
{
    model = m;
}

void Output::Private::setPhysicalSize(const QSize &size)
{
    physicalSize = size;
}

void Output::Private::setScale(int s)
{
    scale = s;
}

QRect Output::geometry() const
{
    if (d->currentMode == d->modes.end()) {
        return QRect();
    }
    return QRect(d->globalPosition, pixelSize());
}

void Output::Private::setSubPixel(Output::SubPixel s)
{
    subPixel = s;
}

void Output::Private::setTransform(Output::Transform t)
{
    transform = t;
}

QPoint Output::globalPosition() const
{
    return d->globalPosition;
}

QString Output::manufacturer() const
{
    return d->manufacturer;
}

QString Output::model() const
{
    return d->model;
}

wl_output *Output::output()
{
    return d->output;
}

QSize Output::physicalSize() const
{
    return d->physicalSize;
}

QSize Output::pixelSize() const
{
    if (d->currentMode == d->modes.end()) {
        return QSize();
    }
    return (*d->currentMode).size;
}

int Output::refreshRate() const
{
    if (d->currentMode == d->modes.end()) {
        return 0;
    }
    return (*d->currentMode).refreshRate;
}

int Output::scale() const
{
    return d->scale;
}

bool Output::isValid() const
{
    return d->output.isValid();
}

Output::SubPixel Output::subPixel() const
{
    return d->subPixel;
}

Output::Transform Output::transform() const
{
    return d->transform;
}

QList<Output::Mode> Output::modes() const
{
    return d->modes;
}

QString Output::name() const
{
    return d->name;
}

QString Output::description() const
{
    return d->description;
}

Output::operator wl_output *()
{
    return d->output;
}

Output::operator wl_output *() const
{
    return d->output;
}

Output *Output::get(wl_output *o)
{
    return Private::get(o);
}

void Output::destroy()
{
    d->output.destroy();
}

}
}

#include "moc_output.cpp"
