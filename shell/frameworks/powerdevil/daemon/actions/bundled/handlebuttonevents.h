/*
 *   SPDX-FileCopyrightText: 2010 Dario Freddi <drf@kde.org>
 *   SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <KScreen/Types>

#include <powerdevilaction.h>
#include <powerdevilenums.h>

#include <optional>

namespace PowerDevil::BundledActions
{
class HandleButtonEvents : public PowerDevil::Action
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.Solid.PowerManagement.Actions.HandleButtonEvents")

public:
    explicit HandleButtonEvents(QObject *parent);
    ~HandleButtonEvents() override;

    bool loadAction(const PowerDevil::ProfileSettings &profileSettings) override;
    bool isSupported() override;

Q_SIGNALS:
    void triggersLidActionChanged(bool triggers);

protected:
    void triggerImpl(const QVariantMap &args) override;
    void onProfileUnload() override;
    void onIdleTimeout(std::chrono::milliseconds timeout) override;

public Q_SLOTS:
    int lidAction() const;
    bool triggersLidAction() const;

private Q_SLOTS:
    void onLidClosedChanged(bool closed);
    void powerOffButtonTriggered();
    void powerDownButtonTriggered();
    void sleep();
    void hibernate();

    void checkOutputs();

private:
    void processAction(PowerDevil::PowerButtonAction action);
    void triggerAction(const QString &action, const QVariant &type);

    KScreen::ConfigPtr m_screenConfiguration;
    PowerDevil::PowerButtonAction m_lidAction = PowerDevil::PowerButtonAction::NoAction;
    bool m_triggerLidActionWhenExternalMonitorPresent = false;
    std::optional<bool> m_externalMonitorPresent;

    PowerDevil::PowerButtonAction m_powerButtonAction = PowerDevil::PowerButtonAction::NoAction;
    PowerDevil::PowerButtonAction m_powerDownButtonAction = PowerDevil::PowerButtonAction::NoAction;
    PowerDevil::PowerButtonAction m_sleepButtonAction = PowerDevil::PowerButtonAction::Sleep;
    PowerDevil::PowerButtonAction m_hibernateButtonAction = PowerDevil::PowerButtonAction::Hibernate;

    std::optional<int> m_oldKeyboardBrightness;
};

}
