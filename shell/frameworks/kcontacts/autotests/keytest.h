/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2016-2019 Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KEY_TEST_H
#define KEY_TEST_H

#include <QObject>

class KeyTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void storeTest();
    void equalsTest();
    void differsTest();
    void assignmentTest();
    void serializeTest();
    void shouldExportVCard3();
    void shouldExportVCard4();
    void shouldParseVcard3();
    void shouldParseVcard4();
};

#endif
