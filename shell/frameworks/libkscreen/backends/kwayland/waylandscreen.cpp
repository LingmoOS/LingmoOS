/*
 *  SPDX-FileCopyrightText: 2014-2015 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */
#include "waylandscreen.h"

#include "waylandconfig.h"
#include "waylandoutputdevice.h"

#include <mode.h>
#include <screen.h>

using namespace KScreen;

WaylandScreen::WaylandScreen(WaylandConfig *config)
    : QObject(config)
    , m_outputCount(0)
{
}

ScreenPtr WaylandScreen::toKScreenScreen(KScreen::ConfigPtr &parent) const
{
    Q_UNUSED(parent);

    KScreen::ScreenPtr kscreenScreen(new KScreen::Screen);
    updateKScreenScreen(kscreenScreen);
    return kscreenScreen;
}

void WaylandScreen::setOutputs(const QList<WaylandOutputDevice *> &outputs)
{
    m_outputCount = outputs.count();

    QRect r;
    for (const auto *out : outputs) {
        if (out->enabled()) {
            r |= QRect(out->globalPosition(), out->pixelSize() / out->scale());
        }
    }
    m_size = r.size();
}

void WaylandScreen::updateKScreenScreen(KScreen::ScreenPtr &screen) const
{
    screen->setMinSize(QSize(0, 0));

    // 64000^2 should be enough for everyone.
    screen->setMaxSize(QSize(64000, 64000));

    screen->setCurrentSize(m_size);
    screen->setMaxActiveOutputsCount(m_outputCount);
}

#include "moc_waylandscreen.cpp"
