/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include "ui_failpage.h"

#include <optional>

#include <QWizardPage>

class SendFileWizard;

class FailPage : public QWizardPage, Ui::FailPage
{
    Q_OBJECT

public:
    explicit FailPage(SendFileWizard *parent = nullptr);

    void initializePage() override;

    /**
     * If not set, \FailPage will use the error message from \SendFileWizard
     */
    void setErrorMessage(const QString &errorMessage);

private:
    SendFileWizard *const m_wizard;
    std::optional<QString> m_errorMessage;
};
