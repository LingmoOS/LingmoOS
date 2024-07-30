/*
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *  SPDX-FileCopyrightText: 2021 Méven Car <meven.car@enioka.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "waylandoutputdevice.h"
#include "waylandoutputmanagement.h"

#include "kscreen_kwayland_logging.h"

#include "../utils.h"

#include <mode.h>
#include <output.h>

#include <wayland-server-protocol.h>

#include <utility>

using namespace KScreen;

WaylandOutputDevice::WaylandOutputDevice(int id)
    : QObject()
    , kde_output_device_v2()
    , m_id(id)
{
}

WaylandOutputDevice::~WaylandOutputDevice()
{
    qDeleteAll(m_modes);

    kde_output_device_v2_destroy(object());
}

void WaylandOutputDevice::kde_output_device_v2_geometry(int32_t x,
                                                        int32_t y,
                                                        int32_t physical_width,
                                                        int32_t physical_height,
                                                        int32_t subpixel,
                                                        const QString &make,
                                                        const QString &model,
                                                        int32_t transform)
{
    m_pos = QPoint(x, y);
    m_physicalSize = QSize(physical_width, physical_height);
    m_subpixel = subpixel;
    m_manufacturer = make;
    m_model = model;
    m_transform = transform;
}

void WaylandOutputDevice::kde_output_device_v2_current_mode(struct ::kde_output_device_mode_v2 *mode)
{
    auto m = WaylandOutputDeviceMode::get(mode);

    if (*m == *m_mode) {
        // unchanged
        return;
    }
    m_mode = m;
}

void WaylandOutputDevice::kde_output_device_v2_mode(struct ::kde_output_device_mode_v2 *mode)
{
    WaylandOutputDeviceMode *m = new WaylandOutputDeviceMode(mode);
    // last mode sent is the current one
    m_mode = m;
    m_modes.append(m);

    connect(m, &WaylandOutputDeviceMode::removed, this, [this, m]() {
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
}

OutputPtr WaylandOutputDevice::toKScreenOutput()
{
    OutputPtr output(new Output());
    output->setId(m_id);
    updateKScreenOutput(output);
    return output;
}

Output::Rotation toKScreenRotation(int32_t transform)
{
    switch (transform) {
    case WL_OUTPUT_TRANSFORM_NORMAL:
        return Output::None;
    case WL_OUTPUT_TRANSFORM_90:
        return Output::Left;
    case WL_OUTPUT_TRANSFORM_180:
        return Output::Inverted;
    case WL_OUTPUT_TRANSFORM_270:
        return Output::Right;
    case WL_OUTPUT_TRANSFORM_FLIPPED:
        return Output::Flipped;
    case WL_OUTPUT_TRANSFORM_FLIPPED_90:
        return Output::Flipped90;
    case WL_OUTPUT_TRANSFORM_FLIPPED_180:
        return Output::Flipped180;
    case WL_OUTPUT_TRANSFORM_FLIPPED_270:
        return Output::Flipped270;
    default:
        Q_UNREACHABLE();
    }
}

wl_output_transform toKWaylandTransform(const Output::Rotation rotation)
{
    switch (rotation) {
    case Output::None:
        return WL_OUTPUT_TRANSFORM_NORMAL;
    case Output::Left:
        return WL_OUTPUT_TRANSFORM_90;
    case Output::Inverted:
        return WL_OUTPUT_TRANSFORM_180;
    case Output::Right:
        return WL_OUTPUT_TRANSFORM_270;
    case Output::Flipped:
        return WL_OUTPUT_TRANSFORM_FLIPPED;
    case Output::Flipped90:
        return WL_OUTPUT_TRANSFORM_FLIPPED_90;
    case Output::Flipped180:
        return WL_OUTPUT_TRANSFORM_FLIPPED_180;
    case Output::Flipped270:
        return WL_OUTPUT_TRANSFORM_FLIPPED_270;
    default:
        Q_UNREACHABLE();
    }
}

void KScreen::WaylandOutputDevice::updateKScreenModes(OutputPtr &output)
{
    ModeList modeList;
    QStringList preferredModeIds;
    QString currentModeId = QStringLiteral("-1");
    int modeId = 0;

    for (const WaylandOutputDeviceMode *wlMode : std::as_const(m_modes)) {
        ModePtr mode(new Mode());

        const QString modeIdStr = QString::number(modeId);
        // KWayland gives the refresh rate as int in mHz
        mode->setId(modeIdStr);
        mode->setRefreshRate(wlMode->refreshRate() / 1000.0);
        mode->setSize(wlMode->size());
        mode->setName(modeName(wlMode));

        if (m_mode == wlMode) {
            currentModeId = modeIdStr;
        }

        if (wlMode->preferred()) {
            preferredModeIds << modeIdStr;
        }

        // Add to the modelist which gets set on the output
        modeList[modeIdStr] = mode;
        modeId++;
    }
    output->setCurrentModeId(currentModeId);
    output->setPreferredModes(preferredModeIds);
    output->setModes(modeList);
}

void WaylandOutputDevice::updateKScreenOutput(OutputPtr &output)
{
    // Initialize primary output
    output->setId(m_id);
    output->setEnabled(enabled());
    output->setConnected(true);
    output->setName(name());
    output->setSizeMm(m_physicalSize);
    output->setPos(m_pos);
    output->setRotation(toKScreenRotation(m_transform));
    if (!output->edid()) {
        output->setEdid(m_edid);
    }

    QSize currentSize = m_mode->size();
    output->setSize(output->isHorizontal() ? currentSize : currentSize.transposed());
    output->setScale(m_factor);
    output->setType(Utils::guessOutputType(m_outputName, m_outputName));
    output->setCapabilities(static_cast<Output::Capabilities>(static_cast<uint32_t>(m_capabilities)));
    output->setOverscan(m_overscan);
    output->setVrrPolicy(static_cast<Output::VrrPolicy>(m_vrr_policy));
    output->setRgbRange(static_cast<Output::RgbRange>(m_rgbRange));
    output->setHdrEnabled(m_hdrEnabled);
    output->setSdrBrightness(m_sdrBrightness);
    output->setWcgEnabled(m_wideColorGamutEnabled);
    output->setAutoRotatePolicy(static_cast<Output::AutoRotatePolicy>(m_autoRotatePolicy));
    output->setIccProfilePath(m_iccProfilePath);
    output->setSdrGamutWideness(m_sdrGamutWideness);
    output->setMaxPeakBrightness(m_maxPeakBrightness);
    output->setMaxAverageBrightness(m_maxAverageBrightness);
    output->setMinBrightness(m_minBrightness);
    output->setMaxPeakBrightnessOverride(m_maxPeakBrightnessOverride);
    output->setMaxAverageBrightnessOverride(m_maxAverageBrightnessOverride);
    output->setMinBrightnessOverride(m_minBrightnessOverride);
    output->setColorProfileSource(static_cast<Output::ColorProfileSource>(m_colorProfileSource));
    output->setBrightness(m_brightness / 10'000.0);

    updateKScreenModes(output);
}

QString WaylandOutputDevice::modeId() const
{
    return QString::number(m_modes.indexOf(m_mode));
}

WaylandOutputDeviceMode *WaylandOutputDevice::deviceModeFromId(const int modeId) const
{
    return m_modes.at(modeId);
}

bool WaylandOutputDevice::setWlConfig(WaylandOutputConfiguration *wlConfig, const KScreen::OutputPtr &output)
{
    bool changed = false;

    // enabled?
    if (enabled() != output->isEnabled()) {
        changed = true;
        wlConfig->enable(object(), output->isEnabled());
    }

    // position
    if (globalPosition() != output->pos()) {
        changed = true;
        wlConfig->position(object(), output->pos().x(), output->pos().y());
    }

    // scale
    if (!qFuzzyCompare(scale(), output->scale())) {
        changed = true;
        wlConfig->scale(object(), wl_fixed_from_double(output->scale()));
    }

    // rotation
    if (toKScreenRotation(m_transform) != output->rotation()) {
        changed = true;
        wlConfig->transform(object(), toKWaylandTransform(output->rotation()));
    }

    // mode
    const ModePtr mode = output->currentMode();
    if (mode->size() != pixelSize() || mode->refreshRate() != refreshRate()) {
        bool toIntOk;
        int modeId = mode->id().toInt(&toIntOk);
        Q_ASSERT(toIntOk);

        changed = true;
        wlConfig->mode(object(), deviceModeFromId(modeId)->object());
    }

    // overscan
    if ((output->capabilities() & Output::Capability::Overscan) && overscan() != output->overscan()) {
        wlConfig->overscan(object(), output->overscan());
        changed = true;
    }

    // vrr
    if ((output->capabilities() & Output::Capability::Vrr) && vrrPolicy() != static_cast<uint32_t>(output->vrrPolicy())) {
        wlConfig->set_vrr_policy(object(), static_cast<uint32_t>(output->vrrPolicy()));
        changed = true;
    }

    if ((output->capabilities() & Output::Capability::RgbRange) && rgbRange() != static_cast<uint32_t>(output->rgbRange())) {
        wlConfig->set_rgb_range(object(), static_cast<uint32_t>(output->rgbRange()));
        changed = true;
    }
    if (output->priority() != m_index) {
        changed = true;
    }
    // always send all outputs
    if (kde_output_configuration_v2_get_version(wlConfig->object()) >= KDE_OUTPUT_CONFIGURATION_V2_SET_PRIORITY_SINCE_VERSION) {
        wlConfig->set_priority(object(), output->priority());
    }
    if ((output->capabilities() & Output::Capability::HighDynamicRange) && (m_hdrEnabled == 1) != output->isHdrEnabled()) {
        wlConfig->set_high_dynamic_range(object(), output->isHdrEnabled());
        changed = true;
    }
    if ((output->capabilities() & Output::Capability::HighDynamicRange) && m_sdrBrightness != output->sdrBrightness()) {
        wlConfig->set_sdr_brightness(object(), output->sdrBrightness());
        changed = true;
    }
    if ((output->capabilities() & Output::Capability::WideColorGamut) && (m_wideColorGamutEnabled == 1) != output->isWcgEnabled()) {
        wlConfig->set_wide_color_gamut(object(), output->isWcgEnabled());
        changed = true;
    }
    if ((output->capabilities() & Output::Capability::AutoRotation) && m_autoRotatePolicy != static_cast<uint32_t>(output->autoRotatePolicy())) {
        wlConfig->set_auto_rotate_policy(object(), static_cast<uint32_t>(output->autoRotatePolicy()));
        changed = true;
    }
    if ((output->capabilities() & Output::Capability::IccProfile) && m_iccProfilePath != output->iccProfilePath()) {
        wlConfig->set_icc_profile_path(object(), output->iccProfilePath());
        changed = true;
    }
    const auto version = kde_output_configuration_v2_get_version(wlConfig->object());
    if (version >= KDE_OUTPUT_CONFIGURATION_V2_SET_SDR_GAMUT_WIDENESS_SINCE_VERSION && m_sdrGamutWideness != output->sdrGamutWideness()) {
        wlConfig->set_sdr_gamut_wideness(object(), std::clamp<uint32_t>(std::round(output->sdrGamutWideness() * 10'000), 0, 10'000));
        changed = true;
    }
    if (version >= KDE_OUTPUT_CONFIGURATION_V2_SET_BRIGHTNESS_OVERRIDES_SINCE_VERSION
        && (m_maxPeakBrightnessOverride != output->maxPeakBrightnessOverride() || m_maxAverageBrightnessOverride != output->maxAverageBrightnessOverride()
            || m_minBrightnessOverride != output->minBrightnessOverride())) {
        wlConfig->set_brightness_overrides(object(),
                                           output->maxPeakBrightnessOverride().value_or(-1),
                                           output->maxAverageBrightnessOverride().value_or(-1),
                                           std::round(output->minBrightnessOverride().value_or(-0.000'1) * 10'000.0));
        changed = true;
    }
    if (version >= KDE_OUTPUT_CONFIGURATION_V2_SET_COLOR_PROFILE_SOURCE_SINCE_VERSION
        && static_cast<Output::ColorProfileSource>(m_colorProfileSource) != output->colorProfileSource()) {
        wlConfig->set_color_profile_source(object(), static_cast<uint32_t>(output->colorProfileSource()));
        changed = true;
    }
    if (version >= KDE_OUTPUT_CONFIGURATION_V2_SET_BRIGHTNESS_SINCE_VERSION && m_brightness != uint32_t(std::round(output->brightness() * 10'000))) {
        wlConfig->set_brightness(object(), std::round(output->brightness() * 10'000));
        changed = true;
    }

    return changed;
}

QString WaylandOutputDevice::modeName(const WaylandOutputDeviceMode *m) const
{
    return QString::number(m->size().width()) + QLatin1Char('x') + QString::number(m->size().height()) + QLatin1Char('@')
        + QString::number(qRound(m->refreshRate() / 1000.0));
}

QString WaylandOutputDevice::name() const
{
    return m_outputName;
}

QDebug operator<<(QDebug dbg, const WaylandOutputDevice *output)
{
    dbg << "WaylandOutput(Id:" << output->id() << ", Name:" << QString(output->manufacturer() + QLatin1Char(' ') + output->model()) << ")";
    return dbg;
}

void WaylandOutputDevice::setIndex(uint32_t index)
{
    m_index = index;
}

uint32_t WaylandOutputDevice::index() const
{
    return m_index;
}

void WaylandOutputDevice::kde_output_device_v2_done()
{
    Q_EMIT done();
}

void WaylandOutputDevice::kde_output_device_v2_scale(wl_fixed_t factor)
{
    const double factorAsDouble = wl_fixed_to_double(factor);

    // the fractional scaling protocol only speaks in unit of 120ths
    // using the same scale throughout makes that simpler
    // this also eliminates most loss from wl_fixed
    m_factor = std::round(factorAsDouble * 120) / 120;
}

void WaylandOutputDevice::kde_output_device_v2_edid(const QString &edid)
{
    m_edid = QByteArray::fromBase64(edid.toUtf8());
}

void WaylandOutputDevice::kde_output_device_v2_enabled(int32_t enabled)
{
    m_enabled = enabled;
}

void WaylandOutputDevice::kde_output_device_v2_uuid(const QString &uuid)
{
    m_uuid = uuid;
}

void WaylandOutputDevice::kde_output_device_v2_serial_number(const QString &serialNumber)
{
    m_serialNumber = serialNumber;
}

void WaylandOutputDevice::kde_output_device_v2_eisa_id(const QString &eisaId)
{
    m_eisaId = eisaId;
}

void WaylandOutputDevice::kde_output_device_v2_capabilities(uint32_t flags)
{
    m_capabilities = flags;
}

void WaylandOutputDevice::kde_output_device_v2_overscan(uint32_t overscan)
{
    m_overscan = overscan;
}

void WaylandOutputDevice::kde_output_device_v2_vrr_policy(uint32_t vrr_policy)
{
    m_vrr_policy = vrr_policy;
}

void WaylandOutputDevice::kde_output_device_v2_rgb_range(uint32_t rgb_range)
{
    m_rgbRange = rgb_range;
}

void WaylandOutputDevice::kde_output_device_v2_name(const QString &outputName)
{
    m_outputName = outputName;
}

void WaylandOutputDevice::kde_output_device_v2_high_dynamic_range(uint32_t hdr_enabled)
{
    m_hdrEnabled = hdr_enabled == 1;
    if (version() < KDE_OUTPUT_DEVICE_V2_CAPABILITY_BRIGHTNESS_SINCE_VERSION) {
        // make the capabilities API be consistent with older versions even if the protocol isn't
        if (m_hdrEnabled) {
            m_capabilities |= KDE_OUTPUT_DEVICE_V2_CAPABILITY_BRIGHTNESS;
        } else {
            m_capabilities &= ~KDE_OUTPUT_DEVICE_V2_CAPABILITY_BRIGHTNESS;
        }
    }
}

void WaylandOutputDevice::kde_output_device_v2_sdr_brightness(uint32_t sdr_brightness)
{
    m_sdrBrightness = sdr_brightness;
}

void WaylandOutputDevice::kde_output_device_v2_wide_color_gamut(uint32_t wcg_enabled)
{
    m_wideColorGamutEnabled = wcg_enabled == 1;
}

void WaylandOutputDevice::kde_output_device_v2_auto_rotate_policy(uint32_t policy)
{
    m_autoRotatePolicy = policy;
}

void WaylandOutputDevice::kde_output_device_v2_icc_profile_path(const QString &profile)
{
    m_iccProfilePath = profile;
}

void WaylandOutputDevice::kde_output_device_v2_brightness_metadata(uint32_t max_peak_brightness, uint32_t max_frame_average_brightness, uint32_t min_brightness)
{
    m_maxPeakBrightness = max_peak_brightness;
    m_maxAverageBrightness = max_frame_average_brightness;
    m_minBrightness = min_brightness / 10'000.0;
}

void WaylandOutputDevice::kde_output_device_v2_brightness_overrides(int32_t max_peak_brightness, int32_t max_average_brightness, int32_t min_brightness)
{
    m_maxPeakBrightnessOverride = max_peak_brightness == -1 ? std::nullopt : std::optional(max_peak_brightness);
    m_maxAverageBrightnessOverride = max_average_brightness == -1 ? std::nullopt : std::optional(max_average_brightness);
    m_minBrightnessOverride = min_brightness == -1 ? std::nullopt : std::optional(min_brightness / 10'000.0);
}

void WaylandOutputDevice::kde_output_device_v2_sdr_gamut_wideness(uint32_t value)
{
    m_sdrGamutWideness = value / 10'000.0;
}

void WaylandOutputDevice::kde_output_device_v2_color_profile_source(uint32_t source)
{
    m_colorProfileSource = source;
}

void WaylandOutputDevice::kde_output_device_v2_brightness(uint32_t brightness)
{
    m_brightness = brightness;
}

QByteArray WaylandOutputDevice::edid() const
{
    return m_edid;
}

bool WaylandOutputDevice::enabled() const
{
    return m_enabled;
}

int WaylandOutputDevice::id() const
{
    return m_id;
}

qreal WaylandOutputDevice::scale() const
{
    return m_factor;
}

QString WaylandOutputDevice::manufacturer() const
{
    return m_manufacturer;
}

QString WaylandOutputDevice::model() const
{
    return m_model;
}

QPoint WaylandOutputDevice::globalPosition() const
{
    return m_pos;
}

QSize WaylandOutputDevice::pixelSize() const
{
    return m_mode->size();
}

int WaylandOutputDevice::refreshRate() const
{
    return m_mode->refreshRate();
}

uint32_t WaylandOutputDevice::vrrPolicy() const
{
    return m_vrr_policy;
}

uint32_t WaylandOutputDevice::overscan() const
{
    return m_overscan;
}

uint32_t WaylandOutputDevice::capabilities() const
{
    return m_capabilities;
}

uint32_t WaylandOutputDevice::rgbRange() const
{
    return m_rgbRange;
}

#include "moc_waylandoutputdevice.cpp"
