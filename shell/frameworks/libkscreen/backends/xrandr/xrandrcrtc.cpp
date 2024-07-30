/*
 * SPDX-FileCopyrightText: 2015 Daniel Vrátil <dvratil@redhat.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 *
 */
#include "xrandrcrtc.h"

#include "xrandr.h"
#include "xrandrconfig.h"

#include "../xcbwrapper.h"

XRandRCrtc::XRandRCrtc(xcb_randr_crtc_t crtc, XRandRConfig *config)
    : QObject(config)
    , m_crtc(crtc)
    , m_mode(0)
    , m_rotation(XCB_RANDR_ROTATION_ROTATE_0)
    , m_timestamp(XCB_CURRENT_TIME)
    , m_configTimestamp(XCB_CURRENT_TIME)
{
    update();
}

xcb_randr_crtc_t XRandRCrtc::crtc() const
{
    return m_crtc;
}

xcb_randr_mode_t XRandRCrtc::mode() const
{
    return m_mode;
}

QRect XRandRCrtc::geometry() const
{
    return m_geometry;
}

xcb_randr_rotation_t XRandRCrtc::rotation() const
{
    return m_rotation;
}

QList<xcb_randr_output_t> XRandRCrtc::possibleOutputs()
{
    return m_possibleOutputs;
}

QList<xcb_randr_output_t> XRandRCrtc::outputs() const
{
    return m_outputs;
}

bool XRandRCrtc::connectOutput(xcb_randr_output_t output)
{
    update();
    qCDebug(KSCREEN_XRANDR) << "Connected output" << output << "to CRTC" << m_crtc;

    if (!m_possibleOutputs.contains(output)) {
        qCDebug(KSCREEN_XRANDR) << "Output" << output << "is not an allowed output for CRTC" << m_crtc;
        return false;
    }

    if (!m_outputs.contains(output)) {
        m_outputs.append(output);
    }
    return true;
}

void XRandRCrtc::disconectOutput(xcb_randr_output_t output)
{
    update();
    qCDebug(KSCREEN_XRANDR) << "Disconnected output" << output << "from CRTC" << m_crtc;

    const int index = m_outputs.indexOf(output);
    if (index > -1) {
        m_outputs.remove(index);
    }
}

bool XRandRCrtc::isFree() const
{
    return m_outputs.isEmpty();
}

void XRandRCrtc::update()
{
    XCB::CRTCInfo crtcInfo(m_crtc, XCB_TIME_CURRENT_TIME);
    m_mode = crtcInfo->mode;

    m_geometry = QRect(crtcInfo->x, crtcInfo->y, crtcInfo->width, crtcInfo->height);
    m_rotation = (xcb_randr_rotation_t)crtcInfo->rotation;

    m_possibleOutputs.clear();
    m_possibleOutputs.reserve(crtcInfo->num_possible_outputs);

    xcb_randr_output_t *possible = xcb_randr_get_crtc_info_possible(crtcInfo);
    for (int i = 0; i < crtcInfo->num_possible_outputs; ++i) {
        m_possibleOutputs.append(possible[i]);
    }

    m_outputs.clear();
    xcb_randr_output_t *outputs = xcb_randr_get_crtc_info_outputs(crtcInfo);
    for (int i = 0; i < crtcInfo->num_outputs; ++i) {
        m_outputs.append(outputs[i]);
    }
}

void XRandRCrtc::update(xcb_randr_mode_t mode, xcb_randr_rotation_t rotation, const QRect &geom)
{
    m_mode = mode;
    m_geometry = geom;
    m_rotation = rotation;
}

void XRandRCrtc::updateTimestamp(const xcb_timestamp_t tmstamp)
{
    if (tmstamp > m_timestamp) {
        qCDebug(KSCREEN_XRANDR) << "XRandRCrtc " << m_crtc << " m_timestamp update " << m_timestamp << " => " << tmstamp;
        m_timestamp = tmstamp;
    }
}

void XRandRCrtc::updateConfigTimestamp(const xcb_timestamp_t tmstamp)
{
    if (tmstamp > m_configTimestamp) {
        qCDebug(KSCREEN_XRANDR) << "XRandRCrtc " << m_crtc << " m_configTimestamp update" << m_configTimestamp << " => " << tmstamp;
        m_configTimestamp = tmstamp;
    }
}

bool XRandRCrtc::isChangedFromOutside() const
{
    return m_configTimestamp > m_timestamp;
}

#include "moc_xrandrcrtc.cpp"
