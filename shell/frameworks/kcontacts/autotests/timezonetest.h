/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TIMEZONE_TEST_H
#define TIMEZONE_TEST_H

#include <QObject>

class TimeZoneTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void emptyTest();
    void storeTest();
    void equalsTest();
    void differsTest();
    void assignmentTest();
    void serializeTest();
    void shouldGenerateVCard3();
    void shouldGenerateVCard4();
};

#endif
