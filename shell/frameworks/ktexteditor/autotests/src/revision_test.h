/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010-2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_REVISION_TEST_H
#define KATE_REVISION_TEST_H

#include <QObject>

class RevisionTest : public QObject
{
    Q_OBJECT

public:
    RevisionTest();
    ~RevisionTest() override;

private Q_SLOTS:
    void testTransformCursor();
    void testTransformRange();
};

#endif // KATE_REVISION_TEST_H
