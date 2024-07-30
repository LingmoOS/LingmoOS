/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2008-2010 Dario Freddi <drf@kde.org>
    SPDX-FileCopyrightText: 2010 Alejandro Fiestas <alex@eyeos.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-only

 */

#pragma once

#include <QObject>

#include "powerdevilcore_export.h"
#include "powerdevilenums.h"
#include "powerdevilkeyboardbrightnesslogic.h"
#include "upower_kbdbacklight_interface.h"

class POWERDEVILCORE_EXPORT KeyboardBrightnessController : public QObject
{
    Q_OBJECT
public:
    KeyboardBrightnessController();

    bool isSupported() const;

    int brightness() const;
    int maxBrightness() const;
    void setBrightness(int value);
    int brightnessSteps();

    int keyboardBrightnessKeyPressed(PowerDevil::BrightnessLogic::StepAdjustmentAction adjustment);
    int toggleBacklight();

Q_SIGNALS:
    void brightnessInfoChanged(const PowerDevil::BrightnessLogic::BrightnessInfo &brightnessInfo);

private Q_SLOTS:
    void onBrightnessChanged(int value, const QString &source);

private:
    int calculateNextBrightnessStep(int value, int valueMax, PowerDevil::BrightnessLogic::StepAdjustmentAction adjustment);

    int m_maxBrightness;
    int m_cachedBrightness;
    int m_brightnessBeforeTogglingOff;
    bool m_isSupported = false;

    PowerDevil::KeyboardBrightnessLogic m_keyboardBrightnessLogic;

    OrgFreedesktopUPowerKbdBacklightInterface *m_kbdBacklight;
};
