// SPDX-FileCopyrightText: 2010 by Dario Freddi <drf@kde.org>
// SPDX-FileCopyrightText: 2015 by Kai Uwe Broulik <kde@privat.broulik.de>
// SPDX-FileCopyrightText: 2015 by Martin Gräßlin <mgraesslin@kde.org>
// SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "xcbdpmshelper_p.h"

#include <QGuiApplication>
#include <QtGui/private/qtx11extras_p.h>

#include <kscreendpms_debug.h>

#include <xcb/dpms.h>

template<typename T>
using ScopedCPointer = QScopedPointer<T, QScopedPointerPodDeleter>;

XcbDpmsHelper::XcbDpmsHelper()
    : AbstractDpmsHelper()
{
    auto *c = QX11Info::connection();

    xcb_prefetch_extension_data(c, &xcb_dpms_id);
    auto *extension = xcb_get_extension_data(c, &xcb_dpms_id);
    if (!extension || !extension->present) {
        qCWarning(KSCREEN_DPMS) << "DPMS extension not available";
        setSupported(false);
        return;
    }

    ScopedCPointer<xcb_dpms_capable_reply_t> capableReply(xcb_dpms_capable_reply(c, xcb_dpms_capable(c), nullptr));
    setSupported(capableReply && capableReply->capable);

    // Disable a default timeout, if any
    xcb_dpms_set_timeouts(QX11Info::connection(), 0, 0, 0);
}

XcbDpmsHelper::~XcbDpmsHelper() = default;

void XcbDpmsHelper::trigger(KScreen::Dpms::Mode mode, const QList<QScreen *> &screens)
{
    setHasPendingChanges(true);
    auto *c = QX11Info::connection();

    if (screens != qGuiApp->screens()) {
        qCWarning(KSCREEN_DPMS) << "DPMS actions are applied to all screens on X11";
    }

    ScopedCPointer<xcb_dpms_info_reply_t> infoReply(xcb_dpms_info_reply(c, xcb_dpms_info(c), nullptr));

    if (!infoReply) {
        qCWarning(KSCREEN_DPMS) << "Failed to query DPMS state, cannot trigger";
        return;
    }

    xcb_dpms_dpms_mode_t level = XCB_DPMS_DPMS_MODE_ON;
    switch (mode) {
    case KScreen::Dpms::Toggle:
        if (infoReply->power_level == XCB_DPMS_DPMS_MODE_ON) {
            level = XCB_DPMS_DPMS_MODE_OFF;
        } else {
            level = XCB_DPMS_DPMS_MODE_ON;
        }
        break;
    case KScreen::Dpms::Off:
        level = XCB_DPMS_DPMS_MODE_OFF;
        break;
    case KScreen::Dpms::Standby:
        level = XCB_DPMS_DPMS_MODE_STANDBY;
        break;
    case KScreen::Dpms::Suspend:
        level = XCB_DPMS_DPMS_MODE_SUSPEND;
        break;
    case KScreen::Dpms::On:
        level = XCB_DPMS_DPMS_MODE_ON;
        break;
    }

    if (!infoReply->state) {
        xcb_dpms_enable(c);
    }

    xcb_dpms_force_level(c, level);

    setHasPendingChanges(false);
}
