// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickItem>

class ut_ColorSelector : public ::testing::Test
{
public:
    virtual void SetUp();
    virtual void TearDown();
};

void ut_ColorSelector::SetUp()
{
}

void ut_ColorSelector::TearDown()
{
}

TEST_F(ut_ColorSelector, getColorFromProperty)
{
    ControlHelper<QQuickItem> helper("qrc:/qml/ColorSelector_GetColorFromProperty.qml");
    ASSERT_TRUE(helper.object);

    auto listView = helper.object->property("view").value<QQuickItem*>();
    ASSERT_NE(listView, nullptr);

    QObject *buttonPanel = qvariant_cast<QObject*>(listView->property("buttonPanel"));
    ASSERT_NE(buttonPanel, nullptr);

    QObject *background = buttonPanel->findChild<QQuickItem*>("background");
    ASSERT_NE(background, nullptr);

    QQmlEngine &engine = helper.engine;
    engine.globalObject().setProperty("background", engine.newQObject(background));

    ThemeTypeGuard themeGurad(DGuiApplicationHelper::LightType);
    Q_UNUSED(themeGurad);
    ASSERT_EQ(background->property("color"), engine.evaluate("background.color1.normal.common.color()").toVariant());
}
