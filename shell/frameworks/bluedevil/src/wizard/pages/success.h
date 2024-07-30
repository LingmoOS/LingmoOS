/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2010 Alejandro Fiestas Olivares <afiestas@kde.org>
 * SPDX-FileCopyrightText: 2010-2011 UFO Coders <info@ufocoders.com>
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include "ui_success.h"

#include <QWizardPage>

class BlueWizard;

class SuccessPage : public QWizardPage, Ui::Success
{
    Q_OBJECT

public:
    explicit SuccessPage(BlueWizard *parent = nullptr);

    int nextId() const override;
    void initializePage() override;

private:
    BlueWizard *const m_wizard;
};
