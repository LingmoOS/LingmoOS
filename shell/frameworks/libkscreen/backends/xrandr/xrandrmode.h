/*
 *  SPDX-FileCopyrightText: 2012, 2013 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#pragma once

#include <QMap>
#include <QObject>
#include <QSize>
#include <QVariant>

#include <xcb/randr.h>
#include <xcb/xcb.h>

#include "types.h"

class XRandROutput;
namespace KScreen
{
class Output;
class Mode;
}

class XRandRMode : public QObject
{
    Q_OBJECT

public:
    typedef QMap<xcb_randr_mode_t, XRandRMode *> Map;

    explicit XRandRMode(const xcb_randr_mode_info_t &modeInfo, XRandROutput *output);
    ~XRandRMode() override;

    KScreen::ModePtr toKScreenMode();

    xcb_randr_mode_t id() const;
    QSize size() const;
    float refreshRate() const;
    QString name() const;

private:
    xcb_randr_mode_t m_id;
    QString m_name;
    QSize m_size;
    float m_refreshRate;
};
