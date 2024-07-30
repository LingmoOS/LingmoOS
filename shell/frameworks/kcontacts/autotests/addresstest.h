/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef ADDRESS_TEST_H
#define ADDRESS_TEST_H

#include <QObject>

class AddressTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void emptyTest();
    void storeTest();
    void equalsTest();
    void differsTest();
    void assignmentTest();
    void serializeTest();
    void formatTest();
    void shouldExportVcard4();
    void shouldExportVcard3();
    void shouldParseAddressVCard3();
    void shouldParseAddressVCard4();
    void shouldExportVCard4WithGeoPosition();
    void shouldParseAddressVCard4WithGeoPosition();
    void testGeoUri();
};

#endif
