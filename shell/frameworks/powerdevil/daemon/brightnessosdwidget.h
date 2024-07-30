/*
 * brightnessosdwidget.h
 * adapted from kdemultimedia/kmix/osdwidget.h
 * SPDX-FileCopyrightText: 2009 Aurélien Gâteau <agateau@kde.org>
 * SPDX-FileCopyrightText: 2009 Dario Andres Rodriguez <andresbajotierra@gmail.com>
 * SPDX-FileCopyrightText: 2009 Christian Esken <christian.esken@arcor.de>
 * SPDX-FileCopyrightText: 2010 Felix Geyer <debfx-kde@fobos.de>
 * SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#pragma once

#include "powerdevilcore_export.h"
#include "powerdevilenums.h"

namespace BrightnessOSDWidget
{
POWERDEVILCORE_EXPORT void show(int percentage, PowerDevil::BrightnessControlType type = PowerDevil::BrightnessControlType::Screen);
}
