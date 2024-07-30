/*
 *   SPDX-FileCopyrightText: 2010 Dario Freddi <drf@kde.org>
 *   SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <powerdevilaction.h>

#include <QScopedPointer>

class AbstractDpmsHelper;

namespace KScreen
{
class Dpms;
}

namespace PowerDevil
{
namespace BundledActions
{
class DPMS : public PowerDevil::Action
{
    Q_OBJECT

public:
    explicit DPMS(QObject *parent);
    ~DPMS() override;

Q_SIGNALS:
    void startFade();
    void stopFade();

protected:
    void onWakeupFromIdle() override;
    void onIdleTimeout(std::chrono::milliseconds timeout) override;
    void triggerImpl(const QVariantMap &args) override;
    bool isSupported() override;
    void onProfileUnload() override;

public:
    bool loadAction(const PowerDevil::ProfileSettings &profileSettings) override;

private Q_SLOTS:
    void turnOffOnIdleTimeout();
    void onUnavailablePoliciesChanged(PowerDevil::PolicyAgent::RequiredPolicies policies);
    void onScreenLockerActiveChanged(bool active);
    void onAboutToSuspend();
    void onResumeFromSuspend();

private:
    void setKeyboardBrightnessHelper(int brightness);
    void registerStandardIdleTimeout();

    std::chrono::milliseconds m_idleTimeoutWhenUnlocked{-1};
    std::chrono::milliseconds m_idleTimeoutWhenLocked{-1};
    std::chrono::milliseconds m_idleTimeoutWhenActivatingLock{-1};
    bool m_isActivatingLock = false;
    bool m_isAboutToSuspend = false;
    PowerDevil::PolicyAgent::RequiredPolicies m_inhibitScreen = PowerDevil::PolicyAgent::None;

    int m_oldKeyboardBrightness = 0;
    QScopedPointer<KScreen::Dpms> m_dpms;

    bool m_lockBeforeTurnOff = false;
    void lockScreen();
};

}
}
