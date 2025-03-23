// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_helper.hpp"

#include <gtest/gtest.h>

#include <QPointer>
#include <QPalette>
#include <QSignalSpy>

#include <DPalette>
#include <DFontManager>
#include <DWindowManagerHelper>
#include <DGuiApplicationHelper>

#include <private/dqmlglobalobject_p.h>
#include "dmessagemanager_p.h"

DQUICK_USE_NAMESPACE
DGUI_USE_NAMESPACE

class ut_DQMLGlobalObject : public ::testing::Test
{
public:
    virtual void SetUp();
    virtual void TearDown();

    QPointer<DQMLGlobalObject> pObj;
    QPointer<DWindowManagerHelper> pWMIns;
    QPointer<DGuiApplicationHelper> pAppIns;
};

void ut_DQMLGlobalObject::SetUp()
{
    pObj = new DQMLGlobalObject();
    pWMIns = DWindowManagerHelper::instance();
    pAppIns = DGuiApplicationHelper::instance();
}

void ut_DQMLGlobalObject::TearDown()
{
    if (!pObj.isNull())
        delete pObj.data();
}

TEST_F(ut_DQMLGlobalObject, hasBlurWindow)
{
    ASSERT_EQ(pWMIns->hasBlurWindow(), pObj->hasBlurWindow());
}

TEST_F(ut_DQMLGlobalObject, hasComposite)
{
    ASSERT_EQ(pWMIns->hasComposite(), pObj->hasComposite());
}

TEST_F(ut_DQMLGlobalObject, hasNoTitlebar)
{
    ASSERT_EQ(pWMIns->hasNoTitlebar(), pObj->hasNoTitlebar());
}

TEST_F(ut_DQMLGlobalObject, windowManagerName)
{
    ASSERT_EQ(pWMIns->windowManagerName(), pObj->windowManagerName());
}

TEST_F(ut_DQMLGlobalObject, themeType)
{
    ASSERT_EQ(pAppIns->themeType(), pObj->themeType());
}

TEST_F(ut_DQMLGlobalObject, windowManagerNameString)
{
    ASSERT_EQ(pWMIns->windowManagerNameString(), pObj->windowManagerNameString());
}

TEST_F(ut_DQMLGlobalObject, platformTheme)
{
    ASSERT_TRUE(pObj->platformTheme());
}

TEST_F(ut_DQMLGlobalObject, fontManager)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    qmlRegisterType<DFontManager>();
#else
    qmlRegisterAnonymousType<DFontManager>("", 1);
#endif
    ASSERT_TRUE(pObj->fontManager());
}

TEST_F(ut_DQMLGlobalObject, palette)
{
    DPalette expected = pAppIns->applicationPalette();
    DPalette actual = pObj->palette();
    for (int i = 0; i < QPalette::NColorRoles; ++i) {
        QPalette::ColorRole role = static_cast<QPalette::ColorRole>(i);
        ASSERT_EQ(expected.color(QPalette::Active, role), actual.color(QPalette::Active, role));
    }
}

TEST_F(ut_DQMLGlobalObject, paletteChanged)
{
    Q_UNUSED(pObj->palette());

    DPalette palatte = pAppIns->applicationPalette();
    palatte.setColor(DPalette::TextTitle, Qt::red);
    QSignalSpy spy(pObj, &DQMLGlobalObject::paletteChanged);
    pAppIns->setApplicationPalette(palatte);
    EXPECT_EQ(spy.count(), 1);
    pAppIns->setApplicationPalette(DPalette());
}

TEST_F(ut_DQMLGlobalObject, inactivePalette)
{
    DPalette expected = pAppIns->applicationPalette();
    DPalette actual = pObj->inactivePalette();
    for (int i = 0; i < QPalette::NColorRoles; ++i) {
        QPalette::ColorRole role = static_cast<QPalette::ColorRole>(i);
        ASSERT_EQ(expected.color(QPalette::Inactive, role), actual.color(QPalette::Active, role));
    }
}

TEST_F(ut_DQMLGlobalObject, makeColor)
{
    ControlHelper<> helper("qrc:/qml/QmlGlobalObject.qml");
    ASSERT_TRUE(helper.object);

    DColor color1 = helper.object->property("color1").value<DColor>();
    ASSERT_TRUE(color1.isTypedColor());

    DColor color2 = helper.object->property("color2").value<DColor>();
    ASSERT_TRUE(!color2.isTypedColor());

    const QPalette &pt = helper.object->property("palette").value<QPalette>();
    ASSERT_EQ(color1.toColor(pt), pt.highlight().color());
}

TEST_F(ut_DQMLGlobalObject, deepinWebsite)
{
    ASSERT_TRUE(!pObj->deepinWebsiteName().isEmpty());
    ASSERT_TRUE(!pObj->deepinWebsiteLink().isEmpty());
}

TEST_F(ut_DQMLGlobalObject, deepinDistributionOrgLogo)
{
    ASSERT_TRUE(!pObj->deepinDistributionOrgLogo().isEmpty());
}

TEST_F(ut_DQMLGlobalObject, blendColor)
{
    QColor color = pObj->blendColor(Qt::red, Qt::blue);
    EXPECT_NE(color, QColor(Qt::red));
    EXPECT_EQ(color, QColor(Qt::blue));
}

TEST_F(ut_DQMLGlobalObject, toColorType)
{
    DGuiApplicationHelper::ColorType colorType = pObj->toColorType(pAppIns->applicationPalette().window().color());
    EXPECT_EQ(colorType, pObj->themeType());
}

TEST_F(ut_DQMLGlobalObject, selectColor)
{
    QColor color = pObj->selectColor(pAppIns->applicationPalette().window().color(), Qt::white, Qt::black);
    QColor expectColor = pObj->themeType() == DGuiApplicationHelper::DarkType ? QColor(Qt::black) : QColor(Qt::white);
    EXPECT_EQ(color, expectColor);
}

TEST_F(ut_DQMLGlobalObject, sendMessage)
{
    ControlHelper<> helper("qrc:/qml/QmlGlobalObject_MessageManager.qml");
    ASSERT_TRUE(helper.object);

    EXPECT_TRUE(pObj->sendMessage(helper.object, "msg1", "icon1", 1));
}

TEST_F(ut_DQMLGlobalObject, sendMessageByDelegate)
{
    ControlHelper<QQuickWindow> helper("qrc:/qml/QmlGlobalObject_MessageManager.qml");
    QQuickWindow *root = helper.object;
    MessageManager *msgMgr = qobject_cast<MessageManager *>(qmlAttachedPropertiesObject<MessageManager>(root, false));
    ASSERT_TRUE(msgMgr);
    QQmlComponent *delegate = root->property("containerCom").value<QQmlComponent *>();
    ASSERT_TRUE(delegate);
    QVariantMap argus;
    argus.insert("msg", QStringList{"msg1"});
    argus.insert("icon", QStringList{"icon1"});
    EXPECT_TRUE(pObj->sendMessage(root, delegate, argus, 1, "id1"));
    EXPECT_EQ(msgMgr->count(), 1);
    pObj->closeMessage(root, "id1");
    EXPECT_EQ(msgMgr->count(), 0);
}

TEST(ut_DColor, construct)
{
    DColor invalidColor;
    ASSERT_FALSE(invalidColor.isValid());

    DColor color(DColor::Highlight);
    ASSERT_TRUE(color.isTypedColor());
    ASSERT_TRUE(color.isValid());

    QPalette pt;
    pt.setBrush(QPalette::Highlight, Qt::red);

    ASSERT_EQ(color.toColor(pt), Qt::red);

    DColor color1(DColor::HighlightedText);
    ASSERT_TRUE(color1.isTypedColor());
    ASSERT_TRUE(color1.isValid());

    pt.setBrush(QPalette::HighlightedText, Qt::blue);
    ASSERT_EQ(color1.toColor(pt), Qt::blue);

    DColor color2(QColor(Qt::red));
    ASSERT_EQ(color2.color(), Qt::red);
}

TEST(ut_DColor, common)
{
    DColor color(QColor(Qt::red));
    DColor color2(color.hue(1).opacity(1).saturation(1).lightness(1));

    EXPECT_NE(color.data.hue, color2.data.hue);
    EXPECT_NE(color.data.opacity, color2.data.opacity);
    EXPECT_NE(color.data.saturation, color2.data.saturation);
    EXPECT_NE(color.data.lightness, color2.data.lightness);
}

TEST(ut_DColor, operatorEQ)
{
    DColor color1(QColor(Qt::red));
    DColor color2(QColor(Qt::blue));
    DColor color3(QColor(Qt::blue));
    EXPECT_NE(color1, color2);
    EXPECT_EQ(color2, color3);
}
