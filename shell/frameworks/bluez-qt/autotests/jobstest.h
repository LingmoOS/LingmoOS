/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#ifndef JOBSTEST_H
#define JOBSTEST_H

#include <QObject>

class JobsTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void deleteManagerTest();
    void deleteInitManagerJobTest();
    void deleteObexManagerTest();
    void deleteInitObexManagerJobTest();
};

#endif // JOBSTEST_H
