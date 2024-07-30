/*
 *  SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "qscreenscreen.h"

#include <configmonitor.h>
#include <mode.h>
#include <screen.h>

#include <QGuiApplication>

using namespace KScreen;

QScreenScreen::QScreenScreen(QScreenConfig *config)
    : QObject(config)
{
}

QScreenScreen::~QScreenScreen()
{
}

ScreenPtr QScreenScreen::toKScreenScreen() const
{
    KScreen::ScreenPtr kscreenScreen(new KScreen::Screen);
    updateKScreenScreen(kscreenScreen);
    return kscreenScreen;
}

void QScreenScreen::updateKScreenScreen(ScreenPtr &screen) const
{
    if (!screen) {
        return;
    }

    auto primary = QGuiApplication::primaryScreen();

    if (primary) {
        QSize _s = primary->availableVirtualGeometry().size();

        screen->setCurrentSize(_s);
        screen->setId(1);
        screen->setMaxSize(_s);
        screen->setMinSize(_s);
        screen->setCurrentSize(_s);
        screen->setMaxActiveOutputsCount(QGuiApplication::screens().count());
    }
}

#include "moc_qscreenscreen.cpp"
