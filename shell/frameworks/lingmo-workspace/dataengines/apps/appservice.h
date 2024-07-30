/*
    SPDX-FileCopyrightText: 2009 Chani Armitage <chani@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#pragma once

// lingmo
#include <Lingmo5Support/Service>
#include <Lingmo5Support/ServiceJob>

// own
#include "appsource.h"

/**
 * App Service
 */
class AppService : public Lingmo5Support::Service
{
    Q_OBJECT

public:
    explicit AppService(AppSource *source);
    ~AppService() override;

protected:
    Lingmo5Support::ServiceJob *createJob(const QString &operation, QMap<QString, QVariant> &parameters) override;

private:
    AppSource *m_source;
};
