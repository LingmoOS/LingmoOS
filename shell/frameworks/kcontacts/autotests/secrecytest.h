/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SECRECY_TEST_H
#define SECRECY_TEST_H

#include <QObject>

class SecrecyTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void emptyTest();
    void storeTest();
    void equalsTest();
    void differsTest();
    void assignmentTest();
    void serializeTest();
};

#endif
