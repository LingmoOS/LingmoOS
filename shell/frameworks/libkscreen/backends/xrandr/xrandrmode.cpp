/*
 *  SPDX-FileCopyrightText: 2012, 2013 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "xrandrmode.h"

#include "xrandroutput.h"

#include "mode.h"

XRandRMode::XRandRMode(const xcb_randr_mode_info_t &modeInfo, XRandROutput *output)
    : QObject(output)
{
    m_id = modeInfo.id;
    // FIXME XCB
    // m_name = QString::fromUtf8(modeInfo->name);
    m_size = QSize(modeInfo.width, modeInfo.height);

    double vTotal = modeInfo.vtotal;
    if (modeInfo.mode_flags & XCB_RANDR_MODE_FLAG_DOUBLE_SCAN) {
        /* doublescan doubles the number of lines */
        vTotal *= 2;
    }

    if (modeInfo.mode_flags & XCB_RANDR_MODE_FLAG_INTERLACE) {
        /* interlace splits the frame into two fields */
        /* the field rate is what is typically reported by monitors */
        vTotal /= 2;
    }

    m_refreshRate = (float)modeInfo.dot_clock / ((float)modeInfo.htotal * vTotal);
}

XRandRMode::~XRandRMode()
{
}

KScreen::ModePtr XRandRMode::toKScreenMode()
{
    KScreen::ModePtr kscreenMode(new KScreen::Mode);

    kscreenMode->setId(QString::number(m_id));
    kscreenMode->setName(m_name);
    kscreenMode->setSize(m_size);
    kscreenMode->setRefreshRate(m_refreshRate);

    return kscreenMode;
}

xcb_randr_mode_t XRandRMode::id() const
{
    return m_id;
}

QSize XRandRMode::size() const
{
    return m_size;
}

float XRandRMode::refreshRate() const
{
    return m_refreshRate;
}

QString XRandRMode::name() const
{
    return m_name;
}

#include "moc_xrandrmode.cpp"
