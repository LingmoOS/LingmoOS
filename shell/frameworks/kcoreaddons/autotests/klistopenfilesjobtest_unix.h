/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2019 David Hallas <david@davidhallas.dk>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KLISTOPENFILESJOBTEST_UNIX_H
#define KLISTOPENFILESJOBTEST_UNIX_H

#include <QObject>

class KListOpenFilesJobTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testOpenFiles();
    void testNoOpenFiles();
    void testNonExistingDir();
    void testLsofNotFound();
};

#endif
