// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test.h"
#include "dfontmanager.h"

#include <QApplication>

DGUI_USE_NAMESPACE

TEST(ut_DFontManager, StaticFunction)
{
    enum { FontSize = 18 };

    QFont tF = DFontManager::get(FontSize, qApp->font());
    ASSERT_EQ(tF.pixelSize(), FontSize);
    ASSERT_EQ(DFontManager::fontPixelSize(tF), FontSize);

    tF.setPointSizeF(FontSize);
    ASSERT_TRUE(DFontManager::fontPixelSize(tF) > 0);
}

class TDFontManager : public DTestWithParam<int>
{
protected:
    void SetUp();
    void TearDown();

    DFontManager *manager;
};

INSTANTIATE_TEST_CASE_P(DFontManager, TDFontManager, testing::Range(int(DFontManager::T1), int(DFontManager::NSizeTypes)));

void TDFontManager::SetUp()
{
    manager = new DFontManager;
}

void TDFontManager::TearDown()
{
    delete manager;
}

TEST_P(TDFontManager, testGetFont)
{
    int param = GetParam();

    QFont font = manager->get(DFontManager::SizeType(param), qApp->font());
    ASSERT_EQ(font.pixelSize(), manager->fontPixelSize(DFontManager::SizeType(param)));

    manager->setBaseFont(qApp->font());
    ASSERT_EQ(manager->baseFont(), qApp->font());

    font = manager->get(DFontManager::SizeType(param));
    ASSERT_EQ(font.pixelSize(), manager->fontPixelSize(DFontManager::SizeType(param)));

    manager->resetBaseFont();
    ASSERT_NE(manager->baseFont(), qApp->font());

    manager->setFontPixelSize(DFontManager::SizeType(param), param);
    ASSERT_EQ(manager->fontPixelSize(DFontManager::SizeType(param)), param);
}

TEST_F(TDFontManager, testFontSize)
{
    manager->setBaseFont(qApp->font());

    ASSERT_EQ(manager->t1().pixelSize(), manager->fontPixelSize(DFontManager::T1));
    ASSERT_EQ(manager->t2().pixelSize(), manager->fontPixelSize(DFontManager::T2));
    ASSERT_EQ(manager->t3().pixelSize(), manager->fontPixelSize(DFontManager::T3));
    ASSERT_EQ(manager->t4().pixelSize(), manager->fontPixelSize(DFontManager::T4));
    ASSERT_EQ(manager->t5().pixelSize(), manager->fontPixelSize(DFontManager::T5));
    ASSERT_EQ(manager->t6().pixelSize(), manager->fontPixelSize(DFontManager::T6));
    ASSERT_EQ(manager->t7().pixelSize(), manager->fontPixelSize(DFontManager::T7));
    ASSERT_EQ(manager->t8().pixelSize(), manager->fontPixelSize(DFontManager::T8));
    ASSERT_EQ(manager->t9().pixelSize(), manager->fontPixelSize(DFontManager::T9));
    ASSERT_EQ(manager->t10().pixelSize(), manager->fontPixelSize(DFontManager::T10));
}
