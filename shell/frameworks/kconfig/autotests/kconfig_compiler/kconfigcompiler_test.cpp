/*
    Tests for KConfig Compiler

    SPDX-FileCopyrightText: 2005 Duncan Mac-Vicar <duncan@kde.org>
    SPDX-FileCopyrightText: 2009 Pino Toscano <pino@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kconfigcompiler_test.h"
#include <QDir>
#include <QProcess>
#include <QString>
#include <QTest>
#include <qstandardpaths.h>

// QT5 TODO QTEST_GUILESS_MAIN(KConfigCompiler_Test)
QTEST_MAIN(KConfigCompiler_Test)

typedef const char *CompilerTestSet[];

static CompilerTestSet testCases = {"test1.cpp",
                                    "test1.h",
                                    "test2.cpp",
                                    "test2.h",
                                    "test3.cpp",
                                    "test3.h",
                                    "test3a.cpp",
                                    "test3a.h",
                                    "test4.cpp",
                                    "test4.h",
                                    "test5.cpp",
                                    "test5.h",
                                    "test6.cpp",
                                    "test6.h",
                                    "test7.cpp",
                                    "test7.h",
                                    "test8a.cpp",
                                    "test8a.h",
                                    "test8b.cpp",
                                    "test8b.h",
                                    "test8c.cpp",
                                    "test8c.h",
                                    "test9.h",
                                    "test9.cpp",
                                    "test10.h",
                                    "test10.cpp",
                                    "test11.h",
                                    "test11.cpp",
                                    "test11a.h",
                                    "test11a.cpp",
                                    "test12.h",
                                    "test12.cpp",
                                    "test13.h",
                                    "test13.cpp",
                                    "test14.h",
                                    "test14.cpp",
                                    "test15.h",
                                    "test15.cpp",
                                    "test_dpointer.cpp",
                                    "test_dpointer.h",
                                    "test_qdebugcategory.cpp",
                                    "test_qdebugcategory.h",
                                    "test_signal.cpp",
                                    "test_signal.h",
                                    "test_notifiers.cpp",
                                    "test_notifiers.h",
                                    "test_translation_kde.cpp",
                                    "test_translation_kde.h",
                                    "test_translation_kde_domain.cpp",
                                    "test_translation_kde_domain.h",
                                    "test_translation_qt.cpp",
                                    "test_translation_qt.h",
                                    "test_emptyentries.cpp",
                                    "test_emptyentries.h",
                                    "test_properties_minmax.cpp",
                                    "test_properties_minmax.h",
                                    "test_param_minmax.cpp",
                                    "test_param_minmax.h",
                                    "test_subgroups.cpp",
                                    "test_subgroups.h",
                                    nullptr};

static CompilerTestSet testCasesToRun = {"Test1",
                                         "Test2",
                                         "Test3",
                                         "Test3a",
                                         "Test4",
                                         "Test5",
                                         "Test6",
                                         "Test7",
                                         "Test8",
                                         "Test9",
                                         "Test10",
                                         "Test11",
                                         "Test12",
                                         "Test13",
                                         "Test14",
                                         "Test15",
                                         "test_enums_and_properties",
                                         "test_dpointer",
                                         "test_qdebugcategory",
                                         "test_signal",
                                         "test_translation_kde",
                                         "test_translation_kde_domain",
                                         "test_translation_qt",
                                         "test_emptyentries",
                                         "test_properties_minmax",
                                         "test_param_minmax",
                                         "test_subgroups",
                                         "Test1_cmake",
                                         "Test2_cmake",
                                         "Test3_cmake",
                                         "Test3a_cmake",
                                         "Test4_cmake",
                                         "Test5_cmake",
                                         "Test6_cmake",
                                         "Test7_cmake",
                                         "Test8_cmake",
                                         "Test9_cmake",
                                         "Test10_cmake",
                                         "Test11_cmake",
                                         "Test12_cmake",
                                         "Test13_cmake",
                                         "Test14_cmake",
                                         "Test15_cmake",
                                         "test_dpointer_cmake",
                                         "test_signal_cmake",
                                         "test_emptyentries_cmake",
                                         "test_subgroups_cmake",
                                         nullptr};

#if 0
static CompilerTestSet willFailCases = {
    // where is that QDir coming from?
    //"test9.cpp", NULL
    NULL
};
#endif

void KConfigCompiler_Test::initTestCase()
{
    m_diffExe = QStandardPaths::findExecutable(QStringLiteral("diff"));
    if (m_diffExe.isEmpty()) {
        qDebug() << "diff command not found, detailed info on comparison failure will not be available.";
    }
}

void KConfigCompiler_Test::testBaselineComparison_data()
{
    QTest::addColumn<QString>("testName");

    for (const char **it = testCases; *it; ++it) {
        QTest::newRow(*it) << QString::fromLatin1(*it);
    }
}

void KConfigCompiler_Test::testBaselineComparison()
{
    QFETCH(QString, testName);

    QFile file(testName);
    QFile fileRef(QFINDTESTDATA(testName + QLatin1String(".ref")));

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open" << file.fileName() << "(" << testName << ")";
        QFAIL("Can't open file for comparison");
    }
    if (!fileRef.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open" << fileRef.fileName() << "(" << testName << ".ref )";
        QFAIL("Can't open file for comparison");
    }
    const QByteArray content = file.readAll();
    const QByteArray contentRef = fileRef.readAll();

    if (content != contentRef) {
        appendFileDiff(fileRef.fileName(), file.fileName());
    }

    QVERIFY(content == contentRef);
}

void KConfigCompiler_Test::testRunning_data()
{
    QTest::addColumn<QString>("testName");

    for (const char **it = testCasesToRun; *it; ++it) {
        QTest::newRow(*it) << QString::fromLatin1(*it);
    }
}

void KConfigCompiler_Test::testRunning()
{
    QFETCH(QString, testName);

#ifdef Q_OS_WIN
    testName += QLatin1String{".exe"};
#endif

    QString program = QFINDTESTDATA(testName);
    QVERIFY2(!program.isEmpty(), qPrintable(testName + QLatin1String(" must exist!")));
    QVERIFY2(QFile::exists(program), qPrintable(program + QLatin1String(" must exist!")));
    QProcess process;
    process.start(program, QStringList(), QIODevice::ReadOnly);
    if (process.waitForStarted()) {
        QVERIFY(process.waitForFinished());
    }
    QCOMPARE((int)process.error(), (int)QProcess::UnknownError);
    QCOMPARE(process.exitCode(), 0);
}

void KConfigCompiler_Test::appendFileDiff(const QString &oldFile, const QString &newFile)
{
    if (m_diffExe.isEmpty()) {
        return;
    }

    QStringList args({QStringLiteral("-u"), QFileInfo(oldFile).absoluteFilePath(), QFileInfo(newFile).absoluteFilePath()});

    QProcess process;
    process.start(m_diffExe, args, QIODevice::ReadOnly);
    process.waitForStarted();
    process.waitForFinished();

    if (process.exitCode() == 1) {
        const QString diffFileName = oldFile + QStringLiteral(".diff");
        QFile diffFile(diffFileName);
        QVERIFY2(diffFile.open(QIODevice::WriteOnly), qPrintable(QLatin1String("Could not save diff file for ") + oldFile));
        diffFile.write(process.readAllStandardOutput());

        // force a failure to print the flename where we stored the diff.
        QVERIFY2(false, qPrintable(QLatin1String("This test failed, look at the following file for details: ") + diffFileName));
    }
}

#include "moc_kconfigcompiler_test.cpp"
