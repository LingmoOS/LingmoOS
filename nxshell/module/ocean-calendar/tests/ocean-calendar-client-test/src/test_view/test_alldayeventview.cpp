// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

//#include "test_alldayeventview.h"
//#include "../dialog_stub.h"

//#include <QTest>
//#include <QEvent>
//#include <QMenu>

//test_alldayeventview::test_alldayeventview()
//{
//}

//void test_alldayeventview::SetUp()
//{
//    cAllDayEventWeekView = new CAllDayEventWeekView();
//    cAllDayEventWeekView->setFixedSize(QSize(800, 300));
//}

//void test_alldayeventview::TearDown()
//{
//    delete cAllDayEventWeekView;
//    cAllDayEventWeekView = nullptr;
//}

////void CAllDayEventWeekView::setTheMe(int type)
//TEST_F(test_alldayeventview, setTheMe)
//{
//    int type = 1;
//    cAllDayEventWeekView->setTheMe();
//    cAllDayEventWeekView->setTheMe(type);
//}

////bool CAllDayEventWeekView::MeetCreationConditions(const QDateTime &date)
//TEST_F(test_alldayeventview, MeetCreationConditions)
//{
//    QString begin = "2020-12-01 12:24:36";
//    QDateTime date = QDateTime::fromString(begin, "yyyy-MM-dd hh:mm:ss");
//    cAllDayEventWeekView->MeetCreationConditions(date);
//}

////bool CAllDayEventWeekView::IsEqualtime(const QDateTime &timeFirst, const QDateTime &timeSecond)
//TEST_F(test_alldayeventview, IsEqualtime)
//{
//    QString begin = "2020-12-01 12:24:36";
//    QDateTime begindate = QDateTime::fromString(begin, "yyyy-MM-dd hh:mm:ss");
//    QString end = "2020-12-21 12:24:36";
//    QDateTime enddate = QDateTime::fromString(end, "yyyy-MM-dd hh:mm:ss");

//    assert(false == cAllDayEventWeekView->IsEqualtime(begindate, enddate));

//    enddate = QDateTime::fromString(begin, "yyyy-MM-dd hh:mm:ss");
//    assert(true == cAllDayEventWeekView->IsEqualtime(begindate, enddate));
//}

////void CAllDayEventWeekView::setRange(int w, int h, QDate begindate, QDate enddate, int rightmagin)
//TEST_F(test_alldayeventview, setRange)
//{
//    int w = 3;
//    int h = 2;
//    QDate begindate(2020, 12, 01);
//    QDate enddate(2020, 12, 21);
//    int rightmagin = 2;
//    cAllDayEventWeekView->setRange(w, h, begindate, enddate, rightmagin);
//}

//QVector<ScheduleDataInfo> getTestScheduleDataInfo()
//{
//    ScheduleDataInfo info1;
//    info1.setID(1);
//    info1.setType(2);
//    info1.setAllDay(true);
//    info1.setRecurID(2);
//    info1.setTitleName("测试1");
//    QString strDate = "2020-12-02 12:24:36";
//    QDateTime ignoreDate = QDateTime::fromString(strDate, "yyyy-MM-dd hh:mm:ss");
//    QVector<QDateTime> ignoreDateList;
//    ignoreDateList.append(ignoreDate);
//    info1.setIgnoreTime(ignoreDateList);
//    QString begin = "2020-12-01 12:24:36";
//    QDateTime begindate = QDateTime::fromString(begin, "yyyy-MM-dd hh:mm:ss");
//    QString end = "2020-12-03 12:24:36";
//    QDateTime enddate = QDateTime::fromString(end, "yyyy-MM-dd hh:mm:ss");
//    info1.setBeginDateTime(begindate);
//    info1.setEndDateTime(enddate);

//    ScheduleDataInfo info2;
//    info2.setID(1);
//    info2.setType(2);
//    info2.setAllDay(true);
//    info2.setRecurID(2);
//    info2.setTitleName("测试2");
//    QString strDate2 = "2020-12-12 12:24:36";
//    QDateTime ignoreDate2 = QDateTime::fromString(strDate2, "yyyy-MM-dd hh:mm:ss");
//    QVector<QDateTime> ignoreDateList2;
//    ignoreDateList2.append(ignoreDate2);
//    info2.setIgnoreTime(ignoreDateList2);
//    QString begin2 = "2020-12-11 12:24:36";
//    QDateTime begindate2 = QDateTime::fromString(begin2, "yyyy-MM-dd hh:mm:ss");
//    QString end2 = "2020-12-13 12:24:36";
//    QDateTime enddate2 = QDateTime::fromString(end2, "yyyy-MM-dd hh:mm:ss");
//    info2.setBeginDateTime(begindate2);
//    info2.setEndDateTime(enddate2);

//    ScheduleDataInfo info3;
//    info3.setID(1);
//    info3.setType(2);
//    info3.setAllDay(true);
//    info3.setRecurID(2);
//    info3.setTitleName("测试3");
//    QString strDate3 = "2020-12-22 12:24:36";
//    QDateTime ignoreDate3 = QDateTime::fromString(strDate3, "yyyy-MM-dd hh:mm:ss");
//    QVector<QDateTime> ignoreDateList3;
//    ignoreDateList3.append(ignoreDate3);
//    info3.setIgnoreTime(ignoreDateList3);
//    QString begin3 = "2020-12-21 12:24:36";
//    QDateTime begindate3 = QDateTime::fromString(begin3, "yyyy-MM-dd hh:mm:ss");
//    QString end3 = "2020-12-23 12:24:36";
//    QDateTime enddate3 = QDateTime::fromString(end3, "yyyy-MM-dd hh:mm:ss");
//    info3.setBeginDateTime(begindate3);
//    info3.setEndDateTime(enddate3);

//    QVector<ScheduleDataInfo> infoList;
//    infoList.append(info1);
//    infoList.append(info2);
//    infoList.append(info3);

//    return infoList;
//}

////void CAllDayEventWeekView::setDayData(const QVector<QVector<ScheduleDataInfo>> &vlistData)
//TEST_F(test_alldayeventview, setDayData)
//{
//    QVector<QVector<ScheduleDataInfo>> vlistData;

//    QVector<ScheduleDataInfo> infoList1;
//    infoList1.append(getTestScheduleDataInfo().at(0));
//    infoList1.append(getTestScheduleDataInfo().at(1));

//    QVector<ScheduleDataInfo> infoList2;
//    infoList2.append(getTestScheduleDataInfo().at(2));

//    vlistData.append(infoList1);
//    vlistData.append(infoList2);

//    cAllDayEventWeekView->setDayData(vlistData);
//}


////void CAllDayEventWeekView::setInfo(const QVector<ScheduleDataInfo> &info)
//TEST_F(test_alldayeventview, setInfo)
//{
//    cAllDayEventWeekView->setInfo(getTestScheduleDataInfo());
//}

////void CAllDayEventWeekView::slotDoubleEvent()
//TEST_F(test_alldayeventview, slotDoubleEvent)
//{
//    cAllDayEventWeekView->slotDoubleEvent();
//}

////void CAllDayEventWeekView::createItemWidget(int index, bool average)
//TEST_F(test_alldayeventview, createItemWidget)
//{
//    QVector<QVector<ScheduleDataInfo>> vlistData;

//    QVector<ScheduleDataInfo> infoList1;
//    infoList1.append(getTestScheduleDataInfo().at(0));
//    infoList1.append(getTestScheduleDataInfo().at(1));

//    QVector<ScheduleDataInfo> infoList2;
//    infoList2.append(getTestScheduleDataInfo().at(2));

//    vlistData.append(infoList1);
//    vlistData.append(infoList2);

//    cAllDayEventWeekView->setDayData(vlistData);

//    int w = 1000;
//    int h = 900;
//    QDate begindate(2020, 12, 01);
//    QDate enddate(2020, 12, 21);
//    int rightmagin = 2;
//    cAllDayEventWeekView->setRange(w, h, begindate, enddate, rightmagin);

//    int index = 0;
//    bool average = true;
//    cAllDayEventWeekView->createItemWidget(index, average);
//    cAllDayEventWeekView->updateHeight();
//    //setSelectSearchSchedule
//    cAllDayEventWeekView->setSelectSearchSchedule(getTestScheduleDataInfo().at(1));
//}

////void CAllDayEventWeekView::updateItemHeightByFontSize()
//TEST_F(test_alldayeventview, updateItemHeightByFontSize)
//{
//    cAllDayEventWeekView->updateItemHeightByFontSize();
//}


////void CAllDayEventWeekView::upDateInfoShow(const DragStatus &status, const ScheduleDataInfo &info)
//TEST_F(test_alldayeventview, upDateInfoShow)
//{
//    cAllDayEventWeekView->setInfo(getTestScheduleDataInfo());
//    cAllDayEventWeekView->upDateInfoShow(DragInfoGraphicsView::DragStatus::ChangeEnd, getTestScheduleDataInfo().at(1));
//    cAllDayEventWeekView->upDateInfoShow(DragInfoGraphicsView::DragStatus::ChangeBegin, getTestScheduleDataInfo().at(1));
//    cAllDayEventWeekView->upDateInfoShow(DragInfoGraphicsView::DragStatus::ChangeWhole, getTestScheduleDataInfo().at(1));
//    cAllDayEventWeekView->upDateInfoShow(DragInfoGraphicsView::DragStatus::IsCreate, getTestScheduleDataInfo().at(1));
//}

////
//TEST_F(test_alldayeventview, getPixmap)
//{
//    cAllDayEventWeekView->setFixedSize(500, 800);
//    QPixmap pixmap(cAllDayEventWeekView->size());
//    pixmap = cAllDayEventWeekView->grab();
//}

////
//TEST_F(test_alldayeventview, eventTest)
//{
//    QTest::keyEvent(QTest::Press, cAllDayEventWeekView, Qt::Key::Key_Enter);
//    cAllDayEventWeekView->setFixedSize(500, 100);
//    QTest::mouseDClick(cAllDayEventWeekView, Qt::LeftButton, Qt::NoModifier, QPoint(200, 50));
//}

////slotCreate
//TEST_F(test_alldayeventview, slotCreate)
//{
//    calendarDDialogExecReturn = 1;
//    Stub stub;
//    calendarDDialogExecStub(stub);
//    cAllDayEventWeekView->slotCreate(QDateTime::currentDateTime());
//}

////getDragScheduleInfoBeginTime
//TEST_F(test_alldayeventview, getDragScheduleInfoBeginTime)
//{
//    cAllDayEventWeekView->getDragScheduleInfoBeginTime(QDateTime::currentDateTime());
//}

////changeEvent
//TEST_F(test_alldayeventview, changeEvent)
//{
//    QEvent event(QEvent::FontChange);
//    QApplication::sendEvent(cAllDayEventWeekView, &event);
//}

////mousePressEvent
//TEST_F(test_alldayeventview, mousePressEvent)
//{
//    QMouseEvent event(QEvent::MouseButtonPress, QPointF(32, 13), QPointF(646, 438), QPointF(646, 438),
//                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
//    QApplication::sendEvent(cAllDayEventWeekView->viewport(), &event);
//}

//TEST_F(test_alldayeventview, JudgeIsCreate)
//{
//    QPointF point(32, 13);
//    cAllDayEventWeekView->JudgeIsCreate(point);
//}

//QAction *stub_exec(const QPoint &pos, QAction *at = nullptr)
//{
//    Q_UNUSED(pos)
//    Q_UNUSED(at)
//    return nullptr;
//}

////RightClickToCreate
//TEST_F(test_alldayeventview, RightClickToCreate)
//{
//    Stub stub;
//    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *)) ADDR(QMenu, exec), stub_exec);
//    cAllDayEventWeekView->RightClickToCreate(nullptr, QPoint(30, 50));
//}

////MoveInfoProcess
//TEST_F(test_alldayeventview, MoveInfoProcess)
//{
//    ScheduleDataInfo info;
//    QDateTime currentTime = QDateTime::currentDateTime();
//    info.setBeginDateTime(currentTime);
//    info.setEndDateTime(currentTime.addDays(1));
//    info.setAllDay(true);
//    cAllDayEventWeekView->MoveInfoProcess(info, QPointF(0, 0));
//    info.setAllDay(false);
//    cAllDayEventWeekView->MoveInfoProcess(info, QPointF(0, 0));
//}

////getDragScheduleInfoEndTime
//TEST_F(test_alldayeventview, getDragScheduleInfoEndTime)
//{
//    cAllDayEventWeekView->getDragScheduleInfoEndTime(QDateTime::currentDateTime());
//}

////slotUpdateScene
//TEST_F(test_alldayeventview, slotUpdateScene)
//{
//    cAllDayEventWeekView->slotUpdateScene();
//}

//TEST_F(test_alldayeventview, updateInfo)
//{
//    cAllDayEventWeekView->updateInfo();
//    cAllDayEventWeekView->m_DragStatus = DragInfoGraphicsView::IsCreate;
//    cAllDayEventWeekView->updateInfo();
//}

////mouseDoubleClickEvent
//TEST_F(test_alldayeventview, mouseDoubleClickEvent)
//{
//    Stub stub;
//    calendarDDialogExecStub(stub);
//    QTest::mouseDClick(cAllDayEventWeekView->viewport(), Qt::LeftButton);
//}

////
//TEST_F(test_alldayeventview, setSceneRect)
//{
//    cAllDayEventWeekView->setSceneRect(20, 20, 1000, 1500);
//    cAllDayEventWeekView->updateBackgroundShowItem();
//}

////slotPosOnView
//TEST_F(test_alldayeventview, slotPosOnView)
//{
//    cAllDayEventWeekView->slotPosOnView(1);
//}

////mouseReleaseEvent
//TEST_F(test_alldayeventview, mouseReleaseEvent)
//{
//    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(32, 13), QPointF(646, 438), QPointF(646, 438),
//                      Qt::RightButton, Qt::RightButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
//    QApplication::sendEvent(cAllDayEventWeekView->viewport(), &event);
//}

////mouseReleaseEvent
//TEST_F(test_alldayeventview, mouseReleaseEvent1)
//{
//    cAllDayEventWeekView->m_TouchBeginTime = QDateTime::currentDateTime().toMSecsSinceEpoch() - 10;
//    QMouseEvent event(QEvent::MouseButtonRelease, QPointF(32, 13), QPointF(646, 438), QPointF(646, 438),
//                      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
//    QApplication::sendEvent(cAllDayEventWeekView->viewport(), &event);
//}

////mouseMoveEvent
//TEST_F(test_alldayeventview, mouseMoveEvent)
//{
//    cAllDayEventWeekView->m_touchState = DragInfoGraphicsView::TS_PRESS;
//    QTest::mouseMove(cAllDayEventWeekView->viewport());
//}

////wheelEvent
//TEST_F(test_alldayeventview, wheelEvent)
//{
//    QEvent event(QEvent::Wheel);
//    QApplication::sendEvent(cAllDayEventWeekView->viewport(), &event);
//}

////contextMenuEvent
//TEST_F(test_alldayeventview, contextMenuEvent)
//{
//    //    QEvent event(QEvent::ContextMenu);
//    QContextMenuEvent event(QContextMenuEvent::Mouse, QPoint(20, 20), QPoint(120, 120), Qt::NoModifier);
//    Stub stub;
//    stub.set((QAction * (QMenu::*)(const QPoint &, QAction *)) ADDR(QMenu, exec), stub_exec);
//    calendarDDialogExecStub(stub);
//    QApplication::sendEvent(cAllDayEventWeekView->viewport(), &event);
//}
