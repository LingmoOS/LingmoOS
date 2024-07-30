/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2019 David Hallas <david@davidhallas.dk>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "klistopenfilesjobtest_win.h"
#include "klistopenfilesjob.h"
#include <QCoreApplication>
#include <QStringLiteral>
#include <QTest>

QTEST_MAIN(KListOpenFilesJobTest)

void KListOpenFilesJobTest::testNotSupported()
{
    QDir path(QCoreApplication::applicationDirPath());
    auto job = new KListOpenFilesJob(path.path());
    job->exec();
    QCOMPARE(job->error(), static_cast<int>(KListOpenFilesJob::Error::NotSupported));
    QCOMPARE(job->errorText(), QStringLiteral("KListOpenFilesJob is not supported on Windows"));
    QVERIFY(job->processInfoList().empty());
}

#include "moc_klistopenfilesjobtest_win.cpp"
