// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test.h"
#define private public
#include "dguiapplicationhelper_p.h"
#include "dguiapplicationhelper.h"
#undef private

#include <QMap>
#include <QProcess>

DGUI_BEGIN_NAMESPACE

class TDGuiApplicationHelper : public DTest
{
protected:
    virtual void SetUp()
    {
        helper = DGuiApplicationHelper::instance();
        helper_d = helper->d_func();

        /* read write */
        readWriteDatas << DGuiApplicationHelper::Attribute::UseInactiveColorGroup
                       << DGuiApplicationHelper::Attribute::ColorCompositing;

        /* read only */
        readOnlyDatas << DGuiApplicationHelper::Attribute::IsDeepinPlatformTheme
                      << DGuiApplicationHelper::Attribute::IsDXcbPlatform
                      << DGuiApplicationHelper::Attribute::IsXWindowPlatform
                      << DGuiApplicationHelper::Attribute::IsTableEnvironment
                      << DGuiApplicationHelper::Attribute::IsDeepinEnvironment;
    }

    DGuiApplicationHelper *helper = nullptr;
    DGuiApplicationHelperPrivate *helper_d = nullptr;
    QList<DGuiApplicationHelper::Attribute> readWriteDatas;
    QList<DGuiApplicationHelper::Attribute> readOnlyDatas;
};

TEST_F(TDGuiApplicationHelper, testFunction)
{
    QColor testColor(Qt::red);
    QColor adjustedColor = helper->adjustColor(testColor, 0, 0, 0, 0, 0, 0, -20);
    ASSERT_NE(testColor, adjustedColor);
    ASSERT_TRUE(adjustedColor.isValid());

    QColor disBlendColor  = Qt::black;
    QColor blendedColor = helper->blendColor(testColor, disBlendColor);
    ASSERT_NE(testColor, blendedColor);
    ASSERT_TRUE(blendedColor.isValid());

    DPalette tPalette;
    helper->generatePaletteColor(tPalette, QPalette::Window, DGuiApplicationHelper::ColorType::LightType);
    ASSERT_EQ(tPalette.brush(QPalette::Disabled, QPalette::Window), tPalette.brush(QPalette::Normal, QPalette::Window));

    tPalette.setColor(DPalette::Window, Qt::black);
    helper->generatePaletteColor(tPalette, QPalette::Highlight, DGuiApplicationHelper::ColorType::DarkType);
    ASSERT_TRUE(tPalette.highlight().color().isValid());

    // 初始化调色板为默认值
    helper->generatePalette(tPalette);

    ASSERT_EQ(helper->isXWindowPlatform(), DGuiApplicationHelper::testAttribute(DGuiApplicationHelper::IsXWindowPlatform));
    ASSERT_EQ(helper->isTabletEnvironment(), DGuiApplicationHelper::testAttribute(DGuiApplicationHelper::IsTableEnvironment));

    ASSERT_TRUE(helper->applicationTheme());
    ASSERT_TRUE(helper->systemTheme());
    ASSERT_NE(helper->applicationPalette(), tPalette);
    qGuiApp->setAttribute(Qt::AA_SetPalette, false);
    helper->setPaletteType(DGuiApplicationHelper::DarkType);
    ASSERT_NE(helper->applicationPalette(), tPalette);

    helper->setApplicationPalette(tPalette);
    ASSERT_EQ(helper->applicationPalette(), tPalette);

    ASSERT_TRUE(helper->fontManager());
    ASSERT_EQ(helper->toColorType(QColor(Qt::white)), DGuiApplicationHelper::LightType);
    ASSERT_EQ(helper->toColorType(QColor(Qt::black)), DGuiApplicationHelper::DarkType);
    ASSERT_EQ(helper->themeType(), DGuiApplicationHelper::DarkType);
    ASSERT_EQ(helper->paletteType(), DGuiApplicationHelper::DarkType);
}

TEST_F(TDGuiApplicationHelper, adjustColor_NoChange)
{
    QColor testColor(Qt::red);
    QColor adjustedColor = helper->adjustColor(testColor, 0, 0, 0, 0, 0, 0, 0);
    ASSERT_EQ(testColor, adjustedColor);

    testColor = QColor::fromHsl(100, 100, 100, 100);
    adjustedColor = helper->adjustColor(testColor, 0, 0, 0, 0, 0, 0, 0);
    ASSERT_EQ(testColor, adjustedColor);
}

TEST_F(TDGuiApplicationHelper, AttributeReadWrite)
{
    QMap<DGuiApplicationHelper::Attribute, bool> oldData;

    for (const DGuiApplicationHelper::Attribute attribute : readWriteDatas) {
        oldData[attribute] = helper->testAttribute(attribute);
    }

    for (const DGuiApplicationHelper::Attribute attribute : readWriteDatas) {
        helper->setAttribute(attribute, !oldData[attribute]);
        EXPECT_EQ(helper->testAttribute(attribute), !oldData[attribute]);
    }
}

TEST_F(TDGuiApplicationHelper, AttributeReadOnly)
{
    QMap<DGuiApplicationHelper::Attribute, bool> oldData;

    for (const DGuiApplicationHelper::Attribute attribute : readOnlyDatas) {
        oldData[attribute] = helper->testAttribute(attribute);
    }

    for (const DGuiApplicationHelper::Attribute attribute : readOnlyDatas) {
        helper->setAttribute(attribute, !oldData[attribute]);
        EXPECT_EQ(helper->testAttribute(attribute), oldData[attribute]);
    }
}

TEST_F(TDGuiApplicationHelper, loadTranslator)
{
    EXPECT_EQ(QProcess::tr("No program defined"), "No program defined");

    DGuiApplicationHelper::loadTranslator({QLocale("zh_CN")});

    EXPECT_EQ(QProcess::tr("No program defined"), "没有定义程序");

    qInfo() << QCoreApplication::translate("TDGuiApplicationHelper", "test-translation");
}

DGUI_END_NAMESPACE
