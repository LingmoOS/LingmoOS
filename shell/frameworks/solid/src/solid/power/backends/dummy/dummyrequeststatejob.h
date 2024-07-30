/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef DUMMY_REQUEST_STATE_JOB_H
#define DUMMY_REQUEST_STATE_JOB_H

#include "backends/abstractrequeststatejob.h"

namespace Solid
{
class DummyRequestStateJob : public AbstractRequestStateJob
{
    Q_OBJECT

private Q_SLOTS:
    void doStart() override;
};
}

#endif // DUMMY_REQUEST_STATE_JOB_H
