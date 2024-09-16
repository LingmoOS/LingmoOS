// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_memorypublic.h"

#include "../../src/memorypublic.h"

Ut_MemoryPublic::Ut_MemoryPublic()
{

}

TEST_F(Ut_MemoryPublic, getwidget)
{
    MemoryPublic *m_memoryPublic = MemoryPublic::instance();
    MemoryWidget *memorywidget = m_memoryPublic->getwidget(MemoryPublic::memorymode::scientificright);
    EXPECT_EQ(memorywidget->m_calculatormode, 1);
}

TEST_F(Ut_MemoryPublic, memoryplus)
{
    MemoryPublic *m_memoryPublic = MemoryPublic::instance();
    MemoryWidget *memorywidget = m_memoryPublic->getwidget(MemoryPublic::memorymode::scientificright);
    m_memoryPublic->memoryplus(Quantity(0));
    EXPECT_EQ(memorywidget->m_listwidget->count(), 1);
}

TEST_F(Ut_MemoryPublic, memoryminus)
{
    MemoryPublic *m_memoryPublic = MemoryPublic::instance();
    MemoryWidget *memorywidget = m_memoryPublic->getwidget(MemoryPublic::memorymode::scientificright);
    m_memoryPublic->memoryminus(Quantity(0));
    EXPECT_EQ(memorywidget->m_listwidget->count(), 1);
}

TEST_F(Ut_MemoryPublic, memoryclean)
{
    MemoryPublic *m_memoryPublic = MemoryPublic::instance();
    MemoryWidget *memorywidget = m_memoryPublic->getwidget(MemoryPublic::memorymode::scientificright);
    m_memoryPublic->memoryclean();
    m_memoryPublic->generateData(Quantity(0));
    m_memoryPublic->generateData(Quantity(0));
    m_memoryPublic->generateData(Quantity(0));
    EXPECT_EQ(m_memoryPublic->m_list.count(), 3);
    m_memoryPublic->memoryclean();
    EXPECT_EQ(memorywidget->m_listwidget->count(), 1);
}

TEST_F(Ut_MemoryPublic, widgetplus)
{
    MemoryPublic *m_memoryPublic = MemoryPublic::instance();
    MemoryWidget *memorywidget = m_memoryPublic->getwidget(MemoryPublic::memorymode::scientificright);
    m_memoryPublic->generateData(Quantity(1));
    m_memoryPublic->widgetplus(0, Quantity(1));
    MemoryItemWidget *w1 = static_cast<MemoryItemWidget *>(memorywidget->m_listwidget->itemWidget(memorywidget->m_listwidget->item(0)));
    EXPECT_EQ(w1->textLabel(), "2");
}

TEST_F(Ut_MemoryPublic, widgetminus)
{
    MemoryPublic *m_memoryPublic = MemoryPublic::instance();
    MemoryWidget *memorywidget = m_memoryPublic->getwidget(MemoryPublic::memorymode::scientificright);
    m_memoryPublic->generateData(Quantity(1));
    m_memoryPublic->widgetminus(0, Quantity(1));
    MemoryItemWidget *w1 = static_cast<MemoryItemWidget *>(memorywidget->m_listwidget->itemWidget(memorywidget->m_listwidget->item(0)));
    EXPECT_EQ(w1->textLabel(), "0");
}

TEST_F(Ut_MemoryPublic, widgetclean)
{
    MemoryPublic *m_memoryPublic = MemoryPublic::instance();
    MemoryWidget *memorywidget = m_memoryPublic->getwidget(MemoryPublic::memorymode::scientificright);
    memorywidget->m_calculatormode = 0;
    m_memoryPublic->memoryclean();
    m_memoryPublic->generateData(Quantity(0));
    m_memoryPublic->generateData(Quantity(0));
    m_memoryPublic->generateData(Quantity(0));
    EXPECT_EQ(m_memoryPublic->m_list.count(), 3);
    m_memoryPublic->widgetclean(1, 1, false);
    EXPECT_EQ(memorywidget->m_listwidget->count(), 2);
}

TEST_F(Ut_MemoryPublic, setThemeType)
{
    MemoryPublic *m_memoryPublic = MemoryPublic::instance();
    MemoryWidget *memorywidget = m_memoryPublic->getwidget(MemoryPublic::memorymode::scientificright);
    m_memoryPublic->m_list.clear();
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::LightType);
    m_memoryPublic->setThemeType(0);
    EXPECT_EQ(memorywidget->m_clearbutton->m_currentUrl, ":/assets/images/light/empty_normal.svg");
    DGuiApplicationHelper::instance()->setThemeType(DGuiApplicationHelper::ColorType::DarkType);
    m_memoryPublic->setThemeType(0);
    EXPECT_EQ(memorywidget->m_clearbutton->m_currentUrl, ":/assets/images/dark/empty_normal.svg");
}
