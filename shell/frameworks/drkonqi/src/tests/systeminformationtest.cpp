/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QTest>

#include <config-drkonqi.h>

#include <KCoreAddons>

#include <errno.h>
#include <sys/utsname.h>

#include <systeminformation.h>

class SystemInformationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    static int uname(utsname *buf)
    {
        strcpy(buf->sysname, "FreeBSD");
        strcpy(buf->release, "1.0.0");
        strcpy(buf->machine, "x86_64");
        return 0;
    }

    void initTestCase()
    {
    }

    // NOTE: bugzillaOperatingSystem is not tested anywhere because it is based
    //   on the compilation platform and testing it fairly moot because of that

    void testLsb()
    {
        SystemInformation::Config config;
        config.basicOperatingSystem = "Linux"; // other parts getting filled depends on OS
        config.lsbReleasePath = QFINDTESTDATA("lsb_release"); // double binary
        config.osReleasePath.clear();
        config.unameFunc = (void *)&uname;

        SystemInformation info(config);
        QTRY_VERIFY(info.complete());

        QCOMPARE(info.bugzillaPlatform(), "openSUSE RPMs");
        QCOMPARE(info.operatingSystem(), "FreeBSD 1.0.0 x86_64");
        QCOMPARE(info.distributionPrettyName(), "KDE SUSE User Edition 5.16");
        QCOMPARE(info.compiledSources(), false);
        QCOMPARE(info.qtVersion(), qVersion());
        QCOMPARE(info.frameworksVersion(), KCoreAddons::versionString());
    }

    void testOsRelease()
    {
        SystemInformation::Config config;
        config.basicOperatingSystem = "Linux"; // other parts getting filled depends on OS
        config.lsbReleasePath.clear();
        config.osReleasePath = QFINDTESTDATA("data/os-release"); // fixture
        config.unameFunc = (void *)&uname;

        SystemInformation info(config);
        QTRY_VERIFY(info.complete());

        QCOMPARE(info.bugzillaPlatform(), "FreeBSD Ports");
        QCOMPARE(info.operatingSystem(), "FreeBSD 1.0.0 x86_64");
        QCOMPARE(info.distributionPrettyName(), "FreeBSD #1");
        QCOMPARE(info.compiledSources(), false);
        QCOMPARE(info.qtVersion(), qVersion());
        QCOMPARE(info.frameworksVersion(), KCoreAddons::versionString());
    }
};

QTEST_GUILESS_MAIN(SystemInformationTest)

#include "systeminformationtest.moc"
