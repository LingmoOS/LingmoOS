/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef UPOWER_DUMMY_AC_PLUGGED_JOB_H
#define UPOWER_DUMMY_AC_PLUGGED_JOB_H

#include "backends/abstractacpluggedjob.h"

namespace Solid
{
class DummyAcPluggedJob : public AbstractAcPluggedJob
{
    Q_OBJECT
public:
    bool isPlugged() const override;

private Q_SLOTS:
    void doStart() override;
};
}

#endif // UPOWER_DUMMY_AC_PLUGGED_JOB_H
