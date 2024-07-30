/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2019 David Hallas <david@davidhallas.dk>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "klistopenfilesjobtest_unix.h"
#include "klistopenfilesjob.h"
#include <QCoreApplication>
#include <QStandardPaths>
#include <QStringLiteral>
#include <QTemporaryDir>
#include <QTest>
#include <algorithm>

#ifdef Q_OS_FREEBSD
// See implementation note in testOpenFiles()
#include <QProcess>
#endif

QTEST_MAIN(KListOpenFilesJobTest)
void initLocale()
{
    qputenv("LC_ALL", "en_US.utf-8");
}

Q_CONSTRUCTOR_FUNCTION(initLocale)

namespace
{
bool hasLsofInstalled()
{
    return !QStandardPaths::findExecutable(QStringLiteral("lsof")).isEmpty();
}

}

void KListOpenFilesJobTest::testOpenFiles()
{
    if (!hasLsofInstalled()) {
        QSKIP("lsof is not installed - skipping test");
    }

    // Create a file and hold it open, so that lsof must report us
    QTemporaryDir tempDir;
    QFile tempFile(tempDir.path() + QStringLiteral("/file"));
    QVERIFY(tempFile.open(QIODevice::WriteOnly));
    bool xfail_zfs = false; // Expected failure because of ZFS
#ifdef Q_OS_FREEBSD
    // FIXME: On FreeBSD, lsof does not support zfs (as of 2022), see
    // https://bugs.freebsd.org/bugzilla/show_bug.cgi?id=253553
    //
    // This affects regular files only. So for FreeBSD, check if
    // the tempDir seems to be on a ZFS filesystem, e.g.
    //
    // ```
    // [adridg@beastie .../invent/kcoreaddons]$ lsof +d /tmp > /dev/null
    // lsof: WARNING: no ZFS support has been defined.
    //   See 00FAQ for more information.
    // ```
    {
        QProcess lsof;
        lsof.start(QStringLiteral("lsof"), {QStringLiteral("+d"), tempDir.path()});
        lsof.waitForFinished();

        auto stderr = lsof.readAllStandardError();
        xfail_zfs = (lsof.exitCode() != 0) && stderr.contains("ZFS");
    }
#endif

    auto job = new KListOpenFilesJob(tempDir.path());
    QVERIFY2(job->exec(), qPrintable(job->errorString()));
    QCOMPARE(job->error(), KJob::NoError);
    auto processInfoList = job->processInfoList();
    if (xfail_zfs) {
        // Ths list is empty, so the subsequent find and validity-checks
        // don't make sense.
        QEXPECT_FAIL("", "lsof(8) does not support regular files on ZFS", Abort);
    }
    QVERIFY(!processInfoList.empty());
    auto testProcessIterator = std::find_if(processInfoList.begin(), processInfoList.end(), [](const KProcessList::KProcessInfo &info) {
        return info.pid() == QCoreApplication::applicationPid();
    });
    QVERIFY(testProcessIterator != processInfoList.end());
    const auto &processInfo = *testProcessIterator;
    QVERIFY(processInfo.isValid());
    QCOMPARE(processInfo.pid(), QCoreApplication::applicationPid());
}

void KListOpenFilesJobTest::testNoOpenFiles()
{
    if (!hasLsofInstalled()) {
        QSKIP("lsof is not installed - skipping test");
    }
    QTemporaryDir tempDir;
    auto job = new KListOpenFilesJob(tempDir.path());
    QVERIFY2(job->exec(), qPrintable(job->errorString()));
    QCOMPARE(job->error(), KJob::NoError);
    QVERIFY(job->processInfoList().empty());
}

void KListOpenFilesJobTest::testNonExistingDir()
{
    if (!hasLsofInstalled()) {
        QSKIP("lsof is not installed - skipping test");
    }
    QString nonExistingDir(QStringLiteral("/does/not/exist"));
    auto job = new KListOpenFilesJob(nonExistingDir);
    QVERIFY(!job->exec());
    QCOMPARE(job->error(), static_cast<int>(KListOpenFilesJob::Error::DoesNotExist));
    QCOMPARE(job->errorText(), QStringLiteral("Path %1 doesn't exist").arg(nonExistingDir));
    QVERIFY(job->processInfoList().empty());
}

/**
 * @brief Helper class to temporarily set an environment variable and reset it on destruction
 */
class ScopedEnvVariable
{
public:
    ScopedEnvVariable(const QLatin1String &Name, const QByteArray &NewValue)
        : name(Name)
        , originalValue(qgetenv(name.latin1()))
    {
        qputenv(name.latin1(), NewValue);
    }
    ~ScopedEnvVariable()
    {
        qputenv(name.latin1(), originalValue);
    }

private:
    const QLatin1String name;
    const QByteArray originalValue;
};

void KListOpenFilesJobTest::testLsofNotFound()
{
    // This test relies on clearing the PATH variable so that lsof is not found
    ScopedEnvVariable emptyPathEnvironment(QLatin1String("PATH"), QByteArray());
    QDir path(QCoreApplication::applicationDirPath());
    auto job = new KListOpenFilesJob(path.path());
    QVERIFY(!job->exec());
    QCOMPARE(job->error(), static_cast<int>(KListOpenFilesJob::Error::InternalError));
    QVERIFY(job->processInfoList().empty());
}

#include "moc_klistopenfilesjobtest_unix.cpp"
