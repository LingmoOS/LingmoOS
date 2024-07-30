/*
    SPDX-FileCopyrightText: 2011 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <Lingmo5Support/Service>
#include <Lingmo5Support/ServiceJob>

using namespace Lingmo5Support;

class PowerManagementService : public Lingmo5Support::Service
{
    Q_OBJECT

public:
    explicit PowerManagementService(QObject *parent = nullptr);
    ServiceJob *createJob(const QString &operation, QMap<QString, QVariant> &parameters) override;
};
