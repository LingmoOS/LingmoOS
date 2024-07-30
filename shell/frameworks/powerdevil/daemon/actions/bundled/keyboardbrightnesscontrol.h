/*
 *   SPDX-FileCopyrightText: 2012 Michael Zanetti <mzanetti@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <powerdevilaction.h>
#include <powerdevilkeyboardbrightnesslogic.h>

namespace PowerDevil::BundledActions
{
class KeyboardBrightnessControl : public PowerDevil::Action
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.Solid.PowerManagement.Actions.KeyboardBrightnessControl")

public:
    explicit KeyboardBrightnessControl(QObject *parent);

protected:
    void onProfileLoad(const QString &previousProfile, const QString &newProfile) override;
    void triggerImpl(const QVariantMap &args) override;
    bool isSupported() override;

public:
    bool loadAction(const PowerDevil::ProfileSettings &profileSettings) override;

    int keyboardBrightness() const;
    int keyboardBrightnessMax() const;
    int keyboardBrightnessSteps() const;

public Q_SLOTS:
    void onBrightnessChangedFromController(const KeyboardBrightnessLogic::BrightnessInfo &brightnessInfo);

    // DBus export
    void increaseKeyboardBrightness();
    void decreaseKeyboardBrightness();
    void toggleKeyboardBacklight();

    void setKeyboardBrightness(int value);
    void setKeyboardBrightnessSilent(int value);

Q_SIGNALS:
    void keyboardBrightnessChanged(int value);
    void keyboardBrightnessMaxChanged(int valueMax);

private:
    int keyboardBrightnessPercent() const;

    int m_defaultValue = -1;
    int m_lastKeyboardBrightness = -1;
};

}
