/*
 *  SPDX-FileCopyrightText: SPDX-FileCopyrightText: 2021 Tomaz Canabrava <tcanabrava@kde.org>
 *  SPDX-FileCopyrightText: SPDX-FileCopyrightText: 2023 Jakob Petsovits <jpetso@petsovits.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QString>

#include "powerdevilcore_export.h"

namespace PowerDevil
{

class POWERDEVILCORE_EXPORT GlobalDefaults
{
public:
    static int defaultBatteryCriticalAction(bool canSuspend, bool canHibernate);
};

class POWERDEVILCORE_EXPORT ProfileDefaults
{
public:
    static bool defaultUseProfileSpecificDisplayBrightness(const QString &profileGroup);
    static int defaultDisplayBrightness(const QString &profileGroup);

    static bool defaultDimDisplayWhenIdle();
    static int defaultDimDisplayIdleTimeoutSec(const QString &profileGroup, bool isMobile);

    static bool defaultTurnOffDisplayWhenIdle();
    static int defaultTurnOffDisplayIdleTimeoutSec(const QString &profileGroup, bool isMobile);
    static bool defaultLockBeforeTurnOffDisplay(bool isMobile);

    static unsigned int defaultAutoSuspendAction(bool isVM, bool canSuspend);
    static bool defaultAutoSuspendWhenIdle(bool isVM, bool canSuspend);
    static int defaultAutoSuspendIdleTimeoutSec(const QString &profileGroup, bool isMobile);
    static unsigned int defaultAutoSuspendType();

    static unsigned int defaultPowerButtonAction(bool isMobile);
    static unsigned int defaultPowerDownAction();
    static unsigned int defaultLidAction(bool isVM, bool canSuspend);
};

}
