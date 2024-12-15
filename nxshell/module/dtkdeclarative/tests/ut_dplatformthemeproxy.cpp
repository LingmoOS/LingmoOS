// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include <QPointer>

#include <DGuiApplicationHelper>
#include <DPlatformThemeProxy>

DQUICK_USE_NAMESPACE
DGUI_USE_NAMESPACE

class ut_DPlatformThemeProxy : public ::testing::Test
{
public:
    virtual void SetUp();
    virtual void TearDown();

    QPointer<DPlatformThemeProxy> pObj;
    QPointer<DPlatformTheme> pAppTheme;
};

void ut_DPlatformThemeProxy::SetUp()
{
    pAppTheme = DGuiApplicationHelper::instance()->applicationTheme();
    pObj = new DPlatformThemeProxy(pAppTheme);
}

void ut_DPlatformThemeProxy::TearDown()
{
    if (!pObj.isNull())
        delete pObj.data();
}

TEST_F(ut_DPlatformThemeProxy, cursorBlinkTime)
{
    ASSERT_EQ(pAppTheme->cursorBlinkTime(), pObj->cursorBlinkTime());
}

TEST_F(ut_DPlatformThemeProxy, cursorBlinkTimeout)
{
    ASSERT_EQ(pAppTheme->cursorBlinkTimeout(), pObj->cursorBlinkTimeout());
}

TEST_F(ut_DPlatformThemeProxy, cursorBlink)
{
    ASSERT_EQ(pAppTheme->cursorBlink(), pObj->cursorBlink());
}

TEST_F(ut_DPlatformThemeProxy, doubleClickDistance)
{
    ASSERT_EQ(pAppTheme->doubleClickDistance(), pObj->doubleClickDistance());
}

TEST_F(ut_DPlatformThemeProxy, doubleClickTime)
{
    ASSERT_EQ(pAppTheme->doubleClickTime(), pObj->doubleClickTime());
}

TEST_F(ut_DPlatformThemeProxy, dndDragThreshold)
{
    ASSERT_EQ(pAppTheme->dndDragThreshold(), pObj->dndDragThreshold());
}

TEST_F(ut_DPlatformThemeProxy, windowRadius)
{
    ASSERT_EQ(pAppTheme->windowRadius(), pObj->windowRadius());
}

TEST_F(ut_DPlatformThemeProxy, themeName)
{
    ASSERT_EQ(pAppTheme->themeName(), pObj->themeName());
}

TEST_F(ut_DPlatformThemeProxy, iconThemeName)
{
    ASSERT_EQ(pAppTheme->iconThemeName(), pObj->iconThemeName());
}

TEST_F(ut_DPlatformThemeProxy, soundThemeName)
{
    ASSERT_EQ(pAppTheme->soundThemeName(), pObj->soundThemeName());
}

TEST_F(ut_DPlatformThemeProxy, fontName)
{
    ASSERT_EQ(pAppTheme->fontName(), pObj->fontName());
}

TEST_F(ut_DPlatformThemeProxy, monoFontName)
{
    ASSERT_EQ(pAppTheme->monoFontName(), pObj->monoFontName());
}

TEST_F(ut_DPlatformThemeProxy, fontPointSize)
{
    ASSERT_EQ(pAppTheme->fontPointSize(), pObj->fontPointSize());
}

TEST_F(ut_DPlatformThemeProxy, gtkFontName)
{
    ASSERT_EQ(pAppTheme->gtkFontName(), pObj->gtkFontName());
}

TEST_F(ut_DPlatformThemeProxy, activeColor)
{
    ASSERT_EQ(pAppTheme->activeColor(), pObj->activeColor());
}

TEST_F(ut_DPlatformThemeProxy, window)
{
    ASSERT_EQ(pAppTheme->window(), pObj->window());
}

TEST_F(ut_DPlatformThemeProxy, windowText)
{
    ASSERT_EQ(pAppTheme->windowText(), pObj->windowText());
}

TEST_F(ut_DPlatformThemeProxy, base)
{
    ASSERT_EQ(pAppTheme->base(), pObj->base());
}

TEST_F(ut_DPlatformThemeProxy, alternateBase)
{
    ASSERT_EQ(pAppTheme->alternateBase(), pObj->alternateBase());
}

TEST_F(ut_DPlatformThemeProxy, toolTipBase)
{
    ASSERT_EQ(pAppTheme->toolTipBase(), pObj->toolTipBase());
}

TEST_F(ut_DPlatformThemeProxy, toolTipText)
{
    ASSERT_EQ(pAppTheme->toolTipText(), pObj->toolTipText());
}

TEST_F(ut_DPlatformThemeProxy, text)
{
    ASSERT_EQ(pAppTheme->text(), pObj->text());
}

TEST_F(ut_DPlatformThemeProxy, button)
{
    ASSERT_EQ(pAppTheme->button(), pObj->button());
}

TEST_F(ut_DPlatformThemeProxy, buttonText)
{
    ASSERT_EQ(pAppTheme->buttonText(), pObj->buttonText());
}

TEST_F(ut_DPlatformThemeProxy, brightText)
{
    ASSERT_EQ(pAppTheme->brightText(), pObj->brightText());
}

TEST_F(ut_DPlatformThemeProxy, light)
{
    ASSERT_EQ(pAppTheme->light(), pObj->light());
}

TEST_F(ut_DPlatformThemeProxy, midlight)
{
    ASSERT_EQ(pAppTheme->midlight(), pObj->midlight());
}

TEST_F(ut_DPlatformThemeProxy, dark)
{
    ASSERT_EQ(pAppTheme->dark(), pObj->dark());
}

TEST_F(ut_DPlatformThemeProxy, mid)
{
    ASSERT_EQ(pAppTheme->mid(), pObj->mid());
}

TEST_F(ut_DPlatformThemeProxy, shadow)
{
    ASSERT_EQ(pAppTheme->shadow(), pObj->shadow());
}

TEST_F(ut_DPlatformThemeProxy, highlight)
{
    ASSERT_EQ(pAppTheme->highlight(), pObj->highlight());
}

TEST_F(ut_DPlatformThemeProxy, highlightedText)
{
    ASSERT_EQ(pAppTheme->highlightedText(), pObj->highlightedText());
}

TEST_F(ut_DPlatformThemeProxy, link)
{
    ASSERT_EQ(pAppTheme->link(), pObj->link());
}

TEST_F(ut_DPlatformThemeProxy, linkVisited)
{
    ASSERT_EQ(pAppTheme->linkVisited(), pObj->linkVisited());
}

TEST_F(ut_DPlatformThemeProxy, itemBackground)
{
    ASSERT_EQ(pAppTheme->itemBackground(), pObj->itemBackground());
}

TEST_F(ut_DPlatformThemeProxy, textTitle)
{
    ASSERT_EQ(pAppTheme->textTitle(), pObj->textTitle());
}

TEST_F(ut_DPlatformThemeProxy, textTips)
{
    ASSERT_EQ(pAppTheme->textTips(), pObj->textTips());
}

TEST_F(ut_DPlatformThemeProxy, textWarning)
{
    ASSERT_EQ(pAppTheme->textWarning(), pObj->textWarning());
}

TEST_F(ut_DPlatformThemeProxy, textLively)
{
    ASSERT_EQ(pAppTheme->textLively(), pObj->textLively());
}

TEST_F(ut_DPlatformThemeProxy, lightLively)
{
    ASSERT_EQ(pAppTheme->lightLively(), pObj->lightLively());
}

TEST_F(ut_DPlatformThemeProxy, darkLively)
{
    ASSERT_EQ(pAppTheme->darkLively(), pObj->darkLively());
}

TEST_F(ut_DPlatformThemeProxy, frameBorder)
{
    ASSERT_EQ(pAppTheme->frameBorder(), pObj->frameBorder());
}
