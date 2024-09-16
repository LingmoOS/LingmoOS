// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "propertygroup.h"

#include <QWidget>

#include <gtest/gtest.h>

class UT_PropertyGroup : public testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    PropertyGroup *m_propertyGroup;
};

void UT_PropertyGroup::SetUp()
{
    m_propertyGroup = new PropertyGroup();
}

void UT_PropertyGroup::TearDown()
{
    delete m_propertyGroup;
}

TEST_F(UT_PropertyGroup, BasicTest)
{
    QWidget widget;
    m_propertyGroup->addObject(&widget);
    m_propertyGroup->removeObject(&widget);
    m_propertyGroup->addProperty("test");
    m_propertyGroup->removeProperty("test");
}
