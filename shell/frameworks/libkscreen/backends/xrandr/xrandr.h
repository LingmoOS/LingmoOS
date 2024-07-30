/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2012, 2013 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#pragma once

#include "abstractbackend.h"

#include <QLoggingCategory>
#include <QSize>

#include <xcb/randr.h>
#include <xcb/xcb.h>

class QRect;
class QTimer;

class XCBEventListener;
class XRandRConfig;

class XRandR : public KScreen::AbstractBackend
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kf6.kscreen.backends.xrandr")

public:
    explicit XRandR();
    ~XRandR() override;

    QString name() const override;
    QString serviceName() const override;
    KScreen::ConfigPtr config() const override;
    void setConfig(const KScreen::ConfigPtr &config) override;
    bool isValid() const override;
    QByteArray edid(int outputId) const override;

    static QByteArray outputEdid(xcb_randr_output_t outputId);
    static xcb_randr_get_screen_resources_reply_t *screenResources();
    static xcb_screen_t *screen();
    static xcb_window_t rootWindow();

    static bool hasProperty(xcb_randr_output_t outputId, const QByteArray &name);

private:
    void outputChanged(xcb_randr_output_t output, xcb_randr_crtc_t crtc, xcb_randr_mode_t mode, xcb_randr_connection_t connection);
    void crtcChanged(xcb_randr_crtc_t crtc, xcb_randr_mode_t mode, xcb_randr_rotation_t rotation, const QRect &geom, xcb_timestamp_t timestamp);
    void screenChanged(xcb_randr_rotation_t rotation, const QSize &sizePx, const QSize &sizeMm);

    static quint8 *getXProperty(xcb_randr_output_t output, xcb_atom_t atom, size_t &len);

    static xcb_screen_t *s_screen;
    static xcb_window_t s_rootWindow;
    static XRandRConfig *s_internalConfig;

    static int s_randrBase;
    static int s_randrError;
    static bool s_monitorInitialized;
    static bool s_has_1_3;
    static bool s_xorgCacheInitialized;

    XCBEventListener *m_x11Helper;
    bool m_isValid;

    QTimer *m_configChangeCompressor;
};

Q_DECLARE_LOGGING_CATEGORY(KSCREEN_XRANDR)
