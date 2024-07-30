/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_FD_INHIBITION_JOB_H
#define SOLID_FD_INHIBITION_JOB_H

#include "backends/abstractinhibitionjob.h"
#include "solid/inhibition.h"
#include "solid/power.h"

namespace Solid
{
class Inhibition;
class FdInhibition;
class FdInhibitionJob : public AbstractInhibitionJob
{
    Q_OBJECT
public:
    explicit FdInhibitionJob(Power::InhibitionTypes inhibitions, const QString &description, QObject *parent = nullptr);
    virtual ~FdInhibitionJob();

    Inhibition *inhibition() override;
private Q_SLOTS:
    void doStart() override;

    void stateChanged(Inhibition::State state);

private:
    FdInhibition *m_inhibition;
};
}
#endif // SOLID_FD_INHIBITION_JOB_H
