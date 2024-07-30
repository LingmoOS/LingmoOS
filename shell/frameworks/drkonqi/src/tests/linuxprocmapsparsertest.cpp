/*
    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
*/

#include <QTest>

#include <limits>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <linuxprocmapsparser.h>

class LinuxProcMapsParserTest : public QObject
{
    Q_OBJECT
    qulonglong m_linuxProcfsMapsInInode = 0;

    QByteArray generateData(const QString &templatePath, qulonglong inode)
    {
        QFile f(templatePath);
        if (!f.open(QFile::ReadOnly)) {
            return {};
        }
        QByteArray data = f.readAll();
        data.replace("/__FILE_PATH__", qUtf8Printable(templatePath));
        data.replace("__INODE__", QByteArray::number(inode));
        return data;
    }

private Q_SLOTS:
    void initTestCase()
    {
        struct stat st {
        };
        QVERIFY(stat(QFile::encodeName(QFINDTESTDATA("data/linux-procfs-maps.so")).constData(), &st) != -1);
        m_linuxProcfsMapsInInode = st.st_ino;
    }

    void testHasDeletedStat()
    {
        // Has definitely missing files because of "(deleted)" suffix.
        // The exe is not actually in the fixture to prevent the function from returning early when not finding the exe.
        // Should this need changing in the future this needs routing through generateData().
        QFile f(QFINDTESTDATA("data/linux-procfs-maps-with-missing-files"));
        QVERIFY(f.open(QFile::ReadOnly));
        QVERIFY(LinuxProc::hasMapsDeletedFiles("/usr/bin/kwrite", f.readAll(), LinuxProc::Check::DeletedMarker));
    }

    void testNoDeletedStat()
    {
        // fixture also includes a /memfd:... path which we expect to not get reported as deleted. We do need
        // to substitute valid data in to get past the inode verification though.
        const QString templatePath = QFINDTESTDATA("data/linux-procfs-maps.so");
        const QByteArray data = generateData(templatePath, m_linuxProcfsMapsInInode);
        QVERIFY(!LinuxProc::hasMapsDeletedFiles(templatePath, data, LinuxProc::Check::Stat));
    }

    void testBadInode()
    {
        const QString templatePath = QFINDTESTDATA("data/linux-procfs-maps.so");
        const QByteArray data = generateData(templatePath, m_linuxProcfsMapsInInode - 1 /* random mutation */);
        QVERIFY(LinuxProc::hasMapsDeletedFiles(templatePath, data, LinuxProc::Check::Stat));
    }

    void testExecMarker()
    {
        QFile f(QFINDTESTDATA("data/linux-procfs-maps-with-deleted-exe"));
        QVERIFY(f.open(QFile::ReadOnly));
        QVERIFY(LinuxProc::hasMapsDeletedFiles("/usr/bin/kwrite", f.readAll(), LinuxProc::Check::DeletedMarker));
    }

    void testSOMarker()
    {
        QFile f(QFINDTESTDATA("data/linux-procfs-maps-with-deleted-exe"));
        QVERIFY(f.open(QFile::ReadOnly));
        QVERIFY(LinuxProc::hasMapsDeletedFiles("/usr/bin/kwrite", f.readAll(), LinuxProc::Check::DeletedMarker));
    }

    void testIsLibraryPath()
    {
        QVERIFY(!LinuxProc::isLibraryPath("/bin/a"));
        QVERIFY(!LinuxProc::isLibraryPath("/bin/a."));

        QVERIFY(LinuxProc::isLibraryPath("/bin/a.so"));
        QVERIFY(LinuxProc::isLibraryPath("/bin/a.so.0.1"));
        // we expect deleted modifiers to match, it keeps the regex simpler
        QVERIFY(LinuxProc::isLibraryPath("/bin/a.so.0.1 (deleted)"));
        // A bit silly but what if .so appears as a dirname
        QVERIFY(!LinuxProc::isLibraryPath("/bin/a.so.0/abc/foo"));

        // Drkonqi also has some python handling for some reason :shrug:
        QVERIFY(LinuxProc::isLibraryPath("/bin/a.py"));
        QVERIFY(LinuxProc::isLibraryPath("/bin/a.pyc"));
        QVERIFY(LinuxProc::isLibraryPath("/bin/a.py (deleted)"));
    }
};

QTEST_GUILESS_MAIN(LinuxProcMapsParserTest)

#include "linuxprocmapsparsertest.moc"
