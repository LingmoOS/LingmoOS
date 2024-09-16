// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

//#include "test_colorpickerwidget.h"

//test_colorpickerwidget::test_colorpickerwidget(QObject *parent) : QObject(parent)
//{

//}

//TEST_F(test_colorpickerwidget, getSelectedColor_001)
//{
//    mWidget->slotUpdateColor(QColor("#000000"));
//    EXPECT_EQ(mWidget->getSelectedColor(), "#000000") << "text: #000000";
//    mWidget->slotUpdateColor(QColor("#123456"));
//    EXPECT_EQ(mWidget->getSelectedColor(), "#123456") << "text: #123456";
//}

//TEST_F(test_colorpickerwidget, setColorHexLineEdit_001)
//{
//    mWidget->setColorHexLineEdit();
//    EXPECT_TRUE(mWidget->m_colHexLineEdit->text().isEmpty());
//}

//TEST_F(test_colorpickerwidget, slotUpdateColor)
//{
//    mWidget->slotUpdateColor(QColor("#000000"));
//    EXPECT_EQ(mWidget->m_colHexLineEdit->text(), "000000") << "text: #000000";
//    mWidget->slotUpdateColor(QColor("#123456"));
//    EXPECT_EQ(mWidget->m_colHexLineEdit->text(), "123456") << "text: #123456";
//}

//TEST_F(test_colorpickerwidget, slotHexLineEditChange_001)
//{
//    mWidget->slotHexLineEditChange("");
//    EXPECT_FALSE(mWidget->m_enterBtn->isEnabled());
//}

//TEST_F(test_colorpickerwidget, slotHexLineEditChange_002)
//{
//    mWidget->slotHexLineEditChange("FFfF");
//    EXPECT_EQ(mWidget->m_colHexLineEdit->text(), "ffff") << "eq";
//    EXPECT_FALSE(mWidget->m_enterBtn->isEnabled()) << "false";
//}

//TEST_F(test_colorpickerwidget, slotHexLineEditChange_003)
//{
//    mWidget->slotHexLineEditChange("FFfFff");
//    EXPECT_EQ(mWidget->m_colHexLineEdit->text(), "ffffff") << "eq";
//    EXPECT_TRUE(mWidget->m_enterBtn->isEnabled()) << "true";
//}

//TEST_F(test_colorpickerwidget, slotCancelBtnClicked_001)
//{
//    mWidget->slotCancelBtnClicked();
//}

//TEST_F(test_colorpickerwidget, slotEnterBtnClicked_001)
//{
//    mWidget->slotEnterBtnClicked();
//}


