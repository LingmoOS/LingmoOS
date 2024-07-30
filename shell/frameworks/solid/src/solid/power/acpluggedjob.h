/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_AC_PLUGGED_JOB_H
#define SOLID_AC_PLUGGED_JOB_H

#include <Solid/Job>

#include "solid_export.h"

namespace Solid
{
class AcPluggedJobPrivate;
class SOLID_EXPORT AcPluggedJob : public Job
{
    Q_OBJECT
    Q_PROPERTY(bool plugged READ isPlugged NOTIFY result)

public:
    explicit AcPluggedJob(QObject *parent = nullptr);

    bool isPlugged() const;

private Q_SLOTS:
    void doStart() override;

    /**
     * We have to re-declare the signal because
     * if not, Q_PROPERTY wouldn't work.
     */
Q_SIGNALS:
    void result(Solid::Job *);

private:
    Q_DECLARE_PRIVATE(AcPluggedJob)
};
} // Solid namespace
#endif // SOLID_AC_PLUGGED_JOB_H
