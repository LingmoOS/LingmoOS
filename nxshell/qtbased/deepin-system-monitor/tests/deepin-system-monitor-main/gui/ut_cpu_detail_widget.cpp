// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "cpu_detail_widget.h"
#include "model/cpu_info_model.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <QResizeEvent>
#include <QPainter>
#include <QGridLayout>

/***************************************STUB begin*********************************************/
int stub_setMultiModeLayout_sysconf_cpu1()
{
    return 1;
}

int stub_setMultiModeLayout_sysconf_cpu2()
{
    return 2;
}

int stub_setMultiModeLayout_sysconf_cpu4()
{
    return 4;
}

int stub_setMultiModeLayout_sysconf_cpu8()
{
    return 8;
}

int stub_setMultiModeLayout_sysconf_cpu16()
{
    return 16;
}

int stub_setMultiModeLayout_sysconf_cpu32()
{
    return 32;
}

int stub_setMultiModeLayout_sysconf_cpu64()
{
    return 64;
}

int stub_setMultiModeLayout_sysconf_cpu0()
{
    return 0;
}
/***************************************STUB end**********************************************/

class UT_CPUDetailGrapTableItem : public ::testing::Test
{
public:
    UT_CPUDetailGrapTableItem() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static CPUInfoModel model;
        static QWidget widget;
        m_tester = new CPUDetailGrapTableItem(&model, 0, &widget);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    CPUDetailGrapTableItem *m_tester;
};

TEST_F(UT_CPUDetailGrapTableItem, initTest)
{

}

TEST_F(UT_CPUDetailGrapTableItem, test_setMode_01)
{
    m_tester->setMode(1);

    EXPECT_EQ(m_tester->m_mode, 1);
}

TEST_F(UT_CPUDetailGrapTableItem, test_setMultiCoreMode_01)
{
    m_tester->setMultiCoreMode(true);

    EXPECT_EQ(m_tester->m_isMutliCoreMode, true);
}

TEST_F(UT_CPUDetailGrapTableItem, test_sethorizontal_01)
{
    m_tester->sethorizontal(true);

    EXPECT_EQ(m_tester->m_isHorizontalLast, true);
}

TEST_F(UT_CPUDetailGrapTableItem, test_setVerticalLast_01)
{
    m_tester->setVerticalLast(true);

    EXPECT_EQ(m_tester->m_isVerticalLast, true);
}

TEST_F(UT_CPUDetailGrapTableItem, test_setColor_01)
{
    m_tester->setColor(QColor(Qt::red));

    EXPECT_EQ(m_tester->m_color, QColor(Qt::red));
}

TEST_F(UT_CPUDetailGrapTableItem, test_updateStat_01)
{
    m_tester->m_isMutliCoreMode = false;
    m_tester->updateStat();

    EXPECT_EQ(m_tester->m_cpuPercents.size(), 1);
}

TEST_F(UT_CPUDetailGrapTableItem, test_updateStat_02)
{
    m_tester->m_isMutliCoreMode = true;
    m_tester->updateStat();

    EXPECT_EQ(m_tester->m_cpuPercents.size(), 1);
}

TEST_F(UT_CPUDetailGrapTableItem, test_updateStat_03)
{
    for (int i = 0; i < 33; i++) {
        m_tester->m_cpuPercents.append(qreal(i));
    }
    m_tester->m_isMutliCoreMode = true;
    m_tester->updateStat();

    EXPECT_EQ(m_tester->m_cpuPercents.size(), 31);
}

TEST_F(UT_CPUDetailGrapTableItem, test_paintEvent_01)
{
    m_tester->m_mode = 1;
    EXPECT_FALSE(m_tester->grab().isNull());
}

TEST_F(UT_CPUDetailGrapTableItem, test_paintEvent_02)
{
    m_tester->m_mode = 2;
    EXPECT_FALSE(m_tester->grab().isNull());
}

TEST_F(UT_CPUDetailGrapTableItem, test_paintEvent_03)
{
    m_tester->m_mode = 3;
    EXPECT_FALSE(m_tester->grab().isNull());
}

TEST_F(UT_CPUDetailGrapTableItem, test_drawNormalMode_01)
{
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    m_tester->drawNormalMode(painter);
}

TEST_F(UT_CPUDetailGrapTableItem, test_drawNormalMode_02)
{
    m_tester->m_isMutliCoreMode = true;

    m_tester->m_cpuPercents.append(0.1);
    m_tester->m_cpuPercents.append(0.2);
    m_tester->m_cpuPercents.append(0.3);

    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    m_tester->drawNormalMode(painter);
}

TEST_F(UT_CPUDetailGrapTableItem, test_drawSimpleMode_01)
{
    m_tester->m_cpuPercents.append(0.1);
    m_tester->m_cpuPercents.append(0.2);
    m_tester->m_cpuPercents.append(0.3);

    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    m_tester->drawSimpleMode(painter);
}

TEST_F(UT_CPUDetailGrapTableItem, test_drawTextMode_01)
{
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    m_tester->drawTextMode(painter);
}

TEST_F(UT_CPUDetailGrapTableItem, test_drawSingleCoreMode_01)
{
    m_tester->m_cpuPercents.append(0.1);
    m_tester->m_cpuPercents.append(0.2);
    m_tester->m_cpuPercents.append(0.3);

    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    m_tester->drawSingleCoreMode(painter);
}

TEST_F(UT_CPUDetailGrapTableItem, test_drawBackground_01)
{
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    QRect rect(0, 0, 10, 10);
    m_tester->drawBackground(painter, rect);
}


class UT_CPUDetailGrapTable : public ::testing::Test
{
public:
    UT_CPUDetailGrapTable() : m_tester2(nullptr) {}

public:
    virtual void SetUp()
    {
        static CPUInfoModel model;
        static QWidget widget;
        m_tester2 = new CPUDetailGrapTable(&model, &widget);
    }

    virtual void TearDown()
    {
        delete m_tester2;
    }

protected:
    CPUDetailGrapTable *m_tester2;
};

TEST_F(UT_CPUDetailGrapTable, initTest)
{

}

TEST_F(UT_CPUDetailGrapTable, test_setMutliCoreMode_01)
{
    m_tester2->setMutliCoreMode(true);

    EXPECT_EQ(m_tester2->m_isMutliCoreMode, true);
}


TEST_F(UT_CPUDetailGrapTable, test_setMutliCoreMode_02)
{
    m_tester2->setMutliCoreMode(false);

    EXPECT_EQ(m_tester2->m_isMutliCoreMode, false);
}

TEST_F(UT_CPUDetailGrapTable, test_setSingleModeLayout_01)
{
    m_tester2->setSingleModeLayout(m_tester2->m_cpuInfoModel);

    EXPECT_EQ(m_tester2->layout()->margin(), 0);
    EXPECT_EQ(dynamic_cast<QGridLayout*>(m_tester2->layout())->verticalSpacing(), 10);
    EXPECT_EQ(dynamic_cast<QGridLayout*>(m_tester2->layout())->horizontalSpacing(), 10);
}

TEST_F(UT_CPUDetailGrapTable, test_setMultiModeLayout_01)
{
    Stub stub;
    stub.set(sysconf, stub_setMultiModeLayout_sysconf_cpu1);
    m_tester2->setMultiModeLayout(m_tester2->m_cpuInfoModel);

    EXPECT_EQ(m_tester2->layout()->margin(), 0);
    EXPECT_EQ(dynamic_cast<QGridLayout*>(m_tester2->layout())->verticalSpacing(), 10);
    EXPECT_EQ(dynamic_cast<QGridLayout*>(m_tester2->layout())->horizontalSpacing(), 10);
}


TEST_F(UT_CPUDetailGrapTable, test_setMultiModeLayout_02)
{
    Stub stub;
    stub.set(sysconf, stub_setMultiModeLayout_sysconf_cpu2);
    m_tester2->setMultiModeLayout(m_tester2->m_cpuInfoModel);

    EXPECT_EQ(m_tester2->layout()->margin(), 0);
    EXPECT_EQ(dynamic_cast<QGridLayout*>(m_tester2->layout())->verticalSpacing(), 10);
    EXPECT_EQ(dynamic_cast<QGridLayout*>(m_tester2->layout())->horizontalSpacing(), 10);
}


TEST_F(UT_CPUDetailGrapTable, test_setMultiModeLayout_04)
{
    Stub stub;
    stub.set(sysconf, stub_setMultiModeLayout_sysconf_cpu4);
    m_tester2->setMultiModeLayout(m_tester2->m_cpuInfoModel);

    EXPECT_EQ(m_tester2->layout()->margin(), 0);
    EXPECT_EQ(dynamic_cast<QGridLayout*>(m_tester2->layout())->verticalSpacing(), 10);
    EXPECT_EQ(dynamic_cast<QGridLayout*>(m_tester2->layout())->horizontalSpacing(), 10);
}


TEST_F(UT_CPUDetailGrapTable, test_setMultiModeLayout_08)
{
    Stub stub;
    stub.set(sysconf, stub_setMultiModeLayout_sysconf_cpu8);
    m_tester2->setMultiModeLayout(m_tester2->m_cpuInfoModel);

    EXPECT_EQ(m_tester2->layout()->margin(), 0);
    EXPECT_EQ(dynamic_cast<QGridLayout*>(m_tester2->layout())->verticalSpacing(), 10);
    EXPECT_EQ(dynamic_cast<QGridLayout*>(m_tester2->layout())->horizontalSpacing(), 10);
}


TEST_F(UT_CPUDetailGrapTable, test_setMultiModeLayout_16)
{
    Stub stub;
    stub.set(sysconf, stub_setMultiModeLayout_sysconf_cpu16);
    m_tester2->setMultiModeLayout(m_tester2->m_cpuInfoModel);

    EXPECT_EQ(m_tester2->layout()->margin(), 0);
    EXPECT_EQ(dynamic_cast<QGridLayout*>(m_tester2->layout())->verticalSpacing(), 10);
    EXPECT_EQ(dynamic_cast<QGridLayout*>(m_tester2->layout())->horizontalSpacing(), 10);
}


TEST_F(UT_CPUDetailGrapTable, test_setMultiModeLayout_32)
{
    Stub stub;
    stub.set(sysconf, stub_setMultiModeLayout_sysconf_cpu32);
    m_tester2->setMultiModeLayout(m_tester2->m_cpuInfoModel);

    EXPECT_EQ(m_tester2->layout()->margin(), 0);
    EXPECT_EQ(dynamic_cast<QGridLayout*>(m_tester2->layout())->verticalSpacing(), 10);
    EXPECT_EQ(dynamic_cast<QGridLayout*>(m_tester2->layout())->horizontalSpacing(), 10);
}


TEST_F(UT_CPUDetailGrapTable, test_setMultiModeLayout_64)
{
    Stub stub;
    stub.set(sysconf, stub_setMultiModeLayout_sysconf_cpu64);
    m_tester2->setMultiModeLayout(m_tester2->m_cpuInfoModel);

    EXPECT_EQ(m_tester2->layout()->margin(), 0);
    EXPECT_EQ(dynamic_cast<QGridLayout*>(m_tester2->layout())->verticalSpacing(), 10);
    EXPECT_EQ(dynamic_cast<QGridLayout*>(m_tester2->layout())->horizontalSpacing(), 10);
}

TEST_F(UT_CPUDetailGrapTable, test_setMultiModeLayout_0)
{
    Stub stub;
    stub.set(sysconf, stub_setMultiModeLayout_sysconf_cpu0);
    m_tester2->setMultiModeLayout(m_tester2->m_cpuInfoModel);

    EXPECT_EQ(m_tester2->layout()->margin(), 0);
    EXPECT_EQ(dynamic_cast<QGridLayout*>(m_tester2->layout())->verticalSpacing(), 10);
    EXPECT_EQ(dynamic_cast<QGridLayout*>(m_tester2->layout())->horizontalSpacing(), 10);
}

class UT_CPUDetailWidget : public ::testing::Test
{
public:
    UT_CPUDetailWidget() : m_tester3(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester3 = new CPUDetailWidget();
    }

    virtual void TearDown()
    {
        delete m_tester3;
    }

protected:
    CPUDetailWidget *m_tester3;
};

TEST_F(UT_CPUDetailWidget, initTest)
{

}

TEST_F(UT_CPUDetailWidget, test_detailFontChanged_01)
{
    QFont font;
    font.setBold(true);
    m_tester3->detailFontChanged(font);
}
