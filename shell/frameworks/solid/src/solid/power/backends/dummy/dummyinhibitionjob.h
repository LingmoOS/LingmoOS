/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_DUMMY_INHIBITION_JOB_H
#define SOLID_DUMMY_INHIBITION_JOB_H

#include "backends/abstractinhibitionjob.h"
#include "solid/power.h"

namespace Solid
{
class Inhibition;
class DummyInhibitionJob : public AbstractInhibitionJob
{
    Q_OBJECT
public:
    explicit DummyInhibitionJob(Power::InhibitionTypes inhibitions, const QString &description, QObject *parent = nullptr);

    Inhibition *inhibition() override;
private Q_SLOTS:
    void doStart() override;

private:
    Power::InhibitionTypes m_inhibitions;
    QString m_description;
};
}
#endif // SOLID_DUMMY_INHIBITION_JOB_H
