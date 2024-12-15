// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QDebug>
#include <QPainter>
#include <QApplication>
#include <QTimer>
#include <QString>
#include <QTest>
#include <QMainWindow>
#include <QHBoxLayout>
#include  <QFont>
#include <QScreen>
#include <QDesktopWidget>

#include "stub.h"
#include "addr_pri.h"


#include "../../src/waylandrecord/waylandintegration.h"
#include "../../src/waylandrecord/waylandintegration_p.h"
#include "../../src/waylandrecord/recordadmin.h"
#include "../../src/waylandrecord/writeframethread.h"

// KWayland
#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/remote_access.h>
using namespace testing;
using namespace WaylandIntegration;
class WaylandIntegrationPrivateTest: public testing::Test
{
public:
    Stub stub;
    WaylandIntegrationPrivate *m_waylandIntegrationPrivate;
    virtual void SetUp() override
    {
        avlibInterface::initFunctions();
        m_waylandIntegrationPrivate = new WaylandIntegrationPrivate();
        std::cout << "start UtilsTest" << std::endl;
    }

    virtual void TearDown() override
    {
        std::cout << "end UtilsTest" << std::endl;
        delete m_waylandIntegrationPrivate;
        avlibInterface::unloadFunctions();
    }
};

void setupRegistry_stub()
{
    qDebug() << "替换wayland客户断链接安装注册";

}
ACCESS_PRIVATE_FUN(WaylandIntegrationPrivate, void(), setupRegistry);
ACCESS_PRIVATE_FIELD(WaylandIntegrationPrivate, QThread *, m_thread);
TEST_F(WaylandIntegrationPrivateTest, initWayland)
{
    auto WaylandIntegrationPrivate_setupRegistry = get_private_fun::WaylandIntegrationPrivatesetupRegistry();
    stub.set(WaylandIntegrationPrivate_setupRegistry, setupRegistry_stub);

    QStringList arguments;
    arguments << QString("%1").arg(3);
    arguments << QString("%1").arg(1000) << QString("%1").arg(500);
    arguments << QString("%1").arg(0) << QString("%1").arg(0);
    arguments << QString("%1").arg(24);
    arguments << QString("%1").arg("tmp/test.mkv");
    arguments << QString("%1").arg(4);
    qDebug() << arguments;


    m_waylandIntegrationPrivate->initWayland(arguments);
    access_private_field::WaylandIntegrationPrivatem_thread(*m_waylandIntegrationPrivate)->terminate();
    access_private_field::WaylandIntegrationPrivatem_thread(*m_waylandIntegrationPrivate)->wait();
    stub.reset(WaylandIntegrationPrivate_setupRegistry);
}

ACCESS_PRIVATE_FIELD(WaylandIntegrationPrivate, bool, m_eglInitialized);
TEST_F(WaylandIntegrationPrivateTest, isEGLInitialized)
{
    bool res = access_private_field::WaylandIntegrationPrivatem_eglInitialized(*m_waylandIntegrationPrivate);
    res = false;
    EXPECT_EQ(m_waylandIntegrationPrivate->isEGLInitialized(), false);
}

//void setup_stub(wl_output *output)
//{
//    qDebug() << "模拟注册输出";
//}
//wl_output *bindOutput_stub(uint32_t name, uint32_t version)
//{
//    wl_output *output = nullptr;
//    return output;
//}
//ACCESS_PRIVATE_FIELD(WaylandIntegrationPrivate, KWayland::Client::Registry *, m_registry);
//TEST_F(WaylandIntegrationPrivateTest, bindOutput)
//{
//    stub.set(ADDR(KWayland::Client::Registry, bindOutput), bindOutput_stub);

////    KWayland::Client::Registry *registry = access_private_field::WaylandIntegrationPrivatem_registry(*m_waylandIntegrationPrivate);
//    access_private_field::WaylandIntegrationPrivatem_registry(*m_waylandIntegrationPrivate)
//        = new KWayland::Client::Registry(m_waylandIntegrationPrivate);

//    m_waylandIntegrationPrivate->bindOutput(1, 1);

//    stub.reset(bindOutput_stub);
//}

ACCESS_PRIVATE_FIELD(WaylandIntegrationPrivate, bool, m_streamingEnabled);
ACCESS_PRIVATE_FIELD(WaylandIntegrationPrivate, KWayland::Client::RemoteAccessManager *, m_remoteAccessManager);
TEST_F(WaylandIntegrationPrivateTest, stopStreaming)
{
    access_private_field::WaylandIntegrationPrivatem_streamingEnabled(*m_waylandIntegrationPrivate) = true;
    access_private_field::WaylandIntegrationPrivatem_remoteAccessManager(*m_waylandIntegrationPrivate) = nullptr;
    m_waylandIntegrationPrivate->stopStreaming();
}

QPixmap grabEntireDesktop()
{
    QPixmap g_tempPixmap;
    QScreen *t_primaryScreen = QGuiApplication::primaryScreen();
    g_tempPixmap = t_primaryScreen->grabWindow(QApplication::desktop()->winId(), 0, 0, 1920, 1080);
    // 在多屏模式下, winId 不是0
    return g_tempPixmap;

}
void appendBuffer_stub(unsigned char *frame, int width, int height, int stride, int64_t time)
{
    Q_UNUSED(frame);
    Q_UNUSED(width);
    Q_UNUSED(height);
    Q_UNUSED(stride);
    Q_UNUSED(time);

}

//无法构造出KWayland::Client::RemoteBuffer 相关数据
//ACCESS_PRIVATE_FUN(WaylandIntegrationPrivate, void(const KWayland::Client::RemoteBuffer *rbuf), processBuffer);
//ACCESS_PRIVATE_FUN(WaylandIntegrationPrivate, void(unsigned char *frame, int width, int height, int stride, int64_t time), appendBuffer);
//TEST_F(WaylandIntegrationPrivateTest, processBuffer)
//{

//}

void addOutput_stub(quint32 name, quint32 version)
{
    Q_UNUSED(name);
    Q_UNUSED(version);
}

void removeOutput_stub(quint32 name)
{
    Q_UNUSED(name);

}
void create_stub1(KWayland::Client::ConnectionThread connection)
{
    Q_UNUSED(connection);
}
void setEventQueue_stub1(KWayland::Client::EventQueue *queue)
{
    Q_UNUSED(queue);
}
void setup_stub1()
{

}
void initEgl_stub()
{

}
ACCESS_PRIVATE_FUN(WaylandIntegrationPrivate, void(quint32 name, quint32 version), addOutput);
ACCESS_PRIVATE_FUN(WaylandIntegrationPrivate, void(quint32 name), removeOutput);
ACCESS_PRIVATE_FUN(WaylandIntegrationPrivate, void(), initEgl);
ACCESS_PRIVATE_FIELD(WaylandIntegrationPrivate, KWayland::Client::ConnectionThread *, m_connection);
TEST_F(WaylandIntegrationPrivateTest, setupRegistry)
{
    if (QSysInfo::currentCpuArchitecture().startsWith("arm"))
        return;
    stub.set((void(KWayland::Client::EventQueue::*)(KWayland::Client::ConnectionThread *))ADDR(KWayland::Client::EventQueue, setup), setup_stub1);

    auto WaylandIntegrationPrivate_initEgl = get_private_fun::WaylandIntegrationPrivateinitEgl();
    stub.set(WaylandIntegrationPrivate_initEgl, initEgl_stub);

    auto WaylandIntegrationPrivate_addOutput = get_private_fun::WaylandIntegrationPrivateaddOutput();
    stub.set(WaylandIntegrationPrivate_addOutput, addOutput_stub);

    auto WaylandIntegrationPrivate_removeOutput = get_private_fun::WaylandIntegrationPrivateremoveOutput();
    stub.set(WaylandIntegrationPrivate_removeOutput, removeOutput_stub);

    stub.set((void(KWayland::Client::Registry::*)(KWayland::Client::ConnectionThread *))ADDR(KWayland::Client::Registry, create), create_stub1);
    stub.set((void(KWayland::Client::Registry::*)(KWayland::Client::EventQueue *))ADDR(KWayland::Client::Registry, setEventQueue), setEventQueue_stub1);
    stub.set(ADDR(KWayland::Client::Registry, setup), setup_stub1);


    access_private_field::WaylandIntegrationPrivatem_connection(*m_waylandIntegrationPrivate) = new KWayland::Client::ConnectionThread;


    QStringList arguments;
    arguments << QString("%1").arg(3);
    arguments << QString("%1").arg(1000) << QString("%1").arg(500);
    arguments << QString("%1").arg(0) << QString("%1").arg(0);
    arguments << QString("%1").arg(24);
    arguments << QString("%1").arg("tmp/test.mkv");
    arguments << QString("%1").arg(4);
    qDebug() << arguments;
//    m_waylandIntegrationPrivate->initWayland(arguments);
    call_private_fun::WaylandIntegrationPrivatesetupRegistry(*m_waylandIntegrationPrivate);

    stub.reset(WaylandIntegrationPrivate_initEgl);
    stub.reset(WaylandIntegrationPrivate_addOutput);
    stub.reset(WaylandIntegrationPrivate_removeOutput);
    stub.reset((void(KWayland::Client::Registry::*)(KWayland::Client::ConnectionThread *))ADDR(KWayland::Client::Registry, create));
    stub.reset(ADDR(KWayland::Client::Registry, setup));
}

//ACCESS_PRIVATE_FUN(WaylandIntegrationPrivate, void(unsigned char *frame, int width, int height, int stride, int64_t time), appendBuffer);
//ACCESS_PRIVATE_FIELD(WaylandIntegrationPrivate, unsigned char *, m_ffmFrame);
//ACCESS_PRIVATE_FIELD(WaylandIntegrationPrivate, QList<unsigned char *>, m_freeList);
//ACCESS_PRIVATE_FIELD(WaylandIntegrationPrivate, QList<WaylandIntegrationPrivate::waylandFrame>, m_waylandList);
//ACCESS_PRIVATE_FIELD(WaylandIntegrationPrivate, int, m_bufferSize);
//ACCESS_PRIVATE_FIELD(WaylandIntegrationPrivate, bool, m_bInit);
//ACCESS_PRIVATE_FIELD(WaylandIntegrationPrivate, int, m_width);
//ACCESS_PRIVATE_FIELD(WaylandIntegrationPrivate, int, m_height);
//ACCESS_PRIVATE_FIELD(WaylandIntegrationPrivate, int, m_stride);
//TEST_F(WaylandIntegrationPrivateTest, appendBuffer)
//{

//    QImage image = grabEntireDesktop().toImage();
//    unsigned char *frame = image.bits();
//    int width = image.width();
//    int height = image.height();
//    int stride = image.bytesPerLine();
//    int size = height * stride;

//    access_private_field::WaylandIntegrationPrivatem_bufferSize(*m_waylandIntegrationPrivate) = 60;
//    access_private_field::WaylandIntegrationPrivatem_bInit(*m_waylandIntegrationPrivate) = false;
//    access_private_field::WaylandIntegrationPrivatem_width(*m_waylandIntegrationPrivate) = width;
//    access_private_field::WaylandIntegrationPrivatem_height(*m_waylandIntegrationPrivate) = height;
//    access_private_field::WaylandIntegrationPrivatem_stride(*m_waylandIntegrationPrivate) = stride;
//    access_private_field::WaylandIntegrationPrivatem_ffmFrame(*m_waylandIntegrationPrivate) = new unsigned char[static_cast<unsigned long>(size)];;
//    unsigned char *ch = nullptr;
//    for (int i = 0; i < 60; i++) {
//        access_private_field::WaylandIntegrationPrivatem_freeList(*m_waylandIntegrationPrivate).append(ch);
//        qDebug() << "创建内存空间";
//        //delete [] ch;
//    }
//    for (int i = 0; i < access_private_field::WaylandIntegrationPrivatem_freeList(*m_waylandIntegrationPrivate).size(); i++) {
//        access_private_field::WaylandIntegrationPrivatem_freeList(*m_waylandIntegrationPrivate)[i] = new unsigned char[static_cast<unsigned long>(size)];
//    }
//    call_private_fun::WaylandIntegrationPrivateappendBuffer(*m_waylandIntegrationPrivate, frame, width, height, stride, 11111111);

//    for (int i = 0; i < access_private_field::WaylandIntegrationPrivatem_freeList(*m_waylandIntegrationPrivate).size(); i++) {
//        if (access_private_field::WaylandIntegrationPrivatem_freeList(*m_waylandIntegrationPrivate)[i]) {
//            delete []access_private_field::WaylandIntegrationPrivatem_freeList(*m_waylandIntegrationPrivate)[i];
//        }
//    }
//    access_private_field::WaylandIntegrationPrivatem_freeList(*m_waylandIntegrationPrivate).clear();
//    access_private_field::WaylandIntegrationPrivatem_waylandList(*m_waylandIntegrationPrivate).clear();

//}

TEST_F(WaylandIntegrationPrivateTest, getFrame)
{
    WaylandIntegrationPrivate::waylandFrame frame;
    m_waylandIntegrationPrivate->getFrame(frame);

}

class WriteFrameThread;
ACCESS_PRIVATE_FIELD(WaylandIntegrationPrivate, RecordAdmin *, m_recordAdmin);
//TEST_F(WaylandIntegrationPrivateTest, isWriteVideo)
//{
//    QStringList arguments;
//    arguments << QString("%1").arg(3);
//    arguments << QString("%1").arg(1000) << QString("%1").arg(500);
//    arguments << QString("%1").arg(0) << QString("%1").arg(0);
//    arguments << QString("%1").arg(24);
//    arguments << QString("%1").arg("tmp/test.mkv");
//    arguments << QString("%1").arg(4);
//    qDebug() << arguments;


//    access_private_field::WaylandIntegrationPrivatem_recordAdmin(*m_waylandIntegrationPrivate) = new  RecordAdmin(arguments, m_waylandIntegrationPrivate);
//    //access_private_field::WaylandIntegrationPrivatem_recordAdmin(*m_waylandIntegrationPrivate)->m_writeFrameThread = new WriteFrameThread(m_waylandIntegrationPrivate);

//    m_waylandIntegrationPrivate->isWriteVideo();

//    //delete access_private_field::WaylandIntegrationPrivatem_recordAdmin(*m_waylandIntegrationPrivate)->m_writeFrameThread;

//}

TEST_F(WaylandIntegrationPrivateTest, bGetFrame)
{
    m_waylandIntegrationPrivate->bGetFrame();

}

ACCESS_PRIVATE_FIELD(WaylandIntegrationPrivate, bool, m_bGetFrame);
TEST_F(WaylandIntegrationPrivateTest, setBGetFrame)
{
    m_waylandIntegrationPrivate->setBGetFrame(false);
    EXPECT_EQ(false, access_private_field::WaylandIntegrationPrivatem_bGetFrame(*m_waylandIntegrationPrivate));
}

int stopStream_stub()
{
    qDebug() << __LINE__ << " stopStream_stub";
    return 1;
}

//TEST_F(WaylandIntegrationPrivateTest, stopVideoRecord)
//{
//    avlibInterface::initFunctions();
//    QStringList arguments;
//    arguments << QString("%1").arg(3);
//    arguments << QString("%1").arg(1000) << QString("%1").arg(500);
//    arguments << QString("%1").arg(0) << QString("%1").arg(0);
//    arguments << QString("%1").arg(24);
//    arguments << QString("%1").arg("tmp/test.mkv");
//    arguments << QString("%1").arg(4);
//    qDebug() << arguments;

//    access_private_field::WaylandIntegrationPrivatem_recordAdmin(*m_waylandIntegrationPrivate) = new  RecordAdmin(arguments, m_waylandIntegrationPrivate);
//    qDebug() << __LINE__ << " new recordAdmin";
//    stub.set(ADDR(RecordAdmin, stopStream), stopStream_stub());
//    qDebug() << __LINE__ << " new recordAdmin";
//    bool flag = m_waylandIntegrationPrivate->stopVideoRecord();
//    qDebug() << __LINE__ << " new recordAdmin";
//    EXPECT_EQ(true, flag);
//    stub.reset(ADDR(RecordAdmin, stopStream));
//    avlibInterface::unloadFunctions();

//}


TEST_F(WaylandIntegrationPrivateTest, screens)
{
    m_waylandIntegrationPrivate->screens();
}

//ACCESS_PRIVATE_FUN(WaylandIntegrationPrivate, void(quint32 name), removeOutput);
TEST_F(WaylandIntegrationPrivateTest, removeOutput)
{
    call_private_fun::WaylandIntegrationPrivateremoveOutput(*m_waylandIntegrationPrivate, 111);
}


