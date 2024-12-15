// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

//#include "test_cdateedit.h"

//test_cdateedit::test_cdateedit(QObject *parent) : QObject(parent)
//{

//}

//TEST_F(test_cdateedit, setDate_001)
//{
//    QDate date = QDate(2022, 4, 20);
//    mWidget->setDate(date);
//    EXPECT_EQ(mWidget->date().toString(), date.toString());
//}

//TEST_F(test_cdateedit, setDisplayFormat_001)
//{
//    mWidget->setDisplayFormat("123");
//    EXPECT_EQ(mWidget->m_format, "123") << "123";
//    mWidget->setDisplayFormat("yyyy/MM/dd");
//    EXPECT_EQ(mWidget->m_format, "yyyy/MM/dd") << "yyyy/MM/dd";
//}

//TEST_F(test_cdateedit, displayFormat_001)
//{
//    mWidget->setDisplayFormat("123");
//    EXPECT_EQ(mWidget->displayFormat(), "123") << "123";
//    mWidget->setDisplayFormat("yyyy/MM/dd");
//    EXPECT_EQ(mWidget->displayFormat(), "yyyy/MM/dd") << "yyyy/MM/dd";
//}

//TEST_F(test_cdateedit, setLunarCalendarStatus_001)
//{
//    mWidget->setLunarCalendarStatus(true);
//    EXPECT_TRUE(mWidget->m_showLunarCalendar) << "true";
//    mWidget->setLunarCalendarStatus(false);
//    EXPECT_FALSE(mWidget->m_showLunarCalendar) << "false";
//}

//TEST_F(test_cdateedit, setLunarTextFormat_001)
//{
//    QTextCharFormat format;
//    mWidget->setLunarTextFormat(format);
//    EXPECT_EQ(mWidget->m_lunarTextFormat, format);
//}

//TEST_F(test_cdateedit, getsetLunarTextFormat_001)
//{
//    QTextCharFormat format;
//    mWidget->setLunarTextFormat(format);
//    EXPECT_EQ(mWidget->getsetLunarTextFormat(), format);
//}

//TEST_F(test_cdateedit, setCalendarPopup_001)
//{
//    mWidget->setCalendarPopup(true);
//}

//TEST_F(test_cdateedit, slotDateEidtInfo_001)
//{
//    mWidget->setLunarCalendarStatus(true);
//    mWidget->slotDateEidtInfo(QDate());
//}

//TEST_F(test_cdateedit, slotDateEidtInfo_002)
//{
//    mWidget->setLunarCalendarStatus(false);
//    mWidget->slotDateEidtInfo(QDate());
//}

//TEST_F(test_cdateedit, slotRefreshLineEditTextFormat_001)
//{
//    mWidget->setLunarCalendarStatus(true);
//    mWidget->slotRefreshLineEditTextFormat("1232/23/13");
//}

//TEST_F(test_cdateedit, slotCursorPositionChanged_001)
//{
//    mWidget->setLunarCalendarStatus(true);
//    mWidget->slotCursorPositionChanged(0, 2);
//}

//TEST_F(test_cdateedit, slotCursorPositionChanged_002)
//{
//    mWidget->setLunarCalendarStatus(false);
//    mWidget->slotCursorPositionChanged(0, 2);
//}

//TEST_F(test_cdateedit, slotCursorPositionChanged_003)
//{
//    mWidget->setLunarCalendarStatus(true);
//    mWidget->selectAll();
//    mWidget->slotCursorPositionChanged(2, 4);
//}

//TEST_F(test_cdateedit, slotSelectionChanged_001)
//{
//    mWidget->setLunarCalendarStatus(true);
//    mWidget->selectAll();
//    mWidget->slotSelectionChanged();
//}

//TEST_F(test_cdateedit, slotSelectionChanged_002)
//{
//    mWidget->setLunarCalendarStatus(false);
//    mWidget->selectAll();
//    mWidget->slotSelectionChanged();
//}

//TEST_F(test_cdateedit, getLunarName_001)
//{
//    mWidget->setLunarCalendarStatus(false);
//    mWidget->getLunarName(QDate(2022, 4, 20));
//}

//TEST_F(test_cdateedit, setLineEditTextFormat_001)
//{
//    mWidget->setLineEditTextFormat(mWidget->lineEdit(), QList<QTextLayout::FormatRange>());
//}

//TEST_F(test_cdateedit, updateCalendarWidget_001)
//{
//    mWidget->setCalendarPopup(true);
//    mWidget->setLunarCalendarStatus(true);
//    mWidget->updateCalendarWidget();
//}

//TEST_F(test_cdateedit, updateCalendarWidget_002)
//{
//    mWidget->setCalendarPopup(true);
//    mWidget->setLunarCalendarStatus(false);
//    mWidget->updateCalendarWidget();
//}
