// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>

#include "test_helper.hpp"
#include <QQuickItem>

#include <DGuiApplicationHelper>
DGUI_USE_NAMESPACE

class ut_ThemeMenu : public ::testing::Test
{
public:
};

TEST_F(ut_ThemeMenu, checkedWithThemeType)
{
    ControlHelper<> root("qrc:/qml/CustomThemeMenu.qml");
    ASSERT_TRUE(root.object);

    QQuickItem *action = nullptr;
    {
        ThemeTypeGuard themeGurad(DGuiApplicationHelper::LightType);
        Q_UNUSED(themeGurad);
        QMetaObject::invokeMethod(root.object, "itemAt", Q_RETURN_ARG(QQuickItem*, action), Q_ARG(int, 0));
        ASSERT_TRUE(action);
        ASSERT_EQ(action->property("checked").toBool(), true);
    }

    {
        ThemeTypeGuard themeGurad(DGuiApplicationHelper::UnknownType);
        Q_UNUSED(themeGurad);
        QMetaObject::invokeMethod(root.object, "itemAt", Q_RETURN_ARG(QQuickItem*, action), Q_ARG(int, 2));
        ASSERT_EQ(action->property("checked").toBool(), true);
    }
}
