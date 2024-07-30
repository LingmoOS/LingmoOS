/*
SPDX-FileCopyrightText: 2022 David Edmundson <davidedmundson@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QSignalSpy>
#include <QTest>

#include "../greeter/pamauthenticator.h"

// This test runs under the expectation that
// we are run with LD_PRELOAD=libpam_wrapper.so pamTest

class PamTest : public QObject
{
    Q_OBJECT
public:
    PamTest();
private Q_SLOTS:
    void testLogin();
};

PamTest::PamTest()
{
    if (!qgetenv("LD_PRELOAD").contains("libpam_wrapper.so")) {
        qFatal("This test must be run with pam_wrapper. See ctest");
    }

    qputenv("PAM_WRAPPER", "1");
    qputenv("PAM_WRAPPER_DEBUGLEVEL", "2"); // DEBUG level
    qputenv("PAM_WRAPPER_SERVICE_DIR", QFINDTESTDATA("data").toUtf8());
    qputenv("PAM_MATRIX_PASSWD", QFINDTESTDATA("data/test_db").toUtf8());
}

void PamTest::testLogin()
{
    PamAuthenticator auth("test_service", "test_user");
    QSignalSpy promptSpy(&auth, &PamAuthenticator::prompt);
    QSignalSpy promptForSecretSpy(&auth, &PamAuthenticator::promptForSecret);
    QSignalSpy succeededSpy(&auth, &PamAuthenticator::succeeded);
    QSignalSpy failedSpy(&auth, &PamAuthenticator::failed);
    QSignalSpy busyChangedSpy(&auth, &PamAuthenticator::busyChanged);

    // invalid password
    auth.tryUnlock();

    QVERIFY(promptForSecretSpy.wait());
    auth.respond("not_my_password");
    QVERIFY(failedSpy.wait());

    QVERIFY(promptSpy.count() == 0);
    QVERIFY(succeededSpy.count() == 0);
    QVERIFY(busyChangedSpy.count() == 2); // changed to busy and back again.

    // try again, with the right password
    auth.tryUnlock();
    QVERIFY(promptForSecretSpy.wait());
    auth.respond("my_password");
    QVERIFY(succeededSpy.wait());

    QVERIFY(busyChangedSpy.count() == 4);
}

QTEST_MAIN(PamTest)
#include "pamtest.moc"
