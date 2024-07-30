/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include "ui_pairing.h"

#include <QWizardPage>

#include <BluezQt/Device>
#include <BluezQt/Request>

class BlueWizard;

class PairingPage : public QWizardPage, Ui::Pairing
{
    Q_OBJECT

public:
    explicit PairingPage(BlueWizard *parent = nullptr);

    int nextId() const override;
    void initializePage() override;

public Q_SLOTS:
    void pairingFinished(BluezQt::PendingCall *call);
    void pinRequested(const QString &pin);
    void confirmationRequested(const QString &passkey, const BluezQt::Request<> &req);
    void matchesClicked();
    void notMatchClicked();
    void cancelClicked();

private:
    QList<QWizard::WizardButton> wizardButtonsLayout() const;

    BlueWizard *const m_wizard;
    BluezQt::DevicePtr m_device;
    BluezQt::Request<> m_req;
    bool m_success = false;
};
