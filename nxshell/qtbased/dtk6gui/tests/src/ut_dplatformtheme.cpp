// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <QWindow>
#include <QTest>

#include "dguiapplicationhelper.h"
#define private public
#include "dplatformtheme.h"
#include "dplatformtheme_p.h"
#undef private

DGUI_USE_NAMESPACE

class TDPlatformTheme : public testing::Test
{
protected:
    void SetUp();
    void TearDown();

    QWidget *widget;
    DPlatformTheme *theme;
    DPlatformThemePrivate *theme_d;
};

void TDPlatformTheme::SetUp()
{
    widget = new QWidget;
    widget->createWinId();
    //widget->show();
    ASSERT_TRUE(widget->windowHandle());

    theme = new DPlatformTheme(quint32(widget->windowHandle()->winId()), widget);
    theme_d = theme->d_func();
}

void TDPlatformTheme::TearDown()
{
    delete widget;
}

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
TEST_F(TDPlatformTheme, testFunction)
{
    ASSERT_TRUE(theme_d->theme);
    ASSERT_EQ(theme->parentTheme(), theme_d->parent);
    ASSERT_TRUE(theme->isValid());

    DPalette tPalette = theme->palette();
    ASSERT_EQ(theme->fetchPalette(tPalette), tPalette);
}
#endif

#define TEST_THEME_NAME(TYPE) \
    QByteArrayLiteral("Test_Name_About_").append(TYPE)

#define ASSERT_EQ_BY_VALUE(SetFunc,Func,Param) \
    theme->SetFunc(Param); \
    ASSERT_EQ(theme->Func(),Param); \

#define ASSERT_EQ_COLOR(SetFunc,Func,Param) \
    ASSERT_EQ_BY_VALUE(SetFunc,Func,Param); \

#define ASSERT_EQ_BY_NAME(SetFunc,Func,Param) \
    theme->SetFunc(TEST_THEME_NAME(Param)); \
    ASSERT_EQ(theme->Func(),TEST_THEME_NAME(Param));

TEST_F(TDPlatformTheme, testSetFunction)
{
    if (qgetenv("QT_QPA_PLATFORM").contains("offscreen"))
        return;

    if (!theme->isValid()) {
        qDebug() << "PlatformTheme is not valid";
        return;
    }

    enum { TEST_COLOR = Qt::blue, TEST_DATA = 50 };

    ASSERT_EQ_BY_VALUE(setCursorBlinkTime, cursorBlinkTime, TEST_DATA);
    ASSERT_EQ_BY_VALUE(setCursorBlinkTimeout, cursorBlinkTimeout, TEST_DATA);
    ASSERT_EQ_BY_VALUE(setCursorBlink, cursorBlink, true)
    ASSERT_EQ_BY_VALUE(setDoubleClickDistance, doubleClickDistance, TEST_DATA);
    ASSERT_EQ_BY_VALUE(setDoubleClickTime, doubleClickTime, TEST_DATA)
    ASSERT_EQ_BY_VALUE(setDndDragThreshold, dndDragThreshold, TEST_DATA);
    ASSERT_EQ_BY_NAME(setThemeName, themeName, "Theme");
    ASSERT_EQ_BY_NAME(setIconThemeName, iconThemeName, "Icon");
    ASSERT_EQ_BY_NAME(setSoundThemeName, soundThemeName, "Sound");
    ASSERT_EQ_BY_NAME(setFontName, fontName, "Font");
    ASSERT_EQ_BY_NAME(setMonoFontName, monoFontName, "MonoFont");
    ASSERT_EQ_BY_NAME(setGtkFontName, gtkFontName, "GtkFont");
    ASSERT_EQ_BY_VALUE(setFontPointSize, fontPointSize, TEST_DATA);
    ASSERT_EQ_BY_VALUE(setActiveColor, activeColor, TEST_COLOR);
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    ASSERT_EQ_COLOR(setWindow, window, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setWindowText, windowText, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setBase, base, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setAlternateBase, alternateBase, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setToolTipBase, toolTipBase, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setToolTipText, toolTipText, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setText, text, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setButton, button, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setButtonText, buttonText, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setBrightText, brightText, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setLight, light, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setMidlight, midlight, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setDark, dark, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setMid, mid, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setShadow, shadow, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setHighlight, highlight, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setLink, link, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setLinkVisited, linkVisited, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setItemBackground, itemBackground, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setTextTitle, textTitle, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setTextTips, textTips, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setTextWarning, textWarning, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setTextLively, textLively, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setLightLively, lightLively, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setDarkLively, darkLively, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setFrameBorder, frameBorder, TEST_COLOR);
    ASSERT_EQ_BY_VALUE(setWindowRadius, windowRadius, TEST_DATA / 10);
#endif
}
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
TEST_F(TDPlatformTheme, palette)
{
    DPalette pa = theme->palette();
    ASSERT_EQ(theme->window(), pa.window().color());
    ASSERT_EQ(theme->windowText(), pa.windowText().color());
    ASSERT_EQ(theme->base(), pa.base().color());
    ASSERT_EQ(theme->alternateBase(), pa.alternateBase().color());
    ASSERT_EQ(theme->toolTipBase(), pa.toolTipBase().color());
    ASSERT_EQ(theme->toolTipText(), pa.toolTipText().color());
    ASSERT_EQ(theme->text(), pa.text().color());
    ASSERT_EQ(theme->button(), pa.button().color());
    ASSERT_EQ(theme->buttonText(), pa.buttonText().color());
    ASSERT_EQ(theme->brightText(), pa.brightText().color());
    ASSERT_EQ(theme->light(), pa.light().color());
    ASSERT_EQ(theme->midlight(), pa.midlight().color());
    ASSERT_EQ(theme->dark(), pa.dark().color());
    ASSERT_EQ(theme->mid(), pa.mid().color());
    ASSERT_EQ(theme->shadow(), pa.shadow().color());
    ASSERT_EQ(theme->highlight(), pa.highlight().color());
    ASSERT_EQ(theme->highlightedText(), pa.highlightedText().color());
    ASSERT_EQ(theme->link(), pa.link().color());
    ASSERT_EQ(theme->linkVisited(), pa.linkVisited().color());
    ASSERT_EQ(theme->itemBackground(), pa.itemBackground().color());
    ASSERT_EQ(theme->textTitle(), pa.textTitle().color());
    ASSERT_EQ(theme->textTips(), pa.textTips().color());
    ASSERT_EQ(theme->textWarning(), pa.textWarning().color());
    ASSERT_EQ(theme->textLively(), pa.textLively().color());
    ASSERT_EQ(theme->lightLively(), pa.lightLively().color());
    ASSERT_EQ(theme->darkLively(), pa.darkLively().color());
    ASSERT_EQ(theme->frameBorder(), pa.frameBorder().color());
}
#endif
