/*
 *  * Copyright (C) 2024, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: amingamingaming <wangyiming01@kylinos.cn>
 *
 */
#include <QtTest>
#include "lingmo-theme-proxy.h"
#include <QGSettings/QGSettings>
#include <QGuiApplication>
#include <QStyle>
#include <QColor>
using namespace LingmoUIQuick;

#define CONTROL_CENTER_SETTING      "org.lingmo.control-center.personalise"
#define LINGMO_STYLE_SETTING          "org.lingmo.style"
#define LINGMO_STYLE_NAME_KEY         "styleName"
#define LINGMO_STYLE_THEME_COLOR_KEY  "themeColor"
#define LINGMO_STYLE_SYSTEM_FONT_KEY  "systemFont"
#define LINGMO_STYLE_SYSTEM_FONT_SIZE "systemFontSize"
#define LINGMO_STYLE_ICON_THEME_NAME_KEY "iconThemeName"
#define LINGMO_STYLE_WINDOW_RADIUS_KEY "windowRadius"
#define CONTROL_CENTER_TRANSPARENCY_KEY "transparency"

class lingmoThemeProxy_test : public QObject
{
    Q_OBJECT

private slots:
    void test_font();
    void test_palette();
    void test_fontSize();
    void test_fontFamily();
    void test_themeName();
    void test_themeColor();
    void test_isDarkTheme();
    void test_themeTransparency();
    void test_layoutDirection();
    void test_windowRadius();
    void test_maxRadius();
    void test_normalRadius();
    void test_minRadius();
    void test_window();
    void test_windowText();
    void test_base();
    void test_text();
    void test_alternateBase();
    void test_button();
    void test_buttonText();
    void test_light();
    void test_midLight();
    void test_dark();
    void test_mid();
    void test_shadow();
    void test_highlight();
    void test_highlightedText();
    void test_separator();
    void test_brightText();
    void test_link();
    void test_linkVisited();
    void test_toolTipBase();
    void test_toolTipText();
    void test_placeholderText();
    void test_colorWithThemeTransparency();
    void test_colorWithThemeTransparency_data();
    void test_colorWithCustomTransparency();
    void test_colorWithCustomTransparency_data();
};

void lingmoThemeProxy_test::test_font()
{
    QCOMPARE(Theme::instance()->font(), QGuiApplication::font());
}

void lingmoThemeProxy_test::test_palette()
{
    QCOMPARE(Theme::instance()->palette(), QGuiApplication::palette());
}

void lingmoThemeProxy_test::test_fontSize()
{
    QCOMPARE(Theme::instance()->font().pointSize(), QGuiApplication::font().pointSize());
}

void lingmoThemeProxy_test::test_fontFamily()
{
    QCOMPARE(Theme::instance()->font().family(), QGuiApplication::font().family());
}

void lingmoThemeProxy_test::test_themeName()
{
    auto settings = new QGSettings(LINGMO_STYLE_SETTING, QByteArray(), Theme::instance());
    QString themeName = settings->get(LINGMO_STYLE_NAME_KEY).toString();
    QCOMPARE(Theme::instance()->themeName(), themeName);
}

void lingmoThemeProxy_test::test_themeColor()
{
    auto settings = new QGSettings(LINGMO_STYLE_SETTING, QByteArray(), Theme::instance());
    QString themeColor = settings->get(LINGMO_STYLE_THEME_COLOR_KEY).toString();
    QCOMPARE(Theme::instance()->themeColor(), themeColor);
}

void lingmoThemeProxy_test::test_isDarkTheme()
{
    auto settings = new QGSettings(LINGMO_STYLE_SETTING, QByteArray(), Theme::instance());
    QString themeName = settings->get(LINGMO_STYLE_NAME_KEY).toString();
    QCOMPARE(Theme::instance()->isDarkTheme(), themeName == "lingmo-dark");

}

void lingmoThemeProxy_test::test_themeTransparency()
{
    auto settings = new QGSettings(CONTROL_CENTER_SETTING, QByteArray(), Theme::instance());
    auto themeTransparency = settings->get(CONTROL_CENTER_TRANSPARENCY_KEY).toReal();
    QCOMPARE(Theme::instance()->themeTransparency(), themeTransparency);
}

void lingmoThemeProxy_test::test_layoutDirection()
{
    QCOMPARE(Theme::instance()->layoutDirection(), QGuiApplication::layoutDirection());
}

void lingmoThemeProxy_test::test_windowRadius()
{
    auto settings = new QGSettings(LINGMO_STYLE_SETTING, QByteArray(), Theme::instance());
    auto radius = settings->get(LINGMO_STYLE_WINDOW_RADIUS_KEY).toInt();
    QCOMPARE(Theme::instance()->windowRadius(), radius);
}

void lingmoThemeProxy_test::test_maxRadius()
{
    auto maxRadius = qApp->style()->property("maxRadius").toInt();
    QCOMPARE(Theme::instance()->maxRadius(), maxRadius);
}

void lingmoThemeProxy_test::test_normalRadius()
{
    auto normalRadius = qApp->style()->property("normalRadius").toInt();
    QCOMPARE(Theme::instance()->normalRadius(), normalRadius);
}

void lingmoThemeProxy_test::test_minRadius()
{
    auto minRadius = qApp->style()->property("minRadius").toInt();
    QCOMPARE(Theme::instance()->minRadius(), minRadius);
}

void lingmoThemeProxy_test::test_window()
{
    QCOMPARE(Theme::instance()->window(Theme::Active), QGuiApplication::palette().color(QPalette::Active, QPalette::Window));
    QCOMPARE(Theme::instance()->window(Theme::Disabled), QGuiApplication::palette().color(QPalette::Disabled, QPalette::Window));
    QCOMPARE(Theme::instance()->window(Theme::Inactive), QGuiApplication::palette().color(QPalette::Inactive, QPalette::Window));
}

void lingmoThemeProxy_test::test_windowText()
{
    QCOMPARE(Theme::instance()->windowText(Theme::Active), QGuiApplication::palette().color(QPalette::Active, QPalette::WindowText));
    QCOMPARE(Theme::instance()->windowText(Theme::Disabled), QGuiApplication::palette().color(QPalette::Disabled, QPalette::WindowText));
    QCOMPARE(Theme::instance()->windowText(Theme::Inactive), QGuiApplication::palette().color(QPalette::Inactive, QPalette::WindowText));
}

void lingmoThemeProxy_test::test_base()
{
    QCOMPARE(Theme::instance()->base(Theme::Active), QGuiApplication::palette().color(QPalette::Active, QPalette::Base));
    QCOMPARE(Theme::instance()->base(Theme::Disabled), QGuiApplication::palette().color(QPalette::Disabled, QPalette::Base));
    QCOMPARE(Theme::instance()->base(Theme::Inactive), QGuiApplication::palette().color(QPalette::Inactive, QPalette::Base));
}

void lingmoThemeProxy_test::test_text()
{
    QCOMPARE(Theme::instance()->text(Theme::Active), QGuiApplication::palette().color(QPalette::Active, QPalette::Text));
    QCOMPARE(Theme::instance()->text(Theme::Disabled), QGuiApplication::palette().color(QPalette::Disabled, QPalette::Text));
    QCOMPARE(Theme::instance()->text(Theme::Inactive), QGuiApplication::palette().color(QPalette::Inactive, QPalette::Text));
}

void lingmoThemeProxy_test::test_alternateBase()
{
    QCOMPARE(Theme::instance()->alternateBase(Theme::Active), QGuiApplication::palette().color(QPalette::Active, QPalette::AlternateBase));
    QCOMPARE(Theme::instance()->alternateBase(Theme::Disabled), QGuiApplication::palette().color(QPalette::Disabled, QPalette::AlternateBase));
    QCOMPARE(Theme::instance()->alternateBase(Theme::Inactive), QGuiApplication::palette().color(QPalette::Inactive, QPalette::AlternateBase));
}

void lingmoThemeProxy_test::test_button()
{
    QCOMPARE(Theme::instance()->button(Theme::Active), QGuiApplication::palette().color(QPalette::Active, QPalette::Button));
    QCOMPARE(Theme::instance()->button(Theme::Disabled), QGuiApplication::palette().color(QPalette::Disabled, QPalette::Button));
    QCOMPARE(Theme::instance()->button(Theme::Inactive), QGuiApplication::palette().color(QPalette::Inactive, QPalette::Button));
}

void lingmoThemeProxy_test::test_buttonText()
{
    QCOMPARE(Theme::instance()->buttonText(Theme::Active), QGuiApplication::palette().color(QPalette::Active, QPalette::ButtonText));
    QCOMPARE(Theme::instance()->buttonText(Theme::Disabled), QGuiApplication::palette().color(QPalette::Disabled, QPalette::ButtonText));
    QCOMPARE(Theme::instance()->buttonText(Theme::Inactive), QGuiApplication::palette().color(QPalette::Inactive, QPalette::ButtonText));
}

void lingmoThemeProxy_test::test_light()
{
    QCOMPARE(Theme::instance()->light(Theme::Active), QGuiApplication::palette().color(QPalette::Active, QPalette::Light));
    QCOMPARE(Theme::instance()->light(Theme::Disabled), QGuiApplication::palette().color(QPalette::Disabled, QPalette::Light));
    QCOMPARE(Theme::instance()->light(Theme::Inactive), QGuiApplication::palette().color(QPalette::Inactive, QPalette::Light));
}

void lingmoThemeProxy_test::test_midLight()
{
    QCOMPARE(Theme::instance()->midLight(Theme::Active), QGuiApplication::palette().color(QPalette::Active, QPalette::Midlight));
    QCOMPARE(Theme::instance()->midLight(Theme::Disabled), QGuiApplication::palette().color(QPalette::Disabled, QPalette::Midlight));
    QCOMPARE(Theme::instance()->midLight(Theme::Inactive), QGuiApplication::palette().color(QPalette::Inactive, QPalette::Midlight));
}

void lingmoThemeProxy_test::test_dark()
{
    QCOMPARE(Theme::instance()->dark(Theme::Active), QGuiApplication::palette().color(QPalette::Active, QPalette::Dark));
    QCOMPARE(Theme::instance()->dark(Theme::Disabled), QGuiApplication::palette().color(QPalette::Disabled, QPalette::Dark));
    QCOMPARE(Theme::instance()->dark(Theme::Inactive), QGuiApplication::palette().color(QPalette::Inactive, QPalette::Dark));
}

void lingmoThemeProxy_test::test_mid()
{
    QCOMPARE(Theme::instance()->mid(Theme::Active), QGuiApplication::palette().color(QPalette::Active, QPalette::Mid));
    QCOMPARE(Theme::instance()->mid(Theme::Disabled), QGuiApplication::palette().color(QPalette::Disabled, QPalette::Mid));
    QCOMPARE(Theme::instance()->mid(Theme::Inactive), QGuiApplication::palette().color(QPalette::Inactive, QPalette::Mid));
}

void lingmoThemeProxy_test::test_shadow()
{
    QCOMPARE(Theme::instance()->shadow(Theme::Active), QGuiApplication::palette().color(QPalette::Active, QPalette::Shadow));
    QCOMPARE(Theme::instance()->shadow(Theme::Disabled), QGuiApplication::palette().color(QPalette::Disabled, QPalette::Shadow));
    QCOMPARE(Theme::instance()->shadow(Theme::Inactive), QGuiApplication::palette().color(QPalette::Inactive, QPalette::Shadow));
}

void lingmoThemeProxy_test::test_highlight()
{
    QCOMPARE(Theme::instance()->highlight(Theme::Active), QGuiApplication::palette().color(QPalette::Active, QPalette::Highlight));
    QCOMPARE(Theme::instance()->highlight(Theme::Disabled), QGuiApplication::palette().color(QPalette::Disabled, QPalette::Highlight));
    QCOMPARE(Theme::instance()->highlight(Theme::Inactive), QGuiApplication::palette().color(QPalette::Inactive, QPalette::Highlight));
}

void lingmoThemeProxy_test::test_highlightedText()
{
    QCOMPARE(Theme::instance()->highlightedText(Theme::Active), QGuiApplication::palette().color(QPalette::Active, QPalette::HighlightedText));
    QCOMPARE(Theme::instance()->highlightedText(Theme::Disabled), QGuiApplication::palette().color(QPalette::Disabled, QPalette::HighlightedText));
    QCOMPARE(Theme::instance()->highlightedText(Theme::Inactive), QGuiApplication::palette().color(QPalette::Inactive, QPalette::HighlightedText));
}

void lingmoThemeProxy_test::test_separator()
{
    QCOMPARE(Theme::instance()->separator(Theme::Active), QGuiApplication::palette().color(QPalette::Active, QPalette::Window));
    QCOMPARE(Theme::instance()->separator(Theme::Disabled), QGuiApplication::palette().color(QPalette::Disabled, QPalette::Window));
    QCOMPARE(Theme::instance()->separator(Theme::Inactive), QGuiApplication::palette().color(QPalette::Inactive, QPalette::Window));
}

void lingmoThemeProxy_test::test_brightText()
{
    QCOMPARE(Theme::instance()->brightText(Theme::Active), QGuiApplication::palette().color(QPalette::Active, QPalette::BrightText));
    QCOMPARE(Theme::instance()->brightText(Theme::Disabled), QGuiApplication::palette().color(QPalette::Disabled, QPalette::BrightText));
    QCOMPARE(Theme::instance()->brightText(Theme::Inactive), QGuiApplication::palette().color(QPalette::Inactive, QPalette::BrightText));
}

void lingmoThemeProxy_test::test_link()
{
    QCOMPARE(Theme::instance()->link(Theme::Active), QGuiApplication::palette().color(QPalette::Active, QPalette::Link));
    QCOMPARE(Theme::instance()->link(Theme::Disabled), QGuiApplication::palette().color(QPalette::Disabled, QPalette::Link));
    QCOMPARE(Theme::instance()->link(Theme::Inactive), QGuiApplication::palette().color(QPalette::Inactive, QPalette::Link));
}

void lingmoThemeProxy_test::test_linkVisited()
{
    QCOMPARE(Theme::instance()->linkVisited(Theme::Active), QGuiApplication::palette().color(QPalette::Active, QPalette::LinkVisited));
    QCOMPARE(Theme::instance()->linkVisited(Theme::Disabled), QGuiApplication::palette().color(QPalette::Disabled, QPalette::LinkVisited));
    QCOMPARE(Theme::instance()->linkVisited(Theme::Inactive), QGuiApplication::palette().color(QPalette::Inactive, QPalette::LinkVisited));
}

void lingmoThemeProxy_test::test_toolTipBase()
{
    QCOMPARE(Theme::instance()->toolTipBase(Theme::Active), QGuiApplication::palette().color(QPalette::Active, QPalette::ToolTipBase));
    QCOMPARE(Theme::instance()->toolTipBase(Theme::Disabled), QGuiApplication::palette().color(QPalette::Disabled, QPalette::ToolTipBase));
    QCOMPARE(Theme::instance()->toolTipBase(Theme::Inactive), QGuiApplication::palette().color(QPalette::Inactive, QPalette::ToolTipBase));
}

void lingmoThemeProxy_test::test_toolTipText()
{
    QCOMPARE(Theme::instance()->toolTipText(Theme::Active), QGuiApplication::palette().color(QPalette::Active, QPalette::ToolTipText));
    QCOMPARE(Theme::instance()->toolTipText(Theme::Disabled), QGuiApplication::palette().color(QPalette::Disabled, QPalette::ToolTipText));
    QCOMPARE(Theme::instance()->toolTipText(Theme::Inactive), QGuiApplication::palette().color(QPalette::Inactive, QPalette::ToolTipText));
}

void lingmoThemeProxy_test::test_placeholderText()
{
    QCOMPARE(Theme::instance()->placeholderText(Theme::Active), QGuiApplication::palette().color(QPalette::Active, QPalette::PlaceholderText));
    QCOMPARE(Theme::instance()->placeholderText(Theme::Disabled), QGuiApplication::palette().color(QPalette::Disabled, QPalette::PlaceholderText));
    QCOMPARE(Theme::instance()->placeholderText(Theme::Inactive), QGuiApplication::palette().color(QPalette::Inactive, QPalette::PlaceholderText));
}

void lingmoThemeProxy_test::test_colorWithThemeTransparency()
{
    QFETCH(Theme::ColorRole, ColorRole);
    QFETCH(Theme::ColorGroup, ColorGroup);
    QFETCH(QColor, result);

    QCOMPARE(Theme::instance()->colorWithThemeTransparency(ColorRole, ColorGroup), result);
}

void lingmoThemeProxy_test::test_colorWithThemeTransparency_data()
{
    auto settings = new QGSettings(CONTROL_CENTER_SETTING, QByteArray(), Theme::instance());
    auto themeTransparency = settings->get(CONTROL_CENTER_TRANSPARENCY_KEY).toReal();
    QTest::addColumn<Theme::ColorRole>("ColorRole");
    QTest::addColumn<Theme::ColorGroup>("ColorGroup");
    QTest::addColumn<QColor>("result");

    QColor colortest = QGuiApplication::palette().color(QPalette::Active, QPalette::Window);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test0ActiveWindow")<<Theme::Window<<Theme::Active<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Disabled, QPalette::Window);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test1DisabledWindow")<<Theme::Window<<Theme::Disabled<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Inactive, QPalette::Window);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test2InactiveWindow")<<Theme::Window<<Theme::Inactive<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Active, QPalette::WindowText);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test3ActiveWindowText")<<Theme::WindowText<<Theme::Active<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Disabled, QPalette::WindowText);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test4DisabledWindowText")<<Theme::WindowText<<Theme::Disabled<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Inactive, QPalette::WindowText);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test5InactiveWindowText")<<Theme::WindowText<<Theme::Inactive<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Active, QPalette::Text);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test6ActiveText")<<Theme::Text<<Theme::Active<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Disabled, QPalette::Text);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test7DisabledText")<<Theme::Text<<Theme::Disabled<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Inactive, QPalette::Text);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test8InactiveText")<<Theme::Text<<Theme::Inactive<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Active, QPalette::Base);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test9ActiveBase")<<Theme::Base<<Theme::Active<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Disabled, QPalette::Base);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test10DisabledBase")<<Theme::Base<<Theme::Disabled<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Inactive, QPalette::Base);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test11InactiveBase")<<Theme::Base<<Theme::Inactive<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Active, QPalette::Button);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test12ActiveButton")<<Theme::Button<<Theme::Active<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Disabled, QPalette::Button);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test13DisabledButton")<<Theme::Button<<Theme::Disabled<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Inactive, QPalette::Button);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test14InactiveButton")<<Theme::Button<<Theme::Inactive<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Active, QPalette::ButtonText);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test15ActiveButtonText")<<Theme::ButtonText<<Theme::Active<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Disabled, QPalette::ButtonText);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test1Disabled6ButtonText")<<Theme::ButtonText<<Theme::Disabled<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Inactive, QPalette::ButtonText);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test17InactiveButtonText")<<Theme::ButtonText<<Theme::Inactive<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Active, QPalette::Highlight);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test18ActiveHighlight")<<Theme::Highlight<<Theme::Active<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Disabled, QPalette::Highlight);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test19DisabledHighlight")<<Theme::Highlight<<Theme::Disabled<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Inactive, QPalette::Highlight);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test20InactiveHighlight")<<Theme::Highlight<<Theme::Inactive<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Active, QPalette::HighlightedText);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test21ActiveHighlightedText")<<Theme::HighlightedText<<Theme::Active<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Disabled, QPalette::HighlightedText);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test22DisabledHighlightedText")<<Theme::HighlightedText<<Theme::Disabled<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Inactive, QPalette::HighlightedText);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test23InactiveHighlightedText")<<Theme::HighlightedText<<Theme::Inactive<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Active, QPalette::ToolTipBase);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test24ActiveToolTipBase")<<Theme::ToolTipBase<<Theme::Active<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Disabled, QPalette::ToolTipBase);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test25DisabledToolTipBase")<<Theme::ToolTipBase<<Theme::Disabled<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Inactive, QPalette::ToolTipBase);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test26InactiveToolTipBase")<<Theme::ToolTipBase<<Theme::Inactive<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Active, QPalette::ToolTipText);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test27ActiveToolTipText")<<Theme::ToolTipText<<Theme::Active<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Disabled, QPalette::ToolTipText);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test28DisabledToolTipText")<<Theme::ToolTipText<<Theme::Disabled<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Inactive, QPalette::ToolTipText);
    colortest.setAlphaF(themeTransparency);
    QTest::newRow("test29InactiveToolTipText")<<Theme::ToolTipText<<Theme::Inactive<<colortest;
}

void lingmoThemeProxy_test::test_colorWithCustomTransparency()
{
    QFETCH(Theme::ColorRole, ColorRole);
    QFETCH(Theme::ColorGroup, ColorGroup);
    QFETCH(qreal, alphaF);
    QFETCH(QColor, result);

    QCOMPARE(Theme::instance()->colorWithCustomTransparency(ColorRole, ColorGroup, alphaF), result);
}

void lingmoThemeProxy_test::test_colorWithCustomTransparency_data()
{
    QTest::addColumn<Theme::ColorRole>("ColorRole");
    QTest::addColumn<Theme::ColorGroup>("ColorGroup");
    QTest::addColumn<qreal>("alphaF");
    QTest::addColumn<QColor>("result");

    qreal alphaF = 0.15;
    QColor colortest = QGuiApplication::palette().color(QPalette::Active, QPalette::Window);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test0ActiveWindow")<<Theme::Window<<Theme::Active<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Disabled, QPalette::Window);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test1DisabledWindow")<<Theme::Window<<Theme::Disabled<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Inactive, QPalette::Window);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test2InactiveWindow")<<Theme::Window<<Theme::Inactive<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Active, QPalette::WindowText);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test3ActiveWindowText")<<Theme::WindowText<<Theme::Active<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Disabled, QPalette::WindowText);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test4DisabledWindowText")<<Theme::WindowText<<Theme::Disabled<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Inactive, QPalette::WindowText);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test5InactiveWindowText")<<Theme::WindowText<<Theme::Inactive<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Active, QPalette::Text);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test6ActiveText")<<Theme::Text<<Theme::Active<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Disabled, QPalette::Text);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test7DisabledText")<<Theme::Text<<Theme::Disabled<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Inactive, QPalette::Text);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test8InactiveText")<<Theme::Text<<Theme::Inactive<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Active, QPalette::Base);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test9ActiveBase")<<Theme::Base<<Theme::Active<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Disabled, QPalette::Base);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test10DisabledBase")<<Theme::Base<<Theme::Disabled<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Inactive, QPalette::Base);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test11InactiveBase")<<Theme::Base<<Theme::Inactive<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Active, QPalette::Button);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test12ActiveButton")<<Theme::Button<<Theme::Active<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Disabled, QPalette::Button);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test13DisabledButton")<<Theme::Button<<Theme::Disabled<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Inactive, QPalette::Button);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test14InactiveButton")<<Theme::Button<<Theme::Inactive<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Active, QPalette::ButtonText);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test15ActiveButtonText")<<Theme::ButtonText<<Theme::Active<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Disabled, QPalette::ButtonText);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test1Disabled6ButtonText")<<Theme::ButtonText<<Theme::Disabled<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Inactive, QPalette::ButtonText);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test17InactiveButtonText")<<Theme::ButtonText<<Theme::Inactive<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Active, QPalette::Highlight);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test18ActiveHighlight")<<Theme::Highlight<<Theme::Active<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Disabled, QPalette::Highlight);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test19DisabledHighlight")<<Theme::Highlight<<Theme::Disabled<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Inactive, QPalette::Highlight);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test20InactiveHighlight")<<Theme::Highlight<<Theme::Inactive<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Active, QPalette::HighlightedText);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test21ActiveHighlightedText")<<Theme::HighlightedText<<Theme::Active<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Disabled, QPalette::HighlightedText);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test22DisabledHighlightedText")<<Theme::HighlightedText<<Theme::Disabled<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Inactive, QPalette::HighlightedText);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test23InactiveHighlightedText")<<Theme::HighlightedText<<Theme::Inactive<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Active, QPalette::ToolTipBase);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test24ActiveToolTipBase")<<Theme::ToolTipBase<<Theme::Active<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Disabled, QPalette::ToolTipBase);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test25DisabledToolTipBase")<<Theme::ToolTipBase<<Theme::Disabled<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Inactive, QPalette::ToolTipBase);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test26InactiveToolTipBase")<<Theme::ToolTipBase<<Theme::Inactive<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Active, QPalette::ToolTipText);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test27ActiveToolTipText")<<Theme::ToolTipText<<Theme::Active<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Disabled, QPalette::ToolTipText);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test28DisabledToolTipText")<<Theme::ToolTipText<<Theme::Disabled<<alphaF<<colortest;

    colortest = QGuiApplication::palette().color(QPalette::Inactive, QPalette::ToolTipText);
    colortest.setAlphaF(alphaF);
    QTest::newRow("test29InactiveToolTipText")<<Theme::ToolTipText<<Theme::Inactive<<alphaF<<colortest;
}

QTEST_MAIN(lingmoThemeProxy_test)
#include "test-theme.moc"
