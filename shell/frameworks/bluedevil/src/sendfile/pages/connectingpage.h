/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2010-2011 Alejandro Fiestas Olivares <afiestas@kde.org>
 * SPDX-FileCopyrightText: 2010-2011 UFO Coders <info@ufocoders.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include "ui_connecting.h"

#include <QWizardPage>

#include <BluezQt/ObexManager>

class SendFileWizard;

class ConnectingPage : public QWizardPage, public Ui::Connecting
{
    Q_OBJECT

public:
    explicit ConnectingPage(SendFileWizard *wizard = nullptr);

    void initializePage() override;
    bool isComplete() const override;

private Q_SLOTS:
    void initJobResult(BluezQt::InitObexManagerJob *job);
    void createSessionFinished(BluezQt::PendingCall *call);

private:
    QList<QWizard::WizardButton> wizardButtonsLayout() const;

    SendFileWizard *const m_wizard;
    BluezQt::DevicePtr m_device;
};
