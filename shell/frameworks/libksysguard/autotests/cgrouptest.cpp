/*
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include <QObject>
#include <QStandardPaths>
#include <QTest>
#define private public
#include "cgroup.h"
#define private private

class CGroupTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testAppUnitRegex_data()
    {
        QTest::addColumn<QString>("id");
        QTest::addColumn<QString>("desktopName");
        QTest::newRow("service") << "app-gnome-org.gnome.Evince@12345.service"
                                 << "org.gnome.Evince";
        QTest::newRow("service .desktop") << "app-flatpak-org.telegram.desktop@12345.service"
                                          << "org.telegram.desktop";
        QTest::newRow("service no launcher") << "app-org.kde.okular@12345.service"
                                             << "org.kde.okular";
        QTest::newRow("service no random") << "app-KDE-org.kde.okular.service"
                                           << "org.kde.okular";
        QTest::newRow("service no launcher no random") << "app-org.kde.amarok.service"
                                                       << "org.kde.amarok";
        QTest::newRow("scope") << "app-gnome-org.gnome.Evince-12345.scope"
                               << "org.gnome.Evince";
        QTest::newRow("scope no launcher") << "app-org.gnome.Evince-12345.scope"
                                           << "org.gnome.Evince";
    }

    void testAppUnitRegex()
    {
        QFETCH(QString, id);
        QFETCH(QString, desktopName);
        KSysGuard::CGroup c(id);
        if (c.service()->menuId().isEmpty()) {
            // The service is not known on this machine and we constructed a service with the id as name
            QCOMPARE(c.service()->name(), desktopName);
        } else {
            QCOMPARE(c.service()->desktopEntryName(), desktopName.toLower());
        }
    }

    void findAutostartApplication()
    {
        qputenv("XDG_CONFIG_DIRS", QFINDTESTDATA("data").toLocal8Bit());
        KSysGuard::CGroup cgroup("app-org.kde.korgac@autostart.service");
        QCOMPARE(cgroup.service()->desktopEntryName(), "org.kde.korgac");
        QCOMPARE(cgroup.service()->name(), "KOrganizer Reminder Client");
    }
};
QTEST_MAIN(CGroupTest);
#include "cgrouptest.moc"
