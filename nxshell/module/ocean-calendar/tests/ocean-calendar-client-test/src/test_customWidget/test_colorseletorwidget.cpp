// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_colorseletorwidget.h"
#include "../dialog_stub.h"

//test_colorseletorwidget::test_colorseletorwidget(QObject *parent) : QObject(parent)
//{

//}

//TEST_F(test_colorseletorwidget, reset_001)
//{
//    mWidget->reset();
//    EXPECT_TRUE(mWidget->m_colorEntityMap.isEmpty());
//}

//TEST_F(test_colorseletorwidget, getSelectedColorInfo_001)
//{
//    JobTypeColorInfo info(0, "#123456", 1);
//    mWidget->m_colorInfo = info;
//    EXPECT_TRUE(mWidget->getSelectedColorInfo() == info);
//}

//TEST_F(test_colorseletorwidget, setUserColor_001)
//{
//    JobTypeColorInfo info(0, "#123456", ColorSeletorWidget::TypeSystem);
//    mWidget->setUserColor(info);
//    EXPECT_TRUE(mWidget->m_userColorBtn == nullptr);
//}

//TEST_F(test_colorseletorwidget, setUserColor_002)
//{
//    JobTypeColorInfo info(0, "#123456", ColorSeletorWidget::TypeUser);
//    mWidget->setUserColor(info);
//    EXPECT_TRUE(mWidget->m_userColorBtn != nullptr);
//}

//TEST_F(test_colorseletorwidget, setSelectedColorByIndex_001)
//{
//    mWidget->reset();
//    JobTypeColorInfo info(0, "#123456", ColorSeletorWidget::TypeUser);
//    mWidget->setUserColor(info);
//    mWidget->setSelectedColorByIndex(0);
//    EXPECT_TRUE(mWidget->m_userColorBtn->isChecked());
//}

//TEST_F(test_colorseletorwidget, setSelectedColorById_001)
//{
//    mWidget->reset();
//    mWidget->addColor(JobTypeColorInfo(1, "#123456", ColorSeletorWidget::TypeSystem));
//    mWidget->addColor(JobTypeColorInfo(2, "#111111", ColorSeletorWidget::TypeSystem));
//    mWidget->addColor(JobTypeColorInfo(3, "#222222", ColorSeletorWidget::TypeSystem));
//    mWidget->addColor(JobTypeColorInfo(4, "#333333", ColorSeletorWidget::TypeSystem));
//    mWidget->addColor(JobTypeColorInfo(0, "#444444", ColorSeletorWidget::TypeUser));
//    mWidget->setSelectedColorById(0);
//    EXPECT_TRUE(mWidget->m_userColorBtn->isChecked()) << "0";
//    mWidget->setSelectedColorById(3);
//    EXPECT_FALSE(mWidget->m_userColorBtn->isChecked()) << "3";
//}

//TEST_F(test_colorseletorwidget, setSelectedColor_001)
//{
//    mWidget->reset();
//    mWidget->addColor(JobTypeColorInfo(1, "#123456", ColorSeletorWidget::TypeSystem));
//    mWidget->addColor(JobTypeColorInfo(2, "#111111", ColorSeletorWidget::TypeSystem));
//    mWidget->addColor(JobTypeColorInfo(3, "#222222", ColorSeletorWidget::TypeSystem));
//    mWidget->addColor(JobTypeColorInfo(4, "#333333", ColorSeletorWidget::TypeSystem));
//    mWidget->addColor(JobTypeColorInfo(0, "#444444", ColorSeletorWidget::TypeUser));
//    mWidget->setSelectedColor(JobTypeColorInfo(0, "#444444", ColorSeletorWidget::TypeUser));
//    EXPECT_TRUE(mWidget->m_userColorBtn->isChecked()) << "1";
//    mWidget->setSelectedColor((JobTypeColorInfo(4, "#333333", ColorSeletorWidget::TypeSystem)));
//    EXPECT_FALSE(mWidget->m_userColorBtn->isChecked()) << "2";
//    mWidget->setSelectedColor((JobTypeColorInfo(4, "#555555", ColorSeletorWidget::TypeSystem)));
//    EXPECT_TRUE(mWidget->m_userColorBtn->isChecked()) << "3";
//}

//TEST_F(test_colorseletorwidget, initColorButton_001)
//{
//    mWidget->initColorButton();
//}

//TEST_F(test_colorseletorwidget, slotButtonClicked_001)
//{
//    mWidget->reset();
//    mWidget->addColor(JobTypeColorInfo(1, "#123456", ColorSeletorWidget::TypeSystem));
//    mWidget->addColor(JobTypeColorInfo(2, "#111111", ColorSeletorWidget::TypeSystem));
//    mWidget->addColor(JobTypeColorInfo(3, "#222222", ColorSeletorWidget::TypeSystem));
//    mWidget->addColor(JobTypeColorInfo(4, "#333333", ColorSeletorWidget::TypeSystem));
//    mWidget->addColor(JobTypeColorInfo(0, "#444444", ColorSeletorWidget::TypeUser));
//    mWidget->slotButtonClicked(1);
//    mWidget->slotButtonClicked(6);
//}

//TEST_F(test_colorseletorwidget, slotAddColorButClicked_001)
//{
//    calendarDDialogExecReturn = 1;
//    Stub stub;
//    calendarDDialogExecStub(stub);

//    mWidget->slotAddColorButClicked();
//    EXPECT_TRUE(mWidget->m_userColorBtn->isChecked());
//}

//TEST_F(test_colorseletorwidget, addColor_001)
//{
//    mWidget->reset();
//    mWidget->addColor(JobTypeColorInfo(0, "", 1));
//    EXPECT_EQ(mWidget->m_colorEntityMap.size(), 1);
//}

//TEST_F(test_colorseletorwidget, addColor_002)
//{
//    mWidget->reset();
//    mWidget->addColor(JobTypeColorInfo(0, "", ColorSeletorWidget::TypeUser));
//    EXPECT_EQ(mWidget->m_colorEntityMap.size(), 1) << "eq";
//    EXPECT_NE(mWidget->m_userColorBtn, nullptr) << "ne";
//}
