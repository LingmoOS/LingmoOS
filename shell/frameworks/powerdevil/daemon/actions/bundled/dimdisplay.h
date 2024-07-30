/*
 *   SPDX-FileCopyrightText: 2010 Dario Freddi <drf@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <powerdevilaction.h>

namespace PowerDevil::BundledActions
{
class DimDisplay : public PowerDevil::Action
{
    Q_OBJECT
public:
    explicit DimDisplay(QObject *parent);

protected:
    void onWakeupFromIdle() override;
    void onIdleTimeout(std::chrono::milliseconds timeout) override;
    void triggerImpl(const QVariantMap &args) override;
    bool isSupported() override;

public:
    bool loadAction(const PowerDevil::ProfileSettings &) override;

private Q_SLOTS:
    void onUnavailablePoliciesChanged(PowerDevil::PolicyAgent::RequiredPolicies policies);

private:
    void setBrightnessHelper(int screenBrightness, int keyboardBrightness);

    std::chrono::milliseconds m_dimOnIdleTime{0};

    int m_oldScreenBrightness = 0;
    int m_oldKeyboardBrightness = 0;

    PowerDevil::PolicyAgent::RequiredPolicies m_inhibitScreen = PowerDevil::PolicyAgent::None;
    bool m_dimmed = false;
};

}
