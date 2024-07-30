/*
    This file is part of the Kate project.

    SPDX-FileCopyrightText: 2010 Christoph Cullmann <cullmann@kde.org>
    SPDX-FileCopyrightText: 2010-2018 Dominik Haumann <dhaumann@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KATETEXTBUFFERTEST_H
#define KATETEXTBUFFERTEST_H

#include <QObject>
#include <QTest>

#include <config.h>

class KateTextBufferTest : public QObject
{
    Q_OBJECT

public:
    KateTextBufferTest();
    ~KateTextBufferTest() override;

private Q_SLOTS:
    void basicBufferTest();
    void wrapLineTest();
    void insertRemoveTextTest();
    void cursorTest();
    void foldingTest();
    void nestedFoldingTest();
    void saveFileInUnwritableFolder();
    void lineLengthLimit();

#if HAVE_KAUTH
    void saveFileWithElevatedPrivileges();
#endif
};

#endif // KATETEXTBUFFERTEST_H
