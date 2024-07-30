/*
 *  SPDX-FileCopyrightText: 2014 Daniel Vrátil <dvratil@redhat.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#pragma once

#include <QMap>
#include <QSharedPointer>

namespace KScreen
{
class Config;
typedef QSharedPointer<KScreen::Config> ConfigPtr;
class Screen;
typedef QSharedPointer<KScreen::Screen> ScreenPtr;
class Output;
typedef QSharedPointer<KScreen::Output> OutputPtr;
typedef QMap<int, KScreen::OutputPtr> OutputList;

class Mode;
typedef QSharedPointer<KScreen::Mode> ModePtr;
typedef QMap<QString, KScreen::ModePtr> ModeList;

}
