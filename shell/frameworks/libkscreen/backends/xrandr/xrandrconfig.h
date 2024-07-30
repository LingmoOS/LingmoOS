/*
 *  SPDX-FileCopyrightText: 2012, 2013 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#pragma once

#include <QObject>
#include <cstdint>

#include "xrandrcrtc.h"
#include "xrandroutput.h"

class XRandRScreen;
namespace KScreen
{
class Config;
}

class XRandRConfig : public QObject
{
    Q_OBJECT

public:
    XRandRConfig();
    ~XRandRConfig() override;

    XRandROutput::Map outputs() const;
    XRandROutput *output(xcb_randr_output_t output) const;

    XRandRCrtc::Map crtcs() const;
    XRandRCrtc *crtc(xcb_randr_crtc_t crtc) const;

    XRandRScreen *screen() const;

    void addNewOutput(xcb_randr_output_t id);
    void addNewCrtc(xcb_randr_crtc_t crtc);
    void removeOutput(xcb_randr_output_t id);

    KScreen::ConfigPtr toKScreenConfig() const;
    void applyKScreenConfig(const KScreen::ConfigPtr &config);

private:
    QSize screenSize(const KScreen::ConfigPtr &config) const;
    bool setScreenSize(const QSize &size) const;

    void setOutputPriority(xcb_randr_output_t outputId, uint32_t priority) const;

    bool disableOutput(const KScreen::OutputPtr &output) const;
    bool enableOutput(const KScreen::OutputPtr &output) const;
    bool changeOutput(const KScreen::OutputPtr &output) const;

    bool sendConfig(const KScreen::OutputPtr &kscreenOutput, XRandRCrtc *crtc) const;

    /**
     * We need to print stuff to discover the damn bug
     * where currentMode is null
     */
    void printConfig(const KScreen::ConfigPtr &config) const;
    void printInternalCond() const;

    XRandROutput::Map m_outputs;
    XRandRCrtc::Map m_crtcs;
    XRandRScreen *m_screen;
};
