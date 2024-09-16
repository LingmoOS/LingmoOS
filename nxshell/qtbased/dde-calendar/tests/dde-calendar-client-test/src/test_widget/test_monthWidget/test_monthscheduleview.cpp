// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test_monthscheduleview.h"
#include "dataManage/scheduledaterangeinfo.h"

//test_monthscheduleview::test_monthscheduleview()
//{
//    QRect rect;
//    m_scene = new QGraphicsScene();
//    mMonthScheduleView = new CMonthScheduleView(nullptr, m_scene);
//    mMonthScheduleWidgetItem = new CMonthScheduleItem(rect);
//    mWeekScheduleView = new CWeekScheduleView();
//    mMonthScheduleNumButton = new CMonthScheduleNumItem();
//}

//test_monthscheduleview::~test_monthscheduleview()
//{
//    delete mMonthScheduleView;
//    mMonthScheduleView = nullptr;
//    delete m_scene;
//    m_scene = nullptr;
//    delete mWeekScheduleView;
//    mWeekScheduleView = nullptr;
//    delete mMonthScheduleNumButton;
//    mMonthScheduleNumButton = nullptr;
//    delete mMonthScheduleWidgetItem;
//    mMonthScheduleWidgetItem = nullptr;
//}

//QVector<ScheduleDataInfo> getMonthScheduleDInfo()
//{
//    QVector<ScheduleDataInfo> scheduleDate {};
//    ScheduleDataInfo schedule1, schedule2, schedule3, schedule4, schedule5, scheduleFes;
//    QDateTime currentDateTime = QDateTime::currentDateTime();

//    schedule1.setID(1);
//    schedule1.setBeginDateTime(currentDateTime);
//    schedule1.setEndDateTime(currentDateTime.addDays(1));
//    schedule1.setTitleName("scheduleOne");
//    schedule1.setAllDay(true);
//    schedule1.setType(1);
//    schedule1.setRecurID(0);

//    schedule2.setID(2);
//    schedule2.setBeginDateTime(currentDateTime.addDays(1));
//    schedule2.setEndDateTime(currentDateTime.addDays(1).addSecs(60 * 60));
//    schedule2.setTitleName("scheduleTwo");
//    schedule2.setAllDay(true);
//    schedule2.setType(2);
//    schedule2.setRecurID(0);

//    schedule3.setID(3);
//    schedule3.setBeginDateTime(currentDateTime.addDays(2));
//    schedule3.setEndDateTime(currentDateTime.addDays(2).addSecs(60 * 60));
//    schedule3.setTitleName("scheduleThree");
//    schedule3.setAllDay(false);
//    schedule3.setType(3);
//    schedule3.setRecurID(0);

//    schedule4.setID(4);
//    schedule4.setBeginDateTime(currentDateTime.addDays(3));
//    schedule4.setEndDateTime(currentDateTime.addDays(3).addSecs(60 * 60));
//    schedule4.setTitleName("scheduleFour");
//    schedule4.setAllDay(false);
//    schedule4.setType(1);
//    schedule4.setRecurID(0);

//    schedule5.setID(5);
//    schedule5.setBeginDateTime(currentDateTime.addDays(4));
//    schedule5.setEndDateTime(currentDateTime.addDays(4).addSecs(60 * 60));
//    schedule5.setTitleName("scheduleFive");
//    schedule5.setAllDay(false);
//    schedule5.setType(2);
//    schedule5.setRecurID(0);

//    scheduleFes.setID(6);
//    scheduleFes.setBeginDateTime(currentDateTime.addDays(5));
//    scheduleFes.setEndDateTime(currentDateTime.addDays(5).addSecs(60 * 60));
//    scheduleFes.setTitleName("scheduleFestival");
//    scheduleFes.setAllDay(true);
//    scheduleFes.setType(4);
//    scheduleFes.setRecurID(0);

//    scheduleDate.append(schedule1);
//    scheduleDate.append(schedule2);
//    scheduleDate.append(schedule3);
//    scheduleDate.append(schedule4);
//    scheduleDate.append(schedule5);
//    scheduleDate.append(scheduleFes);
//    return scheduleDate;
//}

//QMap<QDate, QVector<ScheduleDataInfo>> getMonthMapScheduleDInfo(int getDays)
//{
//    QDate currentDate = QDate::currentDate();
//    QVector<ScheduleDataInfo> scheduleInfo {};
//    QMap<QDate, QVector<ScheduleDataInfo>> scheduleDateInfo {};
//    switch (getDays) {
//    case 0: {
//        scheduleInfo.append(getMonthScheduleDInfo().at(0));
//        scheduleDateInfo[currentDate] = scheduleInfo;
//    } break;
//    case 1: {
//        scheduleInfo.append(getMonthScheduleDInfo().at(1));
//        scheduleDateInfo[currentDate.addDays(1)] = scheduleInfo;
//    } break;
//    case 2: {
//        scheduleInfo.append(getMonthScheduleDInfo().at(2));
//        scheduleDateInfo[currentDate.addDays(2)] = scheduleInfo;
//    } break;
//    default: {
//        scheduleInfo.append(getMonthScheduleDInfo().at(3));
//        scheduleDateInfo[currentDate.addDays(3)] = scheduleInfo;
//        scheduleInfo.append(getMonthScheduleDInfo().at(4));
//        scheduleDateInfo[currentDate.addDays(4)] = scheduleInfo;
//        scheduleInfo.append(getMonthScheduleDInfo().at(5));
//        scheduleDateInfo[currentDate.addDays(5)] = scheduleInfo;
//    } break;
//    }
//    return scheduleDateInfo;
//}

//QVector<MScheduleDateRangeInfo> getScheduleDateRangeInfo()
//{
//    QVector<MScheduleDateRangeInfo> scheduleDateRangeInfo {};
//    MScheduleDateRangeInfo scheduledaterangeinfo1;
//    scheduledaterangeinfo1.num = 4;
//    scheduledaterangeinfo1.bdate = QDate::currentDate();
//    scheduledaterangeinfo1.edate = QDate::currentDate().addDays(1);
//    scheduledaterangeinfo1.state = true;
//    scheduledaterangeinfo1.tData = getMonthScheduleDInfo().first();

//    //    MScheduleDateRangeInfo scheduledaterangeinfo2;
//    //    scheduledaterangeinfo2.num = 2;
//    //    scheduledaterangeinfo2.bDate = QDate(2021, 1, 23);
//    //    scheduledaterangeinfo2.eDate = QDate(2021, 1, 24);
//    //    scheduledaterangeinfo2.state = false;
//    //    scheduledaterangeinfo2.tData = getMonthScheduleDInfo().at(1);

//    //    MScheduleDateRangeInfo scheduledaterangeinfo3;
//    //    scheduledaterangeinfo3.num = 2;
//    //    scheduledaterangeinfo3.bDate = QDate(2021, 1, 25);
//    //    scheduledaterangeinfo3.eDate = QDate(2021, 1, 26);
//    //    scheduledaterangeinfo3.state = false;
//    //    scheduledaterangeinfo3.tData = getMonthScheduleDInfo().at(2);

//    //    MScheduleDateRangeInfo scheduledaterangeinfo4;
//    //    scheduledaterangeinfo4.num = 2;
//    //    scheduledaterangeinfo4.bDate = QDate(2021, 1, 27);
//    //    scheduledaterangeinfo4.eDate = QDate(2021, 1, 28);
//    //    scheduledaterangeinfo4.state = false;
//    //    scheduledaterangeinfo4.tData = getMonthScheduleDInfo().at(3);

//    scheduleDateRangeInfo.append(scheduledaterangeinfo1);
//    //    scheduleDateRangeInfo.append(scheduledaterangeinfo2);
//    //    scheduleDateRangeInfo.append(scheduledaterangeinfo3);
//    //    scheduleDateRangeInfo.append(scheduledaterangeinfo4);

//    return scheduleDateRangeInfo;
//}

////void CMonthScheduleView::setallsize(int w, int h, int left, int top, int buttom, int itemHeight)
//TEST_F(test_monthscheduleview, setallsize)
//{
//    mMonthScheduleView->setallsize(1, 1, 1, 1, 1, 1);
//}

////void CMonthScheduleView::setData(QMap<QDate, QVector<ScheduleDataInfo> > &data, int currentMonth)
//TEST_F(test_monthscheduleview, setData)
//{
//    QMap<QDate, QVector<ScheduleDataInfo>> mapScheduleInfo = getMonthMapScheduleDInfo(4);
//    mMonthScheduleView->setData(mapScheduleInfo, 1);
//}

////void CMonthScheduleView::slotFontChange()
//TEST_F(test_monthscheduleview, slotFontChange)
//{
//    mMonthScheduleView->slotFontChange();
//}

////void CMonthScheduleView::updateData()
//TEST_F(test_monthscheduleview, updateDate)
//{
//    mMonthScheduleView->updateData();
//}

////void CMonthScheduleView::updateHeight()
//TEST_F(test_monthscheduleview, updateHeight)
//{
//    mMonthScheduleView->updateHeight();
//}

////QVector<QGraphicsRectItem *> CMonthScheduleView::getScheduleShowItem() const
//TEST_F(test_monthscheduleview, getScheduleShowItem)
//{
//    mMonthScheduleView->getScheduleShowItem();
//}

////void CMonthScheduleView::updateDate(const ScheduleDataInfo &info)
//TEST_F(test_monthscheduleview, updateData)
//{
//    mMonthScheduleView->updateDate(getMonthScheduleDInfo().first());
//}

////void CMonthScheduleView::changeDate(const ScheduleDataInfo &info)
//TEST_F(test_monthscheduleview, changeDate)
//{
//    mMonthScheduleView->changeDate(getMonthScheduleDInfo().first());
//}

////void CMonthScheduleView::updateDate(const int row, const ScheduleDataInfo &info)
//TEST_F(test_monthscheduleview, updatedate)
//{
//    mMonthScheduleView->updateDate(2, getMonthScheduleDInfo().first());
//}

////void CMonthScheduleView::updateDateShow(QVector<QVector<MScheduleDateRangeInfo>> &vCMDaySchedule, QVector<QGraphicsRectItem *> &scheduleShowItem)
//TEST_F(test_monthscheduleview, updateDateShow)
//{
//    QVector<MScheduleDateRangeInfo> scheduleinfo = getScheduleDateRangeInfo();
//    QVector<QVector<MScheduleDateRangeInfo>> m_scheduledaterangeinfo {};
//    QVector<QGraphicsRectItem *> m_scheduleshowinfo {};
//    m_scheduledaterangeinfo.append(scheduleinfo);
//    m_scheduleshowinfo.append(mMonthScheduleWidgetItem);

//    mMonthScheduleView->updateDateShow(m_scheduledaterangeinfo, m_scheduleshowinfo);
//}

////void CMonthScheduleView::createScheduleItemWidget(MScheduleDateRangeInfo info, int cNum, QVector<QGraphicsRectItem *> &scheduleShowItem)
//TEST_F(test_monthscheduleview, createScheduleItemWidget)
//{
//    QVector<MScheduleDateRangeInfo> scheduledaterangeinfo = getScheduleDateRangeInfo();
//    QVector<QGraphicsRectItem *> m_scheduleshowinfo {};
//    m_scheduleshowinfo.append(mMonthScheduleWidgetItem);
//    mMonthScheduleView->createScheduleItemWidget(scheduledaterangeinfo.first(), 1, m_scheduleshowinfo);
//}

////void CMonthScheduleView::createScheduleNumWidget(MScheduleDateRangeInfo info, int cNum, QVector<QGraphicsRectItem *> &scheduleShowItem)
//TEST_F(test_monthscheduleview, createScheduleNumWidget)
//{
//    MScheduleDateRangeInfo scheduledaterangeinfo = getScheduleDateRangeInfo().first();
//    QVector<QGraphicsRectItem *> m_scheduleshowinfo {};
//    QMap<QDate, QVector<ScheduleDataInfo>> mapScheduleInfo = getMonthMapScheduleDInfo(4);
//    mMonthScheduleView->setData(mapScheduleInfo, 3);
//    mMonthScheduleView->createScheduleNumWidget(scheduledaterangeinfo, 2, m_scheduleshowinfo);
//}

////void CMonthScheduleView::computePos(int cnum, QDate bgeindate, QDate enddate, QPoint &pos, int &fw, int &fh)
//TEST_F(test_monthscheduleview, computePos)
//{
//    QDate bDate = QDate(2020, 1, 20);
//    QDate eDate = QDate(2021, 1, 23);
//    QPoint pos = QPoint(4, 4);
//    int fw = 8;
//    int fh = 8;
//    mMonthScheduleView->computePos(2, bDate, eDate, pos, fw, fh);
//}

////void CWeekScheduleView::setData(QMap<QDate, QVector<ScheduleDataInfo> > &data, const QDate &startDate, const QDate &stopDate)
//TEST_F(test_monthscheduleview, setweekData)
//{
//    QMap<QDate, QVector<ScheduleDataInfo>> mapScheduleInfo = getMonthMapScheduleDInfo(4);
//    QDate startDate = QDate(2021, 1, 21);
//    QDate stopData = QDate(2021, 1, 27);
//    mWeekScheduleView->setData(mapScheduleInfo, startDate, stopData);
//}

////bool CWeekScheduleView::addData(const ScheduleDataInfo &info)
//TEST_F(test_monthscheduleview, addData)
//{
//    QMap<QDate, QVector<ScheduleDataInfo>> mapScheduleInfo = getMonthMapScheduleDInfo(4);
//    QDate startDate = QDate::currentDate();
//    QDate stopData = startDate.addDays(6);
//    mWeekScheduleView->setData(mapScheduleInfo, startDate, stopData);
//    bool re = mWeekScheduleView->addData(getMonthScheduleDInfo().first());
//    EXPECT_TRUE(re);

//    startDate = QDate::currentDate().addDays(-7);
//    stopData = QDate::currentDate().addDays(-1);
//    mWeekScheduleView->setData(mapScheduleInfo, startDate, stopData);
//    re = mWeekScheduleView->addData(getMonthScheduleDInfo().first());
//    EXPECT_FALSE(re);
//}

////void CWeekScheduleView::changeDate(const ScheduleDataInfo &info)
//TEST_F(test_monthscheduleview, changedate)
//{
//    mWeekScheduleView->changeDate(getMonthScheduleDInfo().first());
//}

////void CWeekScheduleView::setHeight(const int ScheduleHeight, const int DayHeigth)
//TEST_F(test_monthscheduleview, setHeight)
//{
//    mWeekScheduleView->setHeight(10, 10);
//}

////void CWeekScheduleView::updateSchedule(const bool isNormalDisplay, const ScheduleDataInfo &info)
//TEST_F(test_monthscheduleview, updateSchedule)
//{
//    mWeekScheduleView->updateSchedule(true, getMonthScheduleDInfo().first());
//    mWeekScheduleView->updateSchedule(false, getMonthScheduleDInfo().first());
//}

////void CWeekScheduleView::clearItem()
//TEST_F(test_monthscheduleview, clearItem)
//{
//    mWeekScheduleView->clearItem();
//}

////void CWeekScheduleView::setMaxNum()
//TEST_F(test_monthscheduleview, setMacNum)
//{
//    mWeekScheduleView->setMaxNum();
//}

////void CWeekScheduleView::mScheduleClear()
//TEST_F(test_monthscheduleview, mScheduleClear)
//{
//    mWeekScheduleView->mScheduleClear();
//}

////void CWeekScheduleView::sortAndFilter(QVector<MScheduleDateRangeInfo> &vMDaySchedule)
//TEST_F(test_monthscheduleview, sortAndFileter)
//{
//    QVector<MScheduleDateRangeInfo> scheduleinfo = getScheduleDateRangeInfo();
//    mWeekScheduleView->setHeight(4, 20);
//    QMap<QDate, QVector<ScheduleDataInfo>> mapScheduleInfo = getMonthMapScheduleDInfo(4);
//    QDate startDate = QDate::currentDate();
//    QDate stopData = startDate.addDays(6);
//    mWeekScheduleView->setData(mapScheduleInfo, startDate, stopData);
//    mWeekScheduleView->sortAndFilter(scheduleinfo);
//}

////void CMonthScheduleNumButton::setColor(QColor color1, QColor color2, bool GradientFlag)
//TEST_F(test_monthscheduleview, setColor)
//{
//    QColor color1(100, 100, 255);
//    QColor color2(200, 200, 255);
//    mMonthScheduleNumButton->setColor(color1, color2);
//}

////void CMonthScheduleNumButton::setText(QColor tcolor, QFont font, QPoint pos)
//TEST_F(test_monthscheduleview, setText)
//{
//    QColor color(100, 100, 255);
//    QFont font;
//    mMonthScheduleNumButton->setText(color, font);
//}

////void CMonthScheduleNumButton::setSizeType(DFontSizeManager::SizeType sizeType)
//TEST_F(test_monthscheduleview, setSizeType)
//{
//    mMonthScheduleNumButton->setSizeType(DFontSizeManager::SizeType::T1);
//}

////QPixmap CMonthScheduleWidgetItem::getPixmap()
//TEST_F(test_monthscheduleview, getPixmap)
//{
//    mMonthScheduleWidgetItem->getPixmap();
//}
