/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2018 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATE_INLINENOTE_TEST_H
#define KATE_INLINENOTE_TEST_H

#include <QObject>

class InlineNoteTest : public QObject
{
    Q_OBJECT

public:
    InlineNoteTest();
    ~InlineNoteTest() override;

private Q_SLOTS:
    void testInlineNote();
};

#endif // KATE_INLINENOTE_TEST_H
