/*
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
*/

#include <QProcess>
#include <QTemporaryDir>
#include <QTest>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace std::chrono_literals;
namespace fs = std::filesystem;

class CleanupTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRun()
    {
        const QString binary = QFINDTESTDATA("drkonqi-coredump-cleanup");
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());

        const fs::path dir = tempDir.path().toStdString();
        const fs::path recentFile = fs::path(dir) / "recent.ini";
        const fs::path oldFile = fs::path(dir) / "old.ini";
        {
            std::ofstream output(recentFile);
        }
        {
            std::ofstream output(oldFile);
            const auto time = fs::last_write_time(oldFile);
            fs::last_write_time(oldFile, time - std::chrono::weeks(2));
        }

        const int exitCode = QProcess::execute(binary, {tempDir.path()});
        QCOMPARE(exitCode, 0);
        QVERIFY(fs::exists(recentFile));
        QVERIFY(!fs::exists(oldFile));
    }
};

QTEST_GUILESS_MAIN(CleanupTest)

#include "cleanuptest.moc"
