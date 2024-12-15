// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QDebug>
#include <QPainter>
#include <QApplication>
#include <QTimer>
#include <QTest>
#include <QMainWindow>
#include <QHBoxLayout>
#include  <QFont>

#include "stub.h"
#include "addr_pri.h"

#include "../../src/waylandrecord/writeframethread.h"
#include "waylandrecord/recordadmin.h"


using namespace testing;

class WriteFrameThreadTest: public testing::Test
{

public:
    Stub stub;
    WriteFrameThread *m_writeFrameThread;
    WaylandIntegration::WaylandIntegrationPrivate *m_context;
    virtual void SetUp() override
    {
        std::cout << "start UtilsTest" << std::endl;
        avlibInterface::initFunctions();
        m_context = new  WaylandIntegration::WaylandIntegrationPrivate();
        m_writeFrameThread = new WriteFrameThread(m_context);

    }

    virtual void TearDown() override
    {
        delete m_writeFrameThread;
        delete m_context;
        avlibInterface::unloadFunctions();
        std::cout << "end UtilsTest" << std::endl;
    }
};

static int g_first1 = 0;
bool isWriteVideo_stub()
{
    g_first1++;
    if (g_first1 == 1) {
        return true;
    } else {
        g_first1 = 0;
        return false;
    }
}
bool getFrame_stub(WaylandIntegration::WaylandIntegrationPrivate::waylandFrame &frame)
{
    return true;
}
int writeVideoFrame_stub(WaylandIntegration::WaylandIntegrationPrivate::waylandFrame &frame)
{
    return 1;
}
ACCESS_PRIVATE_FIELD(WriteFrameThread, WaylandIntegration::WaylandIntegrationPrivate *, m_context);
TEST_F(WriteFrameThreadTest, run)
{
    stub.set(ADDR(WaylandIntegration::WaylandIntegrationPrivate, isWriteVideo), isWriteVideo_stub);
    stub.set(ADDR(WaylandIntegration::WaylandIntegrationPrivate, getFrame), getFrame_stub);
    stub.set(ADDR(CAVOutputStream, writeVideoFrame), writeVideoFrame_stub);

    auto &WriteFrameThread_m_context = access_private_field::WriteFrameThreadm_context(*m_writeFrameThread);
    WriteFrameThread_m_context = new  WaylandIntegration::WaylandIntegrationPrivate();
    QStringList arguments;
    arguments << QString("%1").arg(3);
    arguments << QString("%1").arg(1000) << QString("%1").arg(500);
    arguments << QString("%1").arg(0) << QString("%1").arg(0);
    arguments << QString("%1").arg(24);
    arguments << QString("%1").arg("tmp/test.mkv");
    arguments << QString("%1").arg(4);
    WriteFrameThread_m_context->m_recordAdmin = new RecordAdmin(arguments, WriteFrameThread_m_context);

    m_writeFrameThread->run();

    stub.reset(ADDR(WaylandIntegration::WaylandIntegrationPrivate, isWriteVideo));
    stub.reset(ADDR(WaylandIntegration::WaylandIntegrationPrivate, getFrame));
    stub.reset(ADDR(CAVOutputStream, writeVideoFrame));

//    delete WriteFrameThread_m_context->m_recordAdmin;
    delete WriteFrameThread_m_context;
}

TEST_F(WriteFrameThreadTest, bWriteFrame)
{
    m_writeFrameThread->bWriteFrame();

}
TEST_F(WriteFrameThreadTest, setBWriteFrame)
{
    m_writeFrameThread->setBWriteFrame(false);

}
