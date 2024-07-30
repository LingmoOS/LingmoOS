/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PHONENUMBER_TEST_H
#define PHONENUMBER_TEST_H

#include <QObject>

class PhoneNumberTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void emptyTest();
    void storeTest();
    void equalsTest();
    void differsTest();
    void assignmentTest();
    void serializeTest();
    void labelTest();

    void shouldExportVCard3();
    void shouldExportVCard4();
    void shouldParseVcard3();
    void shouldParseVcard4();

    void shouldExportVCard21();
    void shouldParseVCard21();

    void testNormalizeNumber_data();
    void testNormalizeNumber();
};

#endif
