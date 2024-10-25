/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "themetest.h"
#include <QApplication>
#include <QSignalSpy>
#include <QStandardPaths>

#include <KConfigGroup>
#include <KIconLoader>
#include <KIconTheme>
#include <KWindowSystem>
#include <KX11Extras>

#include <config-lingmo.h>
#if HAVE_X11
#include <KSelectionOwner>
#endif
#include <array>

void ThemeTest::initTestCase()
{
    // make our theme in search path
    qputenv("XDG_DATA_DIRS", QByteArray(qgetenv("XDG_DATA_DIRS") + ":" + QFINDTESTDATA("data").toLocal8Bit()));

    // set default icon theme to test-theme
    QStandardPaths::setTestModeEnabled(true);

    m_theme = new Lingmo::Theme("testtheme", this);
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);

    if (!QDir(configPath).mkpath(QStringLiteral("."))) {
        qFatal("Failed to create test configuration directory.");
    }

    QFile::remove(configPath);

    QIcon::setThemeSearchPaths(QStringList() << QFINDTESTDATA("data/icons"));

    KConfigGroup lingmoConfig(KSharedConfig::openConfig("lingmorc"), "Theme");
    lingmoConfig.writeEntry("name", "default");

    KIconTheme::forceThemeForTests("test-theme");
    KSharedConfig::openConfig()->reparseConfiguration();
    KIconTheme::reconfigure();
    KIconLoader::global()->reconfigure(QString());
}

void ThemeTest::testThemeConfig_data()
{
    QTest::addColumn<QString>("themeName");

    QTest::addRow("new metadata style theme") << QStringLiteral("testtheme");
}

void ThemeTest::testThemeConfig()
{
    QFETCH(QString, themeName);
    auto theme = std::make_unique<Lingmo::Theme>(themeName, this);
    QCOMPARE(theme->backgroundContrastEnabled(), true);
    QCOMPARE(theme->backgroundContrast(), 0.23);
}

void ThemeTest::testColors()
{
    QCOMPARE(m_theme->color(Lingmo::Theme::TextColor, Lingmo::Theme::NormalColorGroup), QColor(49, 54, 59));
    QCOMPARE(m_theme->color(Lingmo::Theme::BackgroundColor, Lingmo::Theme::NormalColorGroup), QColor(239, 240, 241));
    QCOMPARE(m_theme->color(Lingmo::Theme::HighlightColor, Lingmo::Theme::NormalColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Lingmo::Theme::HoverColor, Lingmo::Theme::NormalColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Lingmo::Theme::FocusColor, Lingmo::Theme::NormalColorGroup), QColor(30, 146, 255));
    QCOMPARE(m_theme->color(Lingmo::Theme::LinkColor, Lingmo::Theme::NormalColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Lingmo::Theme::VisitedLinkColor, Lingmo::Theme::NormalColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Lingmo::Theme::HighlightedTextColor, Lingmo::Theme::NormalColorGroup), QColor(252, 252, 252));
    QCOMPARE(m_theme->color(Lingmo::Theme::PositiveTextColor, Lingmo::Theme::NormalColorGroup), QColor(17, 209, 22));
    QCOMPARE(m_theme->color(Lingmo::Theme::NeutralTextColor, Lingmo::Theme::NormalColorGroup), QColor(201, 206, 59));
    QCOMPARE(m_theme->color(Lingmo::Theme::NegativeTextColor, Lingmo::Theme::NormalColorGroup), QColor(237, 21, 21));

    QCOMPARE(m_theme->color(Lingmo::Theme::TextColor, Lingmo::Theme::ButtonColorGroup), QColor(49, 54, 59));
    QCOMPARE(m_theme->color(Lingmo::Theme::BackgroundColor, Lingmo::Theme::ButtonColorGroup), QColor(239, 240, 241));
    QCOMPARE(m_theme->color(Lingmo::Theme::HighlightColor, Lingmo::Theme::ButtonColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Lingmo::Theme::HoverColor, Lingmo::Theme::ButtonColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Lingmo::Theme::FocusColor, Lingmo::Theme::ButtonColorGroup), QColor(30, 146, 255));
    QCOMPARE(m_theme->color(Lingmo::Theme::LinkColor, Lingmo::Theme::ButtonColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Lingmo::Theme::VisitedLinkColor, Lingmo::Theme::ButtonColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Lingmo::Theme::HighlightedTextColor, Lingmo::Theme::ButtonColorGroup), QColor(252, 252, 252));
    QCOMPARE(m_theme->color(Lingmo::Theme::PositiveTextColor, Lingmo::Theme::ButtonColorGroup), QColor(17, 209, 23));
    QCOMPARE(m_theme->color(Lingmo::Theme::NeutralTextColor, Lingmo::Theme::ButtonColorGroup), QColor(201, 206, 60));
    QCOMPARE(m_theme->color(Lingmo::Theme::NegativeTextColor, Lingmo::Theme::ButtonColorGroup), QColor(237, 21, 22));

    QCOMPARE(m_theme->color(Lingmo::Theme::TextColor, Lingmo::Theme::ViewColorGroup), QColor(49, 54, 59));
    QCOMPARE(m_theme->color(Lingmo::Theme::BackgroundColor, Lingmo::Theme::ViewColorGroup), QColor(252, 252, 252));
    QCOMPARE(m_theme->color(Lingmo::Theme::HighlightColor, Lingmo::Theme::ViewColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Lingmo::Theme::HoverColor, Lingmo::Theme::ViewColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Lingmo::Theme::FocusColor, Lingmo::Theme::ViewColorGroup), QColor(30, 146, 255));
    QCOMPARE(m_theme->color(Lingmo::Theme::LinkColor, Lingmo::Theme::ViewColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Lingmo::Theme::VisitedLinkColor, Lingmo::Theme::ViewColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Lingmo::Theme::HighlightedTextColor, Lingmo::Theme::ViewColorGroup), QColor(252, 252, 252));
    QCOMPARE(m_theme->color(Lingmo::Theme::PositiveTextColor, Lingmo::Theme::ViewColorGroup), QColor(17, 209, 24));
    QCOMPARE(m_theme->color(Lingmo::Theme::NeutralTextColor, Lingmo::Theme::ViewColorGroup), QColor(201, 206, 61));
    QCOMPARE(m_theme->color(Lingmo::Theme::NegativeTextColor, Lingmo::Theme::ViewColorGroup), QColor(237, 21, 23));

    QCOMPARE(m_theme->color(Lingmo::Theme::TextColor, Lingmo::Theme::ComplementaryColorGroup), QColor(239, 240, 241));
    QCOMPARE(m_theme->color(Lingmo::Theme::BackgroundColor, Lingmo::Theme::ComplementaryColorGroup), QColor(49, 54, 59));
    QCOMPARE(m_theme->color(Lingmo::Theme::HighlightColor, Lingmo::Theme::ComplementaryColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Lingmo::Theme::HoverColor, Lingmo::Theme::ComplementaryColorGroup), QColor(71, 174, 230));
    QCOMPARE(m_theme->color(Lingmo::Theme::FocusColor, Lingmo::Theme::ComplementaryColorGroup), QColor(40, 146, 255));
    QCOMPARE(m_theme->color(Lingmo::Theme::LinkColor, Lingmo::Theme::ComplementaryColorGroup), QColor(71, 174, 230));
    QCOMPARE(m_theme->color(Lingmo::Theme::VisitedLinkColor, Lingmo::Theme::ComplementaryColorGroup), QColor(71, 174, 230));
    QCOMPARE(m_theme->color(Lingmo::Theme::HighlightedTextColor, Lingmo::Theme::ComplementaryColorGroup), QColor(252, 252, 252));
    QCOMPARE(m_theme->color(Lingmo::Theme::PositiveTextColor, Lingmo::Theme::ComplementaryColorGroup), QColor(17, 209, 25));
    QCOMPARE(m_theme->color(Lingmo::Theme::NeutralTextColor, Lingmo::Theme::ComplementaryColorGroup), QColor(201, 206, 62));
    QCOMPARE(m_theme->color(Lingmo::Theme::NegativeTextColor, Lingmo::Theme::ComplementaryColorGroup), QColor(237, 21, 24));
}

void ThemeTest::testCompositingChange()
{
    // this test simulates the compositing change on X11
#if HAVE_X11
    if (!KWindowSystem::isPlatformX11()) {
        QSKIP("Test is only for X11");
    }
    QVERIFY(!KX11Extras::compositingActive());

    // image path should give us an opaque variant
    QVERIFY(m_theme->imagePath(QStringLiteral("element")).endsWith(QLatin1String("/desktoptheme/testtheme/opaque/element.svg")));

    QSignalSpy themeChangedSpy(m_theme, &Lingmo::Theme::themeChanged);
    QVERIFY(themeChangedSpy.isValid());

    // fake the compositor
    QSignalSpy compositingChangedSpy(KX11Extras::self(), &KX11Extras::compositingChanged);
    QVERIFY(compositingChangedSpy.isValid());
    std::unique_ptr<KSelectionOwner> compositorSelection(new KSelectionOwner("_NET_WM_CM_S0"));
    QSignalSpy claimedSpy(compositorSelection.get(), &KSelectionOwner::claimedOwnership);
    QVERIFY(claimedSpy.isValid());
    compositorSelection->claim(true);
    QVERIFY(claimedSpy.wait());

    QCOMPARE(compositingChangedSpy.count(), 1);
    QVERIFY(KX11Extras::compositingActive());
    QVERIFY(themeChangedSpy.wait());
    QCOMPARE(themeChangedSpy.count(), 1);
    QVERIFY(m_theme->imagePath(QStringLiteral("element")).endsWith(QLatin1String("/desktoptheme/testtheme/element.svg")));

    // remove compositor again
    compositorSelection.reset();
    QVERIFY(compositingChangedSpy.wait());
    QCOMPARE(compositingChangedSpy.count(), 2);
    QVERIFY(!KX11Extras::compositingActive());
    QVERIFY(themeChangedSpy.wait());
    QCOMPARE(themeChangedSpy.count(), 2);
    QVERIFY(m_theme->imagePath(QStringLiteral("element")).endsWith(QLatin1String("/desktoptheme/testtheme/opaque/element.svg")));
#endif
}

QTEST_MAIN(ThemeTest)

#include "moc_themetest.cpp"
