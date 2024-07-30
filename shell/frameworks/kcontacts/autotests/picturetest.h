/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2007 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PICTURE_TEST_H
#define PICTURE_TEST_H

#include <QObject>

class PictureTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void emptyTest();
    void storeTestInternImage();
    void storeTestInternRawData();
    void storeTestExtern();
    void equalsTestInternImage();
    void equalsTestInternRawData();
    void equalsTestInternImageAndRawData();
    void equalsTestExtern();
    void differsTest();
    void differsTestInternRawData();
    void differsTestExtern();
    void assignmentTestIntern();
    void assignmentTestExtern();
    void serializeTestInternImage();
    void serializeTestInternRawData();
    void serializeTestExtern();
};

#endif
