/*
    SPDX-FileCopyrightText: 2023 jccKevin <luochaojiang@uniontech.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "outputdevice_v1_interface.h"

#include "display.h"
#include "display_p.h"
#include "utils.h"
#include "utils/common.h"

#include "core/output.h"
#include "workspace.h"

#include <QDebug>
#include <QPointer>
#include <QString>

#include "qwayland-server-org-kde-kwin-outputdevice.h"

using namespace KWin;

namespace KWaylandServer
{

static const quint32 s_version = 4;

static QtWaylandServer::org_kde_kwin_outputdevice::transform kwinTransformToOutputDeviceTransform(Output::Transform transform)
{
    return static_cast<QtWaylandServer::org_kde_kwin_outputdevice::transform>(transform);
}

static QtWaylandServer::org_kde_kwin_outputdevice::subpixel kwinSubPixelToOutputDeviceSubPixel(Output::SubPixel subPixel)
{
    return static_cast<QtWaylandServer::org_kde_kwin_outputdevice::subpixel>(subPixel);
}

static uint32_t kwinCapabilitiesToOutputDeviceCapabilities(Output::Capabilities caps)
{
    uint32_t ret = 0;
    if (caps & Output::Capability::Overscan) {
        ret |= QtWaylandServer::org_kde_kwin_outputdevice::capability_overscan;
    }
    if (caps & Output::Capability::Vrr) {
        ret |= QtWaylandServer::org_kde_kwin_outputdevice::capability_vrr;
    }
    return ret;
}

static QtWaylandServer::org_kde_kwin_outputdevice::vrr_policy kwinVrrPolicyToOutputDeviceVrrPolicy(RenderLoop::VrrPolicy policy)
{
    return static_cast<QtWaylandServer::org_kde_kwin_outputdevice::vrr_policy>(policy);
}

static QtWaylandServer::org_kde_kwin_outputdevice::rgb_range kwinRgbRangeToOutputDeviceRgbRange(Output::RgbRange range)
{
    return static_cast<QtWaylandServer::org_kde_kwin_outputdevice::rgb_range>(range);
}

class OutputDeviceInterfacePrivate : public QtWaylandServer::org_kde_kwin_outputdevice
{
public:
    OutputDeviceInterfacePrivate(OutputDeviceInterface *q, Display *display, KWin::Output *handle);
    ~OutputDeviceInterfacePrivate() override;

    void sendGeometry(Resource *resource);
    void sendNewMode(Resource *resource, OutputDeviceModeInterface *mode);
    void sendCurrentMode(Resource *resource);
    void sendDone(Resource *resource);
    void sendUuid(Resource *resource);
    void sendEdid(Resource *resource);
    void sendEnabled(Resource *resource);
    void sendScale(Resource *resource);
    void sendEisaId(Resource *resource);
    void sendName(Resource *resource);
    void sendSerialNumber(Resource *resource);
    void sendCapabilities(Resource *resource);
    void sendOverscan(Resource *resource);
    void sendVrrPolicy(Resource *resource);
    void sendRgbRange(Resource *resource);
    void sendColorCurves(Resource *resource);
    void sendBrightness(Resource *resource);
    void sendCtmValue(Resource *resource);

    OutputDeviceInterface *q;
    QPointer<Display> m_display;
    KWin::Output *m_handle;

    QSize m_physicalSize;
    QPoint m_globalPosition;
    QString m_manufacturer = QStringLiteral("org.kde.kwin");
    QString m_model = QStringLiteral("none");
    qreal m_scale = 1.0;
    QString m_serialNumber;
    QString m_eisaId;
    QString m_name;
    subpixel m_subPixel = subpixel_unknown;
    transform m_transform = transform_normal;
    QList<OutputDeviceModeInterface *> m_modes;
    OutputDeviceModeInterface *m_currentMode = nullptr;
    OutputDeviceModeInterface *m_preMode = nullptr;
    QByteArray m_edid;
    bool m_enabled = true;
    QUuid m_uuid;
    uint32_t m_capabilities = 0;
    uint32_t m_overscan = 0;
    vrr_policy m_vrrPolicy = vrr_policy_automatic;
    Output::RgbRange m_rgbRange = Output::RgbRange::Automatic;

    int m_brightness = 120;
    Output::ColorCurves m_colorCurves;
    Output::CtmValue m_ctmValue;

    bool m_invalid = false;

protected:
    void org_kde_kwin_outputdevice_bind_resource(Resource *resource) override;
    void org_kde_kwin_outputdevice_destroy_global() override;
};

OutputDeviceInterfacePrivate::OutputDeviceInterfacePrivate(OutputDeviceInterface *q, Display *display, KWin::Output *handle)
    : QtWaylandServer::org_kde_kwin_outputdevice(*display, s_version)
    , q(q)
    , m_display(display)
    , m_handle(handle)
{
    DisplayPrivate *displayPrivate = DisplayPrivate::get(display);
    displayPrivate->outputdevices.append(q);
}

OutputDeviceInterfacePrivate::~OutputDeviceInterfacePrivate()
{
    if (m_display) {
        DisplayPrivate *displayPrivate = DisplayPrivate::get(m_display);
        displayPrivate->outputdevices.removeOne(q);
    }
}

OutputDeviceInterface::OutputDeviceInterface(Display *display, KWin::Output *handle, QObject *parent)
    : QObject(parent)
    , d(new OutputDeviceInterfacePrivate(this, display, handle))
{
    qCDebug(KWIN_CORE) << "outputv1:" << this << " construct device " << handle;
    updateEnabled();
    updateManufacturer();
    updateEdid();
    updateUuid();
    updateModel();
    updatePhysicalSize();
    updateGlobalPosition();
    updateScale();
    updateTransform();
    updateEisaId();
    updateSerialNumber();
    updateSubPixel();
    updateOverscan();
    updateCapabilities();
    updateVrrPolicy();
    updateRgbRange();
    updateName();
    updateModes();

    connect(handle, &Output::geometryChanged,
            this, &OutputDeviceInterface::updateGlobalPosition);
    connect(handle, &Output::scaleChanged,
            this, &OutputDeviceInterface::updateScale);
    connect(handle, &Output::enabledChanged,
            this, &OutputDeviceInterface::updateEnabled);
    connect(handle, &Output::transformChanged,
            this, &OutputDeviceInterface::updateTransform);
    connect(handle, &Output::currentModeChanged,
            this, &OutputDeviceInterface::updateCurrentMode);
    connect(handle, &Output::capabilitiesChanged,
            this, &OutputDeviceInterface::updateCapabilities);
    connect(handle, &Output::overscanChanged,
            this, &OutputDeviceInterface::updateOverscan);
    connect(handle, &Output::vrrPolicyChanged,
            this, &OutputDeviceInterface::updateVrrPolicy);
    connect(handle, &Output::modesChanged,
            this, &OutputDeviceInterface::updateModes);
    connect(handle, &Output::rgbRangeChanged,
            this, &OutputDeviceInterface::updateRgbRange);
    connect(handle, &Output::brightnessChanged,
            this, &OutputDeviceInterface::updateBrightness);
    connect(handle, &Output::ctmValueChanged,
            this, &OutputDeviceInterface::updateCtmValue);
    connect(handle, &Output::colorCurvesChanged,
            this, &OutputDeviceInterface::updateCurvesChanged);
    connect(handle, &Output::doneChanged,
            this, &OutputDeviceInterface::done);

    connect(handle, &QObject::destroyed, this, [this, handle]() {
        qCDebug(KWIN_CORE) << "outputv1:" << this << " rejectdestroy output " << handle;
        d->m_invalid = true;
    });
}

OutputDeviceInterface::~OutputDeviceInterface()
{
    qCDebug(KWIN_CORE) << "outputv1:" << this << " destruct device ";
    d->globalRemove();
}

void OutputDeviceInterface::remove()
{
    if (d->isGlobalRemoved()) {
        return;
    }

    if (d->m_display) {
        DisplayPrivate *displayPrivate = DisplayPrivate::get(d->m_display);
        displayPrivate->outputdevices.removeOne(this);
    }

    d->globalRemove();
}

KWin::Output *OutputDeviceInterface::handle() const
{
    return d->m_handle;
}

bool OutputDeviceInterface::invalid() const
{
    return d->m_invalid;
}

void OutputDeviceInterface::done() {
    for (auto resource : d->resourceMap()) {
        d->sendDone(resource);
    }
}

void OutputDeviceInterfacePrivate::org_kde_kwin_outputdevice_destroy_global()
{
    qCDebug(KWIN_CORE) << "outputv1:" << q << " destroy device ";
    delete q;
}

void OutputDeviceInterfacePrivate::org_kde_kwin_outputdevice_bind_resource(Resource *resource)
{
    sendGeometry(resource);
    sendScale(resource);
    sendEisaId(resource);
    sendName(resource);
    sendSerialNumber(resource);

    for (OutputDeviceModeInterface *mode : std::as_const(m_modes)) {
        sendNewMode(resource, mode);
    }
    sendCurrentMode(resource);
    sendUuid(resource);
    sendEdid(resource);
    sendEnabled(resource);
    sendCapabilities(resource);
    sendOverscan(resource);
    sendVrrPolicy(resource);
    sendRgbRange(resource);
    sendColorCurves(resource);
    sendBrightness(resource);
    sendCtmValue(resource);
    sendDone(resource);
}

void OutputDeviceInterfacePrivate::sendNewMode(Resource *resource, OutputDeviceModeInterface *mode)
{
    qCDebug(KWIN_CORE) << "outputv1:" << q << " resource " << resource << " mode " << mode->modeId();
    send_mode(resource->handle, mode->flags(),
              mode->size().width(), mode->size().height(), mode->refreshRate(), mode->modeId());
}

void OutputDeviceInterfacePrivate::sendCurrentMode(Resource *resource)
{
    qCDebug(KWIN_CORE) << "outputv1:" << q << " resource " << resource
            << " current mode " << m_currentMode->modeId()
            << " size " << m_currentMode->size() << " refreshRate "
            << m_currentMode->refreshRate() << " mode " << m_currentMode->flags();
    send_mode(resource->handle, m_currentMode->flags(),
              m_currentMode->size().width(), m_currentMode->size().height(),
              m_currentMode->refreshRate(), m_currentMode->modeId());
}

void OutputDeviceInterfacePrivate::sendGeometry(Resource *resource)
{
    qCDebug(KWIN_CORE) << "outputv1:" << q << " resource " << resource
            << " m_manufacturer " << m_manufacturer << " m_model " << m_model
            << " position " << m_globalPosition << " psize " << m_physicalSize;
    send_geometry(resource->handle,
                  m_globalPosition.x(),
                  m_globalPosition.y(),
                  m_physicalSize.width(),
                  m_physicalSize.height(),
                  m_subPixel,
                  m_manufacturer,
                  m_model,
                  m_transform);
}

void OutputDeviceInterfacePrivate::sendScale(Resource *resource)
{
    qCDebug(KWIN_CORE) << "outputv1:" << q << " resource " << resource << " m_scale " << m_scale;
    if (wl_resource_get_version(resource->handle) < ORG_KDE_KWIN_OUTPUTDEVICE_SCALEF_SINCE_VERSION) {
        send_scale(resource->handle, qRound(m_scale));
    } else {
        send_scalef(resource->handle, wl_fixed_from_double(m_scale));
    }
}

void OutputDeviceInterfacePrivate::sendSerialNumber(Resource *resource)
{
    qCDebug(KWIN_CORE) << "outputv1:" << q << " resource " << resource << " m_serialNumber " << m_serialNumber;
    if (wl_resource_get_version(resource->handle) >= ORG_KDE_KWIN_OUTPUTDEVICE_SERIAL_NUMBER_SINCE_VERSION) {
        send_serial_number(resource->handle, m_serialNumber);
    }
}

void OutputDeviceInterfacePrivate::sendEisaId(Resource *resource)
{
    qCDebug(KWIN_CORE) << "outputv1:" << q << " resource " << resource << " m_eisaId " << m_eisaId;
    if (wl_resource_get_version(resource->handle) >= ORG_KDE_KWIN_OUTPUTDEVICE_EISA_ID_SINCE_VERSION) {
        send_eisa_id(resource->handle, m_eisaId);
    }
}

void OutputDeviceInterfacePrivate::sendName(Resource *resource)
{
    qCDebug(KWIN_CORE) << "outputv1:" << q << " resource " << resource << " m_name " << m_name;
    send_name(resource->handle, m_name);
}

void OutputDeviceInterfacePrivate::sendDone(Resource *resource)
{
    qCDebug(KWIN_CORE) << "outputv1:" << q << " done resource " << resource;
    send_done(resource->handle);
}

void OutputDeviceInterfacePrivate::sendEdid(Resource *resource)
{
    qCDebug(KWIN_CORE) << "outputv1:" << q << " resource " << resource << " m_edid " << QString::fromStdString(m_edid.toBase64().toStdString());
    send_edid(resource->handle, QString::fromStdString(m_edid.toBase64().toStdString()));
}

void OutputDeviceInterfacePrivate::sendEnabled(Resource *resource)
{
    qCDebug(KWIN_CORE) << "outputv1:" << q << " resource " << resource << " m_enabled " << m_enabled;
    send_enabled(resource->handle, m_enabled);
}

void OutputDeviceInterfacePrivate::sendUuid(Resource *resource)
{
    qCDebug(KWIN_CORE) << "outputv1:" << q << " resource " << resource << " m_uuid " << m_uuid.toString(QUuid::WithoutBraces);
    send_uuid(resource->handle, m_uuid.toString(QUuid::WithoutBraces));
}

void OutputDeviceInterfacePrivate::sendCapabilities(Resource *resource)
{
    qCDebug(KWIN_CORE) << "outputv1:" << q << " resource " << resource << " m_capabilities " << m_capabilities;
    send_capabilities(resource->handle, m_capabilities);
}

void OutputDeviceInterfacePrivate::sendOverscan(Resource *resource)
{
    qCDebug(KWIN_CORE) << "outputv1:" << q << " resource " << resource << " m_overscan " << m_overscan;
    send_overscan(resource->handle, m_overscan);
}

void OutputDeviceInterfacePrivate::sendVrrPolicy(Resource *resource)
{
    qCDebug(KWIN_CORE) << "outputv1:" << q << " resource " << resource << " m_vrrPolicy " << m_vrrPolicy;
    send_vrr_policy(resource->handle, m_vrrPolicy);
}

void OutputDeviceInterfacePrivate::sendRgbRange(Resource *resource)
{
    qCDebug(KWIN_CORE) << "outputv1:" << q << " resource " << resource << " m_rgbRange " << m_rgbRange;
    send_rgb_range(resource->handle, kwinRgbRangeToOutputDeviceRgbRange(m_rgbRange));
}

void OutputDeviceInterfacePrivate::sendColorCurves(Resource *resource)
{
    qCDebug(KWIN_CORE) << "outputv1:" << q << " sendColorCurves resource " << resource
            << " m_colorCurves " << m_colorCurves.red << m_colorCurves.green << m_colorCurves.blue;

    wl_array wlRed, wlGreen, wlBlue;

    auto fillArray = [](const QVector<quint16> &origin, wl_array *dest) {
        wl_array_init(dest);
        const size_t memLength = sizeof(uint16_t) * origin.size();
        void *s = wl_array_add(dest, memLength);
        memcpy(s, origin.data(), memLength);
    };
    fillArray(m_colorCurves.red, &wlRed);
    fillArray(m_colorCurves.green, &wlGreen);
    fillArray(m_colorCurves.blue, &wlBlue);

    org_kde_kwin_outputdevice_send_colorcurves(resource->handle, &wlRed, &wlGreen, &wlBlue);

    wl_array_release(&wlRed);
    wl_array_release(&wlGreen);
    wl_array_release(&wlBlue);
}

void OutputDeviceInterfacePrivate::sendBrightness(Resource *resource)
{
    qCDebug(KWIN_CORE) << "outputv1:" << q << " resource " << resource << " m_brightness " << m_brightness;
    send_brightness(resource->handle, m_brightness);
}

void OutputDeviceInterfacePrivate::sendCtmValue(Resource *resource)
{
    qCDebug(KWIN_CORE) << "outputv1:" << q << " resource " << resource
            << " m_ctmValue " << m_ctmValue.r << m_ctmValue.g << m_ctmValue.b;
    send_ctm(resource->handle, m_ctmValue.r, m_ctmValue.g, m_ctmValue.b);
}

void OutputDeviceInterface::updateGeometry()
{
    const auto clientResources = d->resourceMap();
    for (const auto &resource : clientResources) {
        d->sendGeometry(resource);
    }
}

void OutputDeviceInterface::updatePhysicalSize()
{
    d->m_physicalSize = d->m_handle->physicalSize();
    qCDebug(KWIN_CORE) << "outputv1:" << this << " m_physicalSize " << d->m_physicalSize;
}

void OutputDeviceInterface::updateGlobalPosition()
{
    const QPoint arg = d->m_handle->geometry().topLeft();
    qCDebug(KWIN_CORE) << "outputv1:" << this << " m_globalPosition " << d->m_subPixel << " arg " << arg;
    if (d->m_globalPosition == arg) {
        return;
    }
    d->m_globalPosition = arg;
    updateGeometry();
}

void OutputDeviceInterface::updateManufacturer()
{
    d->m_manufacturer = d->m_handle->manufacturer();
    qCDebug(KWIN_CORE) << "outputv1:" << this << " m_manufacturer " << d->m_manufacturer;
}

void OutputDeviceInterface::updateModel()
{
    d->m_model = d->m_handle->model();
    qCDebug(KWIN_CORE) << "outputv1:" << this << " m_model " << d->m_model;
}

void OutputDeviceInterface::updateSerialNumber()
{
    d->m_serialNumber = d->m_handle->serialNumber();
    qCDebug(KWIN_CORE) << "outputv1:" << this << " m_serialNumber " << d->m_serialNumber;
}

void OutputDeviceInterface::updateEisaId()
{
    d->m_eisaId = d->m_handle->eisaId();
    qCDebug(KWIN_CORE) << "outputv1:" << this << " m_eisaId " << d->m_eisaId;
}

void OutputDeviceInterface::updateName()
{
    d->m_name = d->m_handle->name();
    qCDebug(KWIN_CORE) << "outputv1:" << this << " m_name " << d->m_name;
}

void OutputDeviceInterface::updateSubPixel()
{
    const auto arg = kwinSubPixelToOutputDeviceSubPixel(d->m_handle->subPixel());
    qCDebug(KWIN_CORE) << "outputv1:" << this << " m_subPixel " << d->m_subPixel << " arg " << arg;
    if (d->m_subPixel != arg) {
        d->m_subPixel = arg;
        updateGeometry();
    }
}

void OutputDeviceInterface::updateTransform()
{
    const auto arg = kwinTransformToOutputDeviceTransform(d->m_handle->transform());
    qCDebug(KWIN_CORE) << "outputv1:" << this << " m_transform " << d->m_transform << " arg " << arg;
    if (d->m_transform != arg) {
        d->m_transform = arg;
        updateGeometry();
    }
}

void OutputDeviceInterface::updateScale()
{
    const qreal scale = d->m_handle->scale();
    qCDebug(KWIN_CORE) << "outputv1:" << this << " m_scale " << d->m_scale << " scale " << scale;
    if (qFuzzyCompare(d->m_scale, scale)) {
        return;
    }
    d->m_scale = scale;
    const auto clientResources = d->resourceMap();
    for (const auto &resource : clientResources) {
        d->sendScale(resource);
    }
}

void OutputDeviceInterface::updateModes()
{
    qCDebug(KWIN_CORE) << "outputv1:" << this << " updateModes ";
    const auto oldModes = d->m_modes;
    d->m_modes.clear();
    d->m_currentMode = nullptr;
    d->m_preMode = nullptr;

    const auto clientResources = d->resourceMap();
    const auto nativeModes = d->m_handle->modes();

    int id = 1;
    for (const std::shared_ptr<OutputMode> &mode : nativeModes) {
        OutputDeviceModeInterface *deviceMode = new OutputDeviceModeInterface(mode, id++, this);
        d->m_modes.append(deviceMode);

        if (d->m_handle->currentMode() == mode) {
            d->m_currentMode = deviceMode;
            d->m_currentMode->markCurrent(true);
        }

        for (auto resource : clientResources) {
            d->sendNewMode(resource, deviceMode);
        }
    }

    for (auto resource : clientResources) {
        d->sendCurrentMode(resource);
    }

    qDeleteAll(oldModes.crbegin(), oldModes.crend());
}

void OutputDeviceInterface::updateCurrentMode()
{
    qCDebug(KWIN_CORE) << "outputv1:" << this << " m_currentMode ";
    for (OutputDeviceModeInterface *mode : std::as_const(d->m_modes)) {
        if (mode->m_handle.lock() == d->m_handle->currentMode()) {
            if (d->m_currentMode != mode) {
                d->m_preMode = d->m_currentMode;
                d->m_currentMode = mode;
                d->m_currentMode->markCurrent(true);
                if (d->m_preMode) {
                    d->m_preMode->markCurrent(false);
                }
                const auto clientResources = d->resourceMap();
                for (auto resource : clientResources) {
                    d->sendCurrentMode(resource);
                }
                updateGeometry();
            }
            return;
        }
    }
}

void OutputDeviceInterface::updateEdid()
{
    d->m_edid = d->m_handle->edid();
    qCDebug(KWIN_CORE) << "outputv1:" << this << " m_edid " << d->m_edid;
    const auto clientResources = d->resourceMap();
    for (const auto &resource : clientResources) {
        d->sendEdid(resource);
    }
}

void OutputDeviceInterface::updateEnabled()
{
    bool enabled = d->m_handle->isEnabled();
    qCDebug(KWIN_CORE) << "outputv1:" << this << " m_enabled " << d->m_enabled << " enabled " << enabled;
    if (d->m_enabled != enabled) {
        d->m_enabled = enabled;
        const auto clientResources = d->resourceMap();
        for (const auto &resource : clientResources) {
            d->sendEnabled(resource);
        }
    }
}

void OutputDeviceInterface::updateUuid()
{
    const QUuid uuid = d->m_handle->uuid();
    qCDebug(KWIN_CORE) << "outputv1:" << this << " m_uuid " << d->m_uuid.toString(QUuid::WithoutBraces) << " uuid " << uuid.toString(QUuid::WithoutBraces);
    if (d->m_uuid != uuid) {
        d->m_uuid = uuid;
        const auto clientResources = d->resourceMap();
        for (const auto &resource : clientResources) {
            d->sendUuid(resource);
        }
    }
}

void OutputDeviceInterface::updateCapabilities()
{
    const uint32_t cap = kwinCapabilitiesToOutputDeviceCapabilities(d->m_handle->capabilities());
    qCDebug(KWIN_CORE) << "outputv1:" << this << " m_capabilities " << d->m_capabilities << " cap " << cap;
    if (d->m_capabilities != cap) {
        d->m_capabilities = cap;
        const auto clientResources = d->resourceMap();
        for (const auto &resource : clientResources) {
            d->sendCapabilities(resource);
        }
    }
}

void OutputDeviceInterface::updateOverscan()
{
    const uint32_t overscan = d->m_handle->overscan();
    qCDebug(KWIN_CORE) << "outputv1:" << this << " m_overscan " << d->m_overscan << " overscan " << overscan;
    if (d->m_overscan != overscan) {
        d->m_overscan = overscan;
        const auto clientResources = d->resourceMap();
        for (const auto &resource : clientResources) {
            d->sendOverscan(resource);
        }
    }
}

void OutputDeviceInterface::updateVrrPolicy()
{
    const auto policy = kwinVrrPolicyToOutputDeviceVrrPolicy(d->m_handle->vrrPolicy());
    qCDebug(KWIN_CORE) << "outputv1:" << this << " m_vrrPolicy " << d->m_vrrPolicy << " policy " << policy;
    if (d->m_vrrPolicy != policy) {
        d->m_vrrPolicy = policy;
        const auto clientResources = d->resourceMap();
        for (const auto &resource : clientResources) {
            d->sendVrrPolicy(resource);
        }
    }
}

void OutputDeviceInterface::updateRgbRange()
{
    const auto rgbRange = d->m_handle->rgbRange();
    qCDebug(KWIN_CORE) << "outputv1:" << this << " m_rgbRange " << d->m_rgbRange << " rgbRange " << rgbRange;
    if (d->m_rgbRange != rgbRange) {
        d->m_rgbRange = rgbRange;
        const auto clientResources = d->resourceMap();
        for (const auto &resource : clientResources) {
            d->sendRgbRange(resource);
        }
    }
}

void OutputDeviceInterface::updateBrightness()
{
    int brightness = d->m_handle->brightness();
    qCDebug(KWIN_CORE) << "outputv1:" << this << " m_brightness " << d->m_brightness << " brightness " << brightness;
    if (d->m_brightness != brightness) {
        d->m_brightness = brightness;
        const auto clientResources = d->resourceMap();
        for (const auto &resource : clientResources) {
            d->sendBrightness(resource);
        }
    }
}

void OutputDeviceInterface::updateCtmValue()
{
    Output::CtmValue ctm = d->m_handle->ctmValue();
    qCDebug(KWIN_CORE) << "outputv1:" << this << " ctm " << ctm.r << ctm.g << ctm.b;
    if (d->m_ctmValue != ctm) {
        d->m_ctmValue = ctm;
        const auto clientResources = d->resourceMap();
        for (const auto &resource : clientResources) {
            d->sendCtmValue(resource);
        }
    }
}

void OutputDeviceInterface::updateCurvesChanged()
{
    Output::ColorCurves colorCurves = d->m_handle->colorCurves();
    qCDebug(KWIN_CORE) << "outputv1:" << this << " ctm " << colorCurves.red << colorCurves.green << colorCurves.blue;
    if (d->m_colorCurves != colorCurves) {
        d->m_colorCurves = colorCurves;
        const auto clientResources = d->resourceMap();
        for (const auto &resource : clientResources) {
            d->sendColorCurves(resource);
        }
    }
}

OutputDeviceInterface *OutputDeviceInterface::get(wl_resource *native)
{
    if (auto devicePrivate = resource_cast<OutputDeviceInterfacePrivate *>(native); devicePrivate && !devicePrivate->isGlobalRemoved()) {
        return devicePrivate->q;
    }
    return nullptr;
}

OutputDeviceModeInterface *OutputDeviceInterface::getMode(int modeId)
{
    for (OutputDeviceModeInterface *mode : std::as_const(d->m_modes)) {
        if (mode->modeId() == modeId) {
            return mode;
        }
    }
    return nullptr;
}

OutputDeviceModeInterface::OutputDeviceModeInterface(std::shared_ptr<KWin::OutputMode> handle, int id, QObject *parent)
    : QObject(parent)
    , m_handle(handle)
    , m_modeId(id)
    , m_size(handle->size())
    , m_refreshRate(handle->refreshRate())
    , m_preferred(handle->flags() & OutputMode::Flag::Preferred)
{
    if (m_preferred) {
        m_flags |= ModeFlag::Preferred;
    }
}

OutputDeviceModeInterface::~OutputDeviceModeInterface() = default;

}

#include "outputdevice_v1_interface.moc"
