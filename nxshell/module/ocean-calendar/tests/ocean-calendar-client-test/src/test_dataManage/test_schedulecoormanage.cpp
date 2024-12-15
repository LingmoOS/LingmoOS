// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

//#include "test_schedulecoormanage.h"
//#include <QDebug>

//test_schedulecoormanage::test_schedulecoormanage()
//{
//    cScheduleCoorManage = new CScheduleCoorManage();
//}

//test_schedulecoormanage::~test_schedulecoormanage()
//{
//    delete cScheduleCoorManage;
//}

////void CScheduleCoorManage::setRange(int w, int h, QDate begindate, QDate enddate, int rightmagin)
//TEST_F(test_schedulecoormanage, setRange)
//{
//    int w = 100;
//    int h = 100;
//    QDate begindate(2020, 12, 01);
//    QDate enddate(2020, 12, 31);
//    int rightmagin = 1;
//    cScheduleCoorManage->setRange(w, h, begindate, enddate, rightmagin);
//}

////void CScheduleCoorManage::setDateRange(QDate begindate, QDate enddate)
//TEST_F(test_schedulecoormanage, setDateRange)
//{
//    QDate begindate(2020, 12, 01);
//    QDate enddate(2020, 12, 21);
//    cScheduleCoorManage->setDateRange(begindate, enddate);

//    cScheduleCoorManage->setDateRange(enddate, begindate);

//    int w = 100;
//    int h = 100;
//    QDate m_begindate(2020, 12, 11);
//    QDate m_enddate(2020, 12, 31);
//    int rightmagin = 1;
//    cScheduleCoorManage->setRange(w, h, m_begindate, m_enddate, rightmagin);
//    cScheduleCoorManage->setDateRange(begindate, enddate);
//}

///**
// * @brief test_schedulecoormanage, getDrawRegion
// * (QDateTime begintime, QDateTime endtime)
// * (QDateTime begintime, QDateTime endtime, int index, int coount)
// * (QDate date, QDateTime begintime, QDateTime endtime, int index, int coount, int maxnum, int type)
// */
//TEST_F(test_schedulecoormanage, getDrawRegion)
//{
//    QString begin = "2020-12-01 12:24:36";
//    QDateTime begindate = QDateTime::fromString(begin, "yyyy-MM-dd hh:mm:ss");
//    QString end = "2020-12-21 12:24:36";
//    QDateTime enddate = QDateTime::fromString(end, "yyyy-MM-dd hh:mm:ss");

//    cScheduleCoorManage->getDrawRegion(begindate, enddate);
//    cScheduleCoorManage->getDrawRegion(enddate, begindate);

//    int index = 2;
//    int coount = 1;
//    cScheduleCoorManage->getDrawRegion(begindate, enddate, index, coount);
//    cScheduleCoorManage->getDrawRegion(enddate, begindate, index, coount);

//    QDate date(2020, 12, 31);
//    int maxnum = 3;
//    int type = 0;
//    cScheduleCoorManage->getDrawRegion(date, begindate, enddate, index, coount, maxnum, type);
//    cScheduleCoorManage->getDrawRegion(date, enddate, begindate, index, coount, maxnum, type);
//    int w = 100;
//    int h = 100;
//    QDate m_begindate(2020, 11, 11);
//    QDate m_enddate(2020, 12, 31);
//    int rightmagin = 1;
//    cScheduleCoorManage->setRange(w, h, m_begindate, m_enddate, rightmagin);
//    cScheduleCoorManage->getDrawRegion(begindate, enddate);
//    cScheduleCoorManage->getDrawRegion(begindate, enddate, index, coount);

//    coount = 5;
//    cScheduleCoorManage->getDrawRegion(date, begindate, enddate, index, coount, maxnum, type);

//    index = 5;
//    cScheduleCoorManage->getDrawRegion(date, begindate, enddate, index, coount, maxnum, type);
//}

////QRectF CScheduleCoorManage::getDrawRegionF(QDateTime begintime, QDateTime endtime)
//TEST_F(test_schedulecoormanage, getDrawRegionF)
//{
//    QString begin = "2020-12-01 12:24:36";
//    QDateTime begindate = QDateTime::fromString(begin, "yyyy-MM-dd hh:mm:ss");
//    QString end = "2020-12-21 12:24:36";
//    QDateTime enddate = QDateTime::fromString(end, "yyyy-MM-dd hh:mm:ss");

//    cScheduleCoorManage->getDrawRegionF(begindate, enddate);
//    cScheduleCoorManage->getDrawRegionF(enddate, begindate);

//    int w = 100;
//    int h = 100;
//    QDate m_begindate(2020, 11, 11);
//    QDate m_enddate(2020, 12, 31);
//    int rightmagin = 1;
//    cScheduleCoorManage->setRange(w, h, m_begindate, m_enddate, rightmagin);
//    cScheduleCoorManage->getDrawRegionF(begindate, enddate);
//}

////QRectF CScheduleCoorManage::getAllDayDrawRegion(QDate begin, QDate end)
//TEST_F(test_schedulecoormanage, getAllDayDrawRegion)
//{
//    QDate begindate(2020, 12, 01);
//    QDate enddate(2020, 12, 31);
//    cScheduleCoorManage->getAllDayDrawRegion(begindate, enddate);
//    cScheduleCoorManage->getAllDayDrawRegion(enddate, begindate);
//}

////QDateTime CScheduleCoorManage::getDate(QPointF pos)
//TEST_F(test_schedulecoormanage, getDate)
//{
//    QPointF pos(0, 0);
//    QDateTime dateTime = cScheduleCoorManage->getDate(pos);
//    qCInfo(CommonLogger) << dateTime;
//    QPointF pos1(-1, 0);
//    QDateTime dateTime1 = cScheduleCoorManage->getDate(pos1);
//}

////QDate CScheduleCoorManage::getsDate(QPointF pos)
//TEST_F(test_schedulecoormanage, getsDate)
//{
//    QPointF pos(0, 0);
//    QDate date = cScheduleCoorManage->getsDate(pos);
//    qCInfo(CommonLogger) << date;
//    QPointF pos1(-1, 0);
//    cScheduleCoorManage->getsDate(pos1);
//}

////float CScheduleCoorManage::getHeight(const QTime &time)
//TEST_F(test_schedulecoormanage, getHeight)
//{
//    QTime time(18, 8, 9, 30);
//    assert(0 <= cScheduleCoorManage->getHeight(time));
//}

////QDate getBegindate()
//TEST_F(test_schedulecoormanage, getBegindate)
//{
//    int w = 100;
//    int h = 100;
//    QDate m_begindate(2020, 11, 11);
//    QDate m_enddate(2020, 12, 31);
//    int rightmagin = 1;
//    cScheduleCoorManage->setRange(w, h, m_begindate, m_enddate, rightmagin);
//    assert(m_begindate == cScheduleCoorManage->getBegindate());
//}
