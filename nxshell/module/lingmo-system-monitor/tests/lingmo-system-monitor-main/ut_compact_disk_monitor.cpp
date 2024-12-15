// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "compact_disk_monitor.h"
#include "system/device_db.h"
#include "system/diskio_info.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

//qt
#include <DApplication>
#include <QMouseEvent>

using namespace core::system;

/***************************************STUB begin*********************************************/

//double stub_cpuAllPercent_noNum()
//{
//    return 1;
//}

/***************************************STUB end**********************************************/


class UT_CompactDiskMonitor: public ::testing::Test
{
public:
    UT_CompactDiskMonitor() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CompactDiskMonitor();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    CompactDiskMonitor *m_tester;
};

TEST_F(UT_CompactDiskMonitor, initTest)
{
}

TEST_F(UT_CompactDiskMonitor, test_updateStatus)
{
    m_tester->readSpeeds->append(0.1);
    m_tester->readSpeeds->append(0.2);
    m_tester->readSpeeds->append(0.3);
    m_tester->readSpeeds->append(0.4);
    m_tester->readSpeeds->append(0.5);
    m_tester->updateStatus();
}

TEST_F(UT_CompactDiskMonitor, test_getPainterPathByData)
{
    m_tester->readSpeeds->append(0.1);
    m_tester->readSpeeds->append(0.2);
    m_tester->readSpeeds->append(0.3);
    m_tester->readSpeeds->append(0.4);
    m_tester->readSpeeds->append(0.5);

    DeviceDB::instance()->diskIoInfo()->diskIoReadBps();
    m_tester->m_readBps = DeviceDB::instance()->diskIoInfo()->diskIoReadBps();
    m_tester->readSpeeds->append(m_tester->m_readBps);
    if (m_tester->readSpeeds->size() > 30 + 1) {
        m_tester->readSpeeds->pop_front();
    }
    double readMaxHeight = *std::max_element(m_tester->readSpeeds->begin(), m_tester->readSpeeds->end()) * 1.1;
    double writeMaxHeight = *std::max_element(m_tester->writeSpeeds->begin(), m_tester->writeSpeeds->end()) * 1.1;
    double maxHeight = qMax(readMaxHeight, writeMaxHeight);
    QPainterPath tmpReadpath;
    m_tester->getPainterPathByData(m_tester->readSpeeds, tmpReadpath, maxHeight);
}

TEST_F(UT_CompactDiskMonitor, test_paintEvent)
{
    EXPECT_FALSE(m_tester->grab().isNull());
}

TEST_F(UT_CompactDiskMonitor, test_changeFont)
{
    QFont font;
    font.setWeight(QFont::Medium);
    m_tester->changeFont(font);
}

TEST_F(UT_CompactDiskMonitor, test_mouseReleaseEvent)
{
    QMouseEvent *event = new QMouseEvent(QEvent::Type::Move, QPointF(0, 0), Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ShiftModifier);
    m_tester->mouseReleaseEvent(event);
    delete event;
}


TEST_F(UT_CompactDiskMonitor, test_mouseMoveEvent)
{
    QMouseEvent *event = new QMouseEvent(QEvent::Type::Move, QPointF(0, 0), Qt::MouseButton::LeftButton, Qt::MouseButton::LeftButton, Qt::KeyboardModifier::ShiftModifier);
    m_tester->mouseMoveEvent(event);
    delete event;
}


