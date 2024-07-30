/*
    SPDX-FileCopyrightText: 2006 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/
#include "kdesktopfiletest.h"
#include "helper.h"
#include <kconfiggroup.h>
#include <kdesktopfileaction.h>
#include <ksharedconfig.h>
#include <qtemporaryfile.h>

#include "kdesktopfile.h"

#include <QTest>

#if defined(Q_OS_UNIX)
#include <unistd.h>
#endif

QTEST_MAIN(KDesktopFileTest)

void KDesktopFileTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);

    KConfigGroup actionRestrictions(KSharedConfig::openConfig(), QStringLiteral("KDE Action Restrictions"));
    actionRestrictions.writeEntry("someBlockedAction", false);
}

void KDesktopFileTest::testRead()
{
    qputenv("TESTVAR", "value");
    QTemporaryFile file(QDir::tempPath() + QStringLiteral("/testReadXXXXXX.desktop"));
    QVERIFY(file.open());
    const QString fileName = file.fileName();
    QTextStream ts(&file);
    ts << "[Desktop Entry]\n"
          "Type= Application\n"
          "Name=My Application\n"
          "Icon = foo\n"
          "URL[$e]=file:///tmp/$TESTVAR\n"
          "MimeType =text/plain;image/png;\n"
          "\n";
    file.close();
    QVERIFY(QFile::exists(fileName));
    QVERIFY(KDesktopFile::isDesktopFile(fileName));
    KDesktopFile df(fileName);
    QCOMPARE(df.readType(), QString::fromLatin1("Application"));
    QVERIFY(df.hasApplicationType());
    QCOMPARE(df.readName(), QString::fromLatin1("My Application"));
    QCOMPARE(df.readIcon(), QString::fromLatin1("foo"));
    QCOMPARE(df.readMimeTypes(), QStringList() << QString::fromLatin1("text/plain") << QString::fromLatin1("image/png"));
    QVERIFY(!df.hasLinkType());
    QCOMPARE(df.fileName(), QFileInfo(fileName).canonicalFilePath());
    QCOMPARE(df.readUrl(), QStringLiteral("file:///tmp/value"));
}

void KDesktopFileTest::testReadLocalized_data()
{
    QTest::addColumn<QLocale>("locale");
    QTest::addColumn<QString>("translation");

    const QString german = QStringLiteral("Meine Anwendung");
    const QString swiss = QStringLiteral("Mein Anwendungsli");

    QTest::newRow("de") << QLocale(QLocale::German) << german;
    QTest::newRow("de_DE") << QLocale(QStringLiteral("de_DE")) << german;
    QTest::newRow("de_DE@bayern") << QLocale(QStringLiteral("de_DE@bayern")) << german;
    QTest::newRow("de@bayern") << QLocale(QStringLiteral("de@bayern")) << german;
    QTest::newRow("de@freiburg") << QLocale(QStringLiteral("de@freiburg")) << QStringLiteral("Mein Anwendungsle");
    // For CH we have a special translation
    QTest::newRow("de_CH") << QLocale(QLocale::German, QLocale::Switzerland) << swiss;
    QTest::newRow("de_CH@bern") << QLocale(QStringLiteral("de_CH@bern")) << swiss;
    // Austria should fall back to "de"
    QTest::newRow("de_AT") << QLocale(QLocale::German, QLocale::Austria) << german;
    QTest::newRow("de_AT@tirol") << QLocale(QStringLiteral("de_AT@tirol")) << german;
    // no translation for French
    QTest::newRow("fr") << QLocale(QLocale::French) << QStringLiteral("My Application");
}

void KDesktopFileTest::testReadLocalized()
{
    QTemporaryFile file(QDir::tempPath() + QStringLiteral("/testReadLocalizedXXXXXX.desktop"));
    QVERIFY(file.open());
    const QString fileName = file.fileName();
    QTextStream ts(&file);
    ts << "[Desktop Entry]\n"
          "Type=Application\n"
          "Name=My Application\n"
          "Name[de]=Meine Anwendung\n"
          "Name[de@freiburg]=Mein Anwendungsle\n"
          "Name[de_CH]=Mein Anwendungsli\n"
          "Icon=foo\n"
          "\n";
    file.close();
    QVERIFY(QFile::exists(fileName));
    QVERIFY(KDesktopFile::isDesktopFile(fileName));

    DefaultLocale defaultLocale;

    QFETCH(QLocale, locale);
    QLocale::setDefault(locale);
    KDesktopFile df(fileName);

    QEXPECT_FAIL("de@freiburg", "QLocale doesn't support modifiers", Continue);
    QTEST(df.readName(), "translation");
}

void KDesktopFileTest::testSuccessfulTryExec()
{
    QTemporaryFile file(QDir::tempPath() + QStringLiteral("/testSuccessfulTryExecXXXXXX.desktop"));
    QVERIFY(file.open());
    const QString fileName = file.fileName();
    QTextStream ts(&file);
    ts << "[Desktop Entry]\n"
          "TryExec=whoami\n"
          "\n";
    file.close();
    QVERIFY(QFile::exists(fileName));
    KDesktopFile df(fileName);
    QCOMPARE(df.tryExec(), true);
}

void KDesktopFileTest::testUnsuccessfulTryExec()
{
    QTemporaryFile file(QDir::tempPath() + QStringLiteral("/testUnsuccessfulTryExecXXXXXX.desktop"));
    QVERIFY(file.open());
    const QString fileName = file.fileName();
    QTextStream ts(&file);
    ts << "[Desktop Entry]\n"
          "TryExec=/does/not/exist\n"
          "\n";
    file.close();
    QVERIFY(QFile::exists(fileName));
    KDesktopFile df(fileName);
    QCOMPARE(df.tryExec(), false);
}

void KDesktopFileTest::testActionGroup()
{
    QTemporaryFile file(QDir::tempPath() + QStringLiteral("/testActionGroupXXXXXX.desktop"));
    QVERIFY(file.open());
    const QString fileName = file.fileName();
    QTextStream ts(&file);
    ts << "[Desktop Entry]\n"
          // make sure escaping of ';' using "\;" works
          "Actions=encrypt;semi\\;colon;decrypt;\n"
          "[Desktop Action encrypt]\n"
          "Name=Encrypt file\n"
          "[Desktop Action decrypt]\n"
          "Name=Decrypt file\n"
          // no escaping needed in group header
          "[Desktop Action semi;colon]\n"
          "Name=With semicolon\n"
          "\n";
    file.close();
    QVERIFY(QFile::exists(fileName));
    KDesktopFile df(fileName);
    QCOMPARE(df.readType(), QString());
    QCOMPARE(df.fileName(), QFileInfo(fileName).canonicalFilePath());
    QCOMPARE(df.readActions(), (QStringList{QStringLiteral("encrypt"), QStringLiteral("semi;colon"), QStringLiteral("decrypt")}));
    QCOMPARE(df.hasActionGroup(QStringLiteral("encrypt")), true);
    QCOMPARE(df.hasActionGroup(QStringLiteral("semi;colon")), true);
    QCOMPARE(df.hasActionGroup(QStringLiteral("decrypt")), true);
    QCOMPARE(df.hasActionGroup(QStringLiteral("doesnotexist")), false);

    const auto actions = df.actions();
    QCOMPARE(actions.size(), 3);
    QVERIFY(std::all_of(actions.begin(), actions.end(), [&fileName](const KDesktopFileAction &action) {
        return action.desktopFilePath() == fileName;
    }));
    QCOMPARE(actions.at(0).actionsKey(), QStringLiteral("encrypt"));
    QCOMPARE(actions.at(0).name(), QStringLiteral("Encrypt file"));
    QCOMPARE(actions.at(1).actionsKey(), QStringLiteral("semi;colon"));
    QCOMPARE(actions.at(1).name(), QStringLiteral("With semicolon"));
    QCOMPARE(actions.at(2).name(), QStringLiteral("Decrypt file"));
    QCOMPARE(actions.at(2).actionsKey(), QStringLiteral("decrypt"));
    QCOMPARE(actions.at(2).name(), QStringLiteral("Decrypt file"));
}

void KDesktopFileTest::testIsAuthorizedDesktopFile()
{
    QTemporaryFile file(QDir::tempPath() + QStringLiteral("/testAuthXXXXXX.desktop"));
    QVERIFY(file.open());
    const QString fileName = file.fileName();
    QTextStream ts(&file);
    ts << "[Desktop Entry]\n"
          "Type=Application\n"
          "Name=My Application\n"
          "Exec=kfoo\n"
          "\n";
    file.close();
    QVERIFY(QFile::exists(fileName));

#if defined(Q_OS_UNIX)
    // Try to fix test failing in docker running as root
    const QFileInfo entryInfo(fileName);
    if (entryInfo.ownerId() == 0) {
        // try to find a valid user/group combination
        for (int i = 1; i < 100; ++i) {
            if (chown(fileName.toLocal8Bit().constData(), i, i) == 0) {
                break;
            }
        }
        const QFileInfo entryInfo1(fileName);
        if (entryInfo1.ownerId() == 0) {
            QEXPECT_FAIL("", "Running test as root and could not find a non root user to change the ownership of the file too", Continue);
        }
    }
#endif

    QVERIFY(!KDesktopFile::isAuthorizedDesktopFile(fileName));

    const QString autostartFile = QStandardPaths::locate(QStandardPaths::GenericConfigLocation, QStringLiteral("autostart/plasma-desktop.desktop"));
    if (!autostartFile.isEmpty()) {
        QVERIFY(KDesktopFile::isAuthorizedDesktopFile(autostartFile));
    } else {
        qWarning("Skipping test for plasma-desktop.desktop, not found. kde-workspace not installed?");
    }
}

void KDesktopFileTest::testTryExecWithAuthorizeAction()
{
    {
        QTemporaryFile file(QDir::tempPath() + QStringLiteral("/testAuthActionXXXXXX.desktop"));
        QVERIFY(file.open());
        const QString fileName = file.fileName();
        QTextStream ts(&file);
        ts << "[Desktop Entry]\n"
              "Type=Application\n"
              "Name=My Application\n"
              "Exec=kfoo\n"
              "TryExec=";
#ifdef Q_OS_WIN
        ts << "cmd\n";
#else
        ts << "bash\n";
#endif
        ts << "X-KDE-AuthorizeAction=someAction"
              "\n";
        file.close();

        KDesktopFile desktopFile(fileName);
        QVERIFY(desktopFile.tryExec());
    }
    {
        QTemporaryFile file(QDir::tempPath() + QStringLiteral("/testAuthActionXXXXXX.desktop"));
        QVERIFY(file.open());
        const QString fileName = file.fileName();
        QTextStream ts(&file);
        ts << "[Desktop Entry]\n"
              "Type=Application\n"
              "Name=My Application\n"
              "Exec=kfoo\n"
              "TryExec=bash\n"
              "X-KDE-AuthorizeAction=someBlockedAction"
              "\n";
        file.close();

        KDesktopFile desktopFile(fileName);
        QVERIFY(!desktopFile.tryExec());
    }
}

void KDesktopFileTest::testLocateLocal_data()
{
    const QString systemConfigLocation = QStandardPaths::standardLocations(QStandardPaths::GenericConfigLocation).constLast();
    const QString writableConfigLocation = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    const QString systemDataLocation = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).constLast();
    const QString writableDataLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);

    QTest::addColumn<QString>("path");
    QTest::addColumn<QString>("result");

    QTest::newRow("configLocation, system-wide") << systemConfigLocation + QLatin1String{"/test.desktop"}
                                                 << writableConfigLocation + QLatin1String{"/test.desktop"};
    QTest::newRow("autostart, system-wide") << systemConfigLocation + QLatin1String{"/autostart/test.desktop"}
                                            << writableConfigLocation + QLatin1String{"/autostart/test.desktop"};
    QTest::newRow("dataLocation, system-wide") << systemDataLocation + QLatin1String{"/test.desktop"} << writableDataLocation + QLatin1String{"/test.desktop"};
    QTest::newRow("applications, system-wide") << systemDataLocation + QLatin1String{"/applications/test.desktop"}
                                               << writableDataLocation + QLatin1String{"/applications/test.desktop"};
    QTest::newRow("desktop-directories, system-wide") << systemDataLocation + QLatin1String{"/desktop-directories/test.directory"}
                                                      << writableDataLocation + QLatin1String{"/desktop-directories/test.directory"};
    QTest::newRow("configLocation, writable") << writableConfigLocation + QLatin1String{"/test.desktop"}
                                              << writableConfigLocation + QLatin1String{"/test.desktop"};
    QTest::newRow("autostart, writable") << writableConfigLocation + QLatin1String{"/autostart/test.desktop"}
                                         << writableConfigLocation + QLatin1String{"/autostart/test.desktop"};
    QTest::newRow("dataLocation, writable") << writableDataLocation + QLatin1String{"/test.desktop"} << writableDataLocation + QLatin1String{"/test.desktop"};
    QTest::newRow("applications, writable") << writableDataLocation + QLatin1String{"/applications/test.desktop"}
                                            << writableDataLocation + QLatin1String{"/applications/test.desktop"};
    QTest::newRow("desktop-directories, writable") << writableDataLocation + QLatin1String{"/desktop-directories/test.directory"}
                                                   << writableDataLocation + QLatin1String{"/desktop-directories/test.directory"};
    QTest::newRow("unknown location") << QStringLiteral("/test.desktop") << writableDataLocation + QLatin1String{"/test.desktop"};
}

void KDesktopFileTest::testLocateLocal()
{
    QFETCH(QString, path);
    QFETCH(QString, result);

    QCOMPARE(KDesktopFile::locateLocal(path), result);
}

#include "moc_kdesktopfiletest.cpp"
