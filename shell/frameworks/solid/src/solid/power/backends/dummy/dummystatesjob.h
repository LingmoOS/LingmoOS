/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef DUMMY_STATES_JOB_H
#define DUMMY_STATES_JOB_H

#include "backends/abstractstatesjob.h"

namespace Solid
{
class DummyStatesJob : public AbstractStatesJob
{
    Q_OBJECT
public:
    Power::InhibitionTypes states() const override;

private Q_SLOTS:
    void doStart() override;
};
}

#endif // DUMMY_STATES_JOB_H
