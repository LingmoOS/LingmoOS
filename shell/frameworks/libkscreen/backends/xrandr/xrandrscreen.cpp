/*
 *  SPDX-FileCopyrightText: 2012 Alejandro Fiestas Olivares <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2012, 2013 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "xrandrscreen.h"

#include "xrandr.h"
#include "xrandrconfig.h"

#include "../xcbwrapper.h"

#include "screen.h"

#include <QtGui/private/qtx11extras_p.h>

XRandRScreen::XRandRScreen(XRandRConfig *config)
    : QObject(config)
{
    XCB::ScreenSize size(XRandR::rootWindow());
    m_maxSize = QSize(size->max_width, size->max_height);
    m_minSize = QSize(size->min_width, size->min_height);
    update();
}

XRandRScreen::~XRandRScreen()
{
}

void XRandRScreen::update()
{
    const int appScreen = QX11Info::appScreen();
    m_id = appScreen;
    xcb_screen_t *screen = XCB::screenOfDisplay(XCB::connection(), appScreen);
    m_currentSize = QSize(screen->width_in_pixels, screen->height_in_pixels);
}

void XRandRScreen::update(const QSize &size)
{
    m_currentSize = size;
}

QSize XRandRScreen::currentSize()
{
    return m_currentSize;
}

KScreen::ScreenPtr XRandRScreen::toKScreenScreen() const
{
    KScreen::ScreenPtr kscreenScreen(new KScreen::Screen);
    kscreenScreen->setId(m_id);
    kscreenScreen->setMaxSize(m_maxSize);
    kscreenScreen->setMinSize(m_minSize);
    kscreenScreen->setCurrentSize(m_currentSize);

    XCB::ScopedPointer<xcb_randr_get_screen_resources_reply_t> screenResources(XRandR::screenResources());
    kscreenScreen->setMaxActiveOutputsCount(screenResources->num_crtcs);

    return kscreenScreen;
}

void XRandRScreen::updateKScreenScreen(KScreen::ScreenPtr &screen) const
{
    screen->setCurrentSize(m_currentSize);
}

#include "moc_xrandrscreen.cpp"
