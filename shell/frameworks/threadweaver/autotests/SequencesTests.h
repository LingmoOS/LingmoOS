/* -*- C++ -*-
    This file contains a testsuite for sequences in ThreadWeaver.

    SPDX-FileCopyrightText: 2005-2013 Mirko Boehm <mirko@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SEQUENCESTESTS_H
#define SEQUENCESTESTS_H

#include <QObject>

class SequencesTests : public QObject
{
    Q_OBJECT

public:
    SequencesTests();

private Q_SLOTS:
    void RecursiveStopTest();
};

#endif // SEQUENCESTESTS_H
