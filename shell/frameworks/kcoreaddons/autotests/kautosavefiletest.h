/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006 Jacob R Rideout <kde@jacobrideout.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef kautosavefiletest_h
#define kautosavefiletest_h

#include <QObject>
#include <QStringList>

class KAutoSaveFileTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void test_readWrite();
    void test_fileNameMaxLength();
    void test_fileStaleFiles();
    void test_applicationStaleFiles();
    void test_locking();
    void cleanupTestCase();

private:
    QStringList filesToRemove;
};

#endif
