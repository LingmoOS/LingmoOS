/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2016 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/
#include "../src/platformtheme/khintssettings.h"
#include <KSharedConfig>
#include <QDialogButtonBox>
#include <QTest>
#include <config-platformtheme.h>

class KHintsSettingsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testDefaults();
};

void KHintsSettingsTest::testDefaults()
{
    // this test verifies that default are correctly loaded if there is no config yet
    KSharedConfig::Ptr config = KSharedConfig::openConfig(QString(), KConfig::SimpleConfig);
    KHintsSettings hints(config);

    QCOMPARE(hints.hint(QPlatformTheme::CursorFlashTime).toInt(), 1000);
    QCOMPARE(hints.hint(QPlatformTheme::MouseDoubleClickInterval).toInt(), 400);
    QCOMPARE(hints.hint(QPlatformTheme::StartDragDistance).toInt(), 10);
    QCOMPARE(hints.hint(QPlatformTheme::StartDragTime).toInt(), 500);

    QCOMPARE(hints.hint(QPlatformTheme::ToolButtonStyle).toInt(), int(Qt::ToolButtonTextBesideIcon));

    QCOMPARE(hints.hint(QPlatformTheme::ToolBarIconSize).toInt(), 22);
    QCOMPARE(hints.hint(QPlatformTheme::ItemViewActivateItemOnSingleClick).toBool(), false);

    QCOMPARE(hints.hint(QPlatformTheme::SystemIconThemeName).toString(), QStringLiteral("breeze"));
    QCOMPARE(hints.hint(QPlatformTheme::SystemIconFallbackThemeName).toString(), QStringLiteral("hicolor"));
    QCOMPARE(hints.hint(QPlatformTheme::IconThemeSearchPaths).toStringList(), hints.xdgIconThemePaths());

    const QStringList expectedStyles = QStringList{QStringLiteral("breeze"), QStringLiteral("oxygen"), QStringLiteral("fusion"), QStringLiteral("windows")};
    QCOMPARE(hints.hint(QPlatformTheme::StyleNames).toStringList(), expectedStyles);

    QCOMPARE(hints.hint(QPlatformTheme::DialogButtonBoxLayout).toInt(), int(QDialogButtonBox::KdeLayout));
    QCOMPARE(hints.hint(QPlatformTheme::DialogButtonBoxButtonsHaveIcons).toBool(), true);
    QCOMPARE(hints.hint(QPlatformTheme::UseFullScreenForPopupMenu).toBool(), true);
    QCOMPARE(hints.hint(QPlatformTheme::KeyboardScheme).toInt(), int(QPlatformTheme::KdeKeyboardScheme));
    QCOMPARE(hints.hint(QPlatformTheme::UiEffects).toInt(), QPlatformTheme::HoverEffect);
    QCOMPARE(hints.hint(QPlatformTheme::IconPixmapSizes).value<QList<int>>(), QList<int>({512, 256, 128, 64, 32, 22, 16, 8}));
    QCOMPARE(hints.hint(QPlatformTheme::WheelScrollLines).toInt(), 3);
}

QTEST_GUILESS_MAIN(KHintsSettingsTest)

#include "khintssettings_unittest.moc"
