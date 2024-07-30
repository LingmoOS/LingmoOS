/*
    SPDX-FileCopyrightText: 2019 Milian Wolff <mail@milianw.de>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include <QAbstractItemModel>
#include <QObject>
#include <QTest>

#include "kcolorscheme.h"
#include "kcolorschememanager.h"

class KColorSchemeTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void benchConstruction_data()
    {
        KColorSchemeManager manager;
        if (manager.model()->rowCount() <= 1) {
            QSKIP("no scheme files found, cannot run benchmark");
        }

        const auto anyScheme = manager.model()->index(1, 0).data(Qt::UserRole).toString();
        QVERIFY(QFile::exists(anyScheme));

        QTest::addColumn<QString>("file");

        QTest::newRow("default") << QString();
        QTest::newRow("explicit") << anyScheme;
    }

    void benchConstruction()
    {
        QFETCH(QString, file);
        qApp->setProperty("KDE_COLOR_SCHEME_PATH", file);

        QBENCHMARK {
            KColorScheme scheme(QPalette::Active);
        }
    }
    void readColors_data()
    {
        QTest::addColumn<int>("colorSet");

        QTest::newRow("View") << static_cast<int>(KColorScheme::View);
        QTest::newRow("Window") << static_cast<int>(KColorScheme::Window);
        QTest::newRow("Button") << static_cast<int>(KColorScheme::Button);
        QTest::newRow("Selection") << static_cast<int>(KColorScheme::Selection);
        QTest::newRow("Tooltip") << static_cast<int>(KColorScheme::Tooltip);
        QTest::newRow("Complementary") << static_cast<int>(KColorScheme::Complementary);
        QTest::newRow("Header") << static_cast<int>(KColorScheme::Header);
    }

    void readColors()
    {
        QFETCH(int, colorSet);
        auto file = QFINDTESTDATA("kcolorschemetest.colors");
        KColorScheme activeScheme(QPalette::Active, static_cast<KColorScheme::ColorSet>(colorSet), KSharedConfig::openConfig(file, KConfig::SimpleConfig));
        KColorScheme inactiveScheme(QPalette::Inactive, static_cast<KColorScheme::ColorSet>(colorSet), KSharedConfig::openConfig(file, KConfig::SimpleConfig));

#define checkBackgroundRole(role)                                                                                                                              \
    QCOMPARE(activeScheme.background(role).color(), QColor(colorSet, role, QPalette::Active));                                                                 \
    QCOMPARE(inactiveScheme.background(role).color(), QColor(colorSet, role, QPalette::Inactive));

        checkBackgroundRole(KColorScheme::NormalBackground);
        checkBackgroundRole(KColorScheme::AlternateBackground);

#define checkForegroundRole(role)                                                                                                                              \
    QCOMPARE(activeScheme.foreground(role).color(), QColor(colorSet, role + KColorScheme::NBackgroundRoles, QPalette::Active));                                \
    QCOMPARE(inactiveScheme.foreground(role).color(), QColor(colorSet, role + KColorScheme::NBackgroundRoles, QPalette::Inactive));

        checkForegroundRole(KColorScheme::NormalText);
        checkForegroundRole(KColorScheme::InactiveText);
        checkForegroundRole(KColorScheme::ActiveText);
        checkForegroundRole(KColorScheme::LinkText);
        checkForegroundRole(KColorScheme::VisitedText);
        checkForegroundRole(KColorScheme::NegativeText);
        checkForegroundRole(KColorScheme::NeutralText);
        checkForegroundRole(KColorScheme::PositiveText);

#define checkDecorationRole(role)                                                                                                                              \
    QCOMPARE(activeScheme.decoration(role).color(),                                                                                                            \
             QColor(colorSet, role + KColorScheme::NBackgroundRoles + KColorScheme::NForegroundRoles, QPalette::Active));                                      \
    QCOMPARE(inactiveScheme.decoration(role).color(),                                                                                                          \
             QColor(colorSet, role + KColorScheme::NBackgroundRoles + KColorScheme::NForegroundRoles, QPalette::Inactive));

        checkDecorationRole(KColorScheme::FocusColor);
        checkDecorationRole(KColorScheme::HoverColor);
    }

    void readContrast()
    {
        auto file = QFINDTESTDATA("kcolorschemetest.colors");
        QCOMPARE(KColorScheme::contrastF(KSharedConfig::openConfig(file, KConfig::SimpleConfig)), 0.5);
    }
};

QTEST_MAIN(KColorSchemeTest)

#include "kcolorschemetest.moc"
