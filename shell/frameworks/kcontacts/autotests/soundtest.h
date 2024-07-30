/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SOUND_TEST_H
#define SOUND_TEST_H

#include <QObject>

class SoundTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void emptyTest();
    void storeTestIntern();
    void storeTestExtern();
    void equalsTestIntern();
    void equalsTestExtern();
    void differsTest();
    void assignmentTestIntern();
    void assignmentTestExtern();
    void serializeTest();
    void shouldParseSource();
    void shouldGenerateVCard4WithData();
    void shouldGenerateVCard4WithUrl();
    void shouldGenerateVCard3WithData();
    void shouldGenerateVCard3WithUrl();
};

#endif
