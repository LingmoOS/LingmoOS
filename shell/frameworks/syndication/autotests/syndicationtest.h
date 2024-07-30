/*
    This file is part of LibSyndication.
    SPDX-FileCopyrightText: Laurent Montel <montel@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#ifndef SYNDICATIONTEST_H
#define SYNDICATIONTEST_H

#include <QObject>

class SyndicationTest : public QObject
{
    Q_OBJECT
public:
    explicit SyndicationTest(QObject *parent = nullptr);
    ~SyndicationTest() override = default;
private Q_SLOTS:
    void testSyncationFile_data();
    void testSyncationFile();
};

#endif // SYNDICATIONTEST_H
