/*
 *   SPDX-FileCopyrightText: 2010 Dario Freddi <drf@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <powerdevilaction.h>
#include <powerdevilenums.h>

#include <QScopedPointer>

namespace PowerDevil
{
class KWinKScreenHelperEffect;

namespace BundledActions
{
class SuspendSession : public PowerDevil::Action
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.Solid.PowerManagement.Actions.SuspendSession")

public:
    explicit SuspendSession(QObject *parent);
    ~SuspendSession() override;

    bool loadAction(const PowerDevil::ProfileSettings &profileSettings) override;

protected:
    void onWakeupFromIdle() override;
    void onIdleTimeout(std::chrono::milliseconds timeout) override;
    void triggerImpl(const QVariantMap &args) override;

public Q_SLOTS:
    void suspendToRam();
    void suspendToDisk();
    void suspendHybrid();

Q_SIGNALS:
    void aboutToSuspend();
    void resumingFromSuspend();

private Q_SLOTS:
    void triggerSuspendSession(PowerDevil::PowerButtonAction action);

private:
    std::chrono::milliseconds m_idleTime{0};
    PowerDevil::PowerButtonAction m_autoSuspendAction;
    PowerDevil::SleepMode m_sleepMode = PowerDevil::SleepMode::SuspendToRam;
    QScopedPointer<PowerDevil::KWinKScreenHelperEffect> m_fadeEffect;
};

}

}
