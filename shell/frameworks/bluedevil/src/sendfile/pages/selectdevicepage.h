/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2010-2011 Alejandro Fiestas Olivares <afiestas@kde.org>
 * SPDX-FileCopyrightText: 2010-2011 UFO Coders <info@ufocoders.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include "ui_selectfilediscover.h"

#include <QWizardPage>

#include <BluezQt/Types>

class SendFileWizard;

class SelectDevicePage : public QWizardPage, public Ui::SelectFileDiscover
{
    Q_OBJECT

public:
    explicit SelectDevicePage(SendFileWizard *wizard);

    bool isComplete() const override;

private Q_SLOTS:
    void deviceSelected(BluezQt::DevicePtr device);

private:
    SendFileWizard *const m_wizard;
};
