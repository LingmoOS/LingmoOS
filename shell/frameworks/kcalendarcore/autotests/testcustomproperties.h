/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2009 Allen Winter <winter@kde.org>
  SPDX-FileCopyrightText: 2010 Casey Link <unnamedrambler@gmail.com>
  SPDX-FileCopyrightText: 2009-2010 Klaralvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTCUSTOMPROPERTIES_H
#define TESTCUSTOMPROPERTIES_H

#include <QObject>

class CustomPropertiesTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValidity();
    void testCompare();
    void testMapValidity();
    void testMapCompare();
    void testEmpty();
    void testDataStreamOut();
    void testDataStreamIn();
    void testVolatile();
};

#endif
