// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2026 LingmoOS Contributors
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <sys/stat.h>

#include <gtest/gtest.h>

#include <QString>
#include <QStringList>

#include "autostartentries.h"
#include "base/file.h"
#include "testutil.h"

using Lingmo::Base::file::TemporaryDirectory;

static void entry(const QString &dir, const QString &name, const QString &values)
{
    ASSERT_TRUE(SessionTest::writeFile(
        dir + QLatin1Char('/') + name + QStringLiteral(".desktop"),
        QStringLiteral("[Desktop Entry]\nType=Application\n") + values));
}

static bool contains(const QStringList &entries, const QString &value)
{
    return entries.contains(value);
}

TEST(Autostart, overrides)
{
    TemporaryDirectory root(SessionTest::temporaryParent().toStdString());
    const QString user   = QString::fromStdString(root.path()) + QStringLiteral("/user");
    const QString system = QString::fromStdString(root.path()) + QStringLiteral("/system");

    entry(system, "bridge",      QStringLiteral("Exec=bridge\n"));
    entry(user,   "bridge",      QStringLiteral("Hidden=true\n"));
    entry(system, "switched-off",QStringLiteral("Exec=switched-off\n"));
    entry(user,   "switched-off",QStringLiteral("Exec=switched-off\nX-GNOME-Autostart-enabled=false\n"));
    entry(system, "duplicate",   QStringLiteral("Exec=old\n"));
    entry(user,   "duplicate",   QStringLiteral("Exec=new\n"));
    entry(system, "filtered",    QStringLiteral("Exec=wrong-fallback\n"));
    entry(user,   "filtered",    QStringLiteral("Exec=other\nOnlyShowIn=KDE;\n"));

    EXPECT_EQ(Lingmo::autostartEntries({user, system, system}, {QStringLiteral("Lingmo")}),
              QStringList{QStringLiteral("new")});
}

TEST(Autostart, filtering)
{
    TemporaryDirectory root(SessionTest::temporaryParent().toStdString());
    const QString dir = QString::fromStdString(root.path());

    entry(dir, "allowed",     QStringLiteral("Exec=allowed\nOnlyShowIn=KDE;Lingmo;\n"));
    entry(dir, "excluded",    QStringLiteral("Exec=excluded\nNotShowIn=Lingmo;\n"));
    entry(dir, "other",       QStringLiteral("Exec=other\nOnlyShowIn=KDE;\n"));
    entry(dir, "hidden",      QStringLiteral("Exec=hidden\nHidden=true\n"));
    entry(dir, "menu-hidden", QStringLiteral("Exec=menu-hidden\nNoDisplay=true\n"));
    entry(dir, "visible",     QStringLiteral("Exec=visible\nHidden=false\n"));
    entry(dir, "disabled",    QStringLiteral("Exec=disabled\nX-GNOME-Autostart-enabled=false\n"));
    entry(dir, "enabled",     QStringLiteral("Exec=enabled\nX-GNOME-Autostart-enabled=true\n"));

    EXPECT_EQ(Lingmo::autostartEntries({dir}, {QStringLiteral("Lingmo")}),
              (QStringList{QStringLiteral("allowed"), QStringLiteral("enabled"),
                           QStringLiteral("menu-hidden"), QStringLiteral("visible")}));
    EXPECT_TRUE(contains(Lingmo::autostartEntries({dir}, {QStringLiteral("Other"), QStringLiteral("KDE")}),
                         QStringLiteral("other")));
}

TEST(Autostart, tryExec)
{
    TemporaryDirectory root(SessionTest::temporaryParent().toStdString());
    const QString dir     = QString::fromStdString(root.path());
    const QString program = dir + QStringLiteral("/executable");

    ASSERT_TRUE(SessionTest::writeFile(program, QString()));
    ASSERT_EQ(::chmod(program.toLocal8Bit().constData(), 0600), 0);

    entry(dir, "absolute", QStringLiteral("Exec=absolute\nTryExec=") + program + QStringLiteral("\n"));
    entry(dir, "missing",  QStringLiteral("Exec=missing\nTryExec=lingmo-nonexistent-test-program\n"));
    entry(dir, "relative", QStringLiteral("Exec=relative\nTryExec=./executable\n"));

    EXPECT_TRUE(Lingmo::autostartEntries({dir}, {QStringLiteral("Lingmo")}).isEmpty());

    ASSERT_EQ(::chmod(program.toLocal8Bit().constData(), 0700), 0);
    EXPECT_EQ(Lingmo::autostartEntries({dir}, {QStringLiteral("Lingmo")}),
              QStringList{QStringLiteral("absolute")});

    entry(dir, "path", QStringLiteral("Exec=path\nTryExec=sh\n"));
    EXPECT_TRUE(contains(Lingmo::autostartEntries({dir}, {QStringLiteral("Lingmo")}),
                         QStringLiteral("path")));
}

TEST(Autostart, sessionPrograms)
{
    EXPECT_TRUE(Lingmo::isStartedBySession(QStringLiteral("lingmo-polkit-agent")));
    EXPECT_TRUE(Lingmo::isStartedBySession(QStringLiteral("lingmo-services")));
    EXPECT_FALSE(Lingmo::isStartedBySession(QStringLiteral("lingmo-calendar-reminders")));
}
