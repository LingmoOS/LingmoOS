/*
 * This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2010-2011 Alejandro Fiestas Olivares <afiestas@kde.org>
 * SPDX-FileCopyrightText: 2010-2011 UFO Coders <info@ufocoders.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#pragma once

#include <QWizardPage>

class KFileWidget;

class SelectFilesPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit SelectFilesPage(QWidget *parent = nullptr);

    void initializePage() override;
    bool isComplete() const override;

private Q_SLOTS:
    void selectionChanged();

private:
    KFileWidget *m_files;
};
