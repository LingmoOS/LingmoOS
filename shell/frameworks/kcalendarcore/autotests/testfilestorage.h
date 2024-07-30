/*
  This file is part of the kcalcore library.
  SPDX-FileCopyrightText: 2007 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef TESTFILESTORAGE_H
#define TESTFILESTORAGE_H

#include <QObject>

class FileStorageTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testValidity();
    void testSave();
    void testSaveLoadSave();

    /** Saves an incidence with éèü chars, then reads the file into a second incidence
        and compares both incidences. The comparison should yield true.
    */
    void testSpecialChars();
};

#endif
