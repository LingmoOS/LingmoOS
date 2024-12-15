// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>


#include "stub.h"
#include "addr_pri.h"
#include "../../src/utils/waylandscrollmonitor.h"
using namespace testing;

class WaylandScrollMonitorTest: public testing::Test
{

public:
    Stub stub;
    virtual void SetUp() override
    {
        std::cout << "start WaylandScrollMonitorTest" << std::endl;
    }

    virtual void TearDown() override
    {
        std::cout << "end WaylandScrollMonitorTest" << std::endl;
    }
};

void common_stub()
{

}
void setup_stub(KWayland::Client::ConnectionThread *connection)
{
    Q_UNUSED(connection);
}
void create_stub(KWayland::Client::ConnectionThread connection)
{
    Q_UNUSED(connection);
}
void setEventQueue_stub(KWayland::Client::EventQueue *queue)
{
    Q_UNUSED(queue);
}
void setupFakeinput_stub(quint32 name, quint32 version)
{
    Q_UNUSED(name);
    Q_UNUSED(version);
}
org_kde_kwin_fake_input *bindFakeInput_stub(uint32_t name, uint32_t version)
{
    Q_UNUSED(name);
    Q_UNUSED(version);

    return nullptr;
}

void authenticate_stub(const QString &applicationName, const QString &reason)
{
    Q_UNUSED(applicationName);
    Q_UNUSED(reason);
}
ACCESS_PRIVATE_FUN(WaylandScrollMonitor, void(), initWaylandScrollThread);
ACCESS_PRIVATE_FUN(WaylandScrollMonitor, void(), releaseWaylandScrollThread);
ACCESS_PRIVATE_FIELD(WaylandScrollMonitor, KWayland::Client::ConnectionThread *, m_connection);
ACCESS_PRIVATE_FIELD(WaylandScrollMonitor, KWayland::Client::EventQueue *, m_queue);
ACCESS_PRIVATE_FIELD(WaylandScrollMonitor, KWayland::Client::Registry *, m_registry);
ACCESS_PRIVATE_FIELD(WaylandScrollMonitor, KWayland::Client::FakeInput *, m_fakeinput);
ACCESS_PRIVATE_FUN(WaylandScrollMonitor, void(), setupRegistry);
ACCESS_PRIVATE_FUN(WaylandScrollMonitor, void(quint32, quint32), setupFakeinput);
TEST_F(WaylandScrollMonitorTest, slotManualScroll)
{
    stub.set(get_private_fun::WaylandScrollMonitorinitWaylandScrollThread(), common_stub);
    stub.set(get_private_fun::WaylandScrollMonitorreleaseWaylandScrollThread(), common_stub);

    WaylandScrollMonitor *waylandScrollMonitor;
    waylandScrollMonitor = new WaylandScrollMonitor();

    waylandScrollMonitor->slotManualScroll(1.0);


    delete waylandScrollMonitor;
    stub.reset(get_private_fun::WaylandScrollMonitorreleaseWaylandScrollThread());
    stub.reset(get_private_fun::WaylandScrollMonitorinitWaylandScrollThread());
}


TEST_F(WaylandScrollMonitorTest, setupRegistry)
{
    if (QSysInfo::currentCpuArchitecture().startsWith("arm"))
        return;
    stub.set(get_private_fun::WaylandScrollMonitorinitWaylandScrollThread(), common_stub);
    stub.set(get_private_fun::WaylandScrollMonitorreleaseWaylandScrollThread(), common_stub);
    WaylandScrollMonitor *waylandScrollMonitor;
    waylandScrollMonitor = new WaylandScrollMonitor();

    stub.set((void(KWayland::Client::EventQueue::*)(KWayland::Client::ConnectionThread *))ADDR(KWayland::Client::EventQueue, setup), setup_stub);
    stub.set((void(KWayland::Client::Registry::*)(KWayland::Client::ConnectionThread *))ADDR(KWayland::Client::Registry, create), create_stub);
    stub.set((void(KWayland::Client::Registry::*)(KWayland::Client::EventQueue *))ADDR(KWayland::Client::Registry, setEventQueue), setEventQueue_stub);
    stub.set(ADDR(KWayland::Client::Registry, setup), common_stub);
    stub.set(get_private_fun::WaylandScrollMonitorsetupFakeinput(), setupFakeinput_stub);

    call_private_fun::WaylandScrollMonitorsetupRegistry(*waylandScrollMonitor);

    delete waylandScrollMonitor;
    stub.reset(get_private_fun::WaylandScrollMonitorreleaseWaylandScrollThread());
    stub.reset(get_private_fun::WaylandScrollMonitorinitWaylandScrollThread());

    stub.reset((void(KWayland::Client::EventQueue::*)(KWayland::Client::ConnectionThread *))ADDR(KWayland::Client::EventQueue, setup));
    stub.reset((void(KWayland::Client::Registry::*)(KWayland::Client::ConnectionThread *))ADDR(KWayland::Client::Registry, create));
    stub.reset((void(KWayland::Client::Registry::*)(KWayland::Client::EventQueue *))ADDR(KWayland::Client::Registry, setEventQueue));
    stub.reset(ADDR(KWayland::Client::Registry, setup));
    stub.reset(get_private_fun::WaylandScrollMonitorsetupFakeinput());
}

TEST_F(WaylandScrollMonitorTest, setupFakeinput)
{
    stub.set(get_private_fun::WaylandScrollMonitorinitWaylandScrollThread(), common_stub);
    stub.set(get_private_fun::WaylandScrollMonitorreleaseWaylandScrollThread(), common_stub);
    WaylandScrollMonitor *waylandScrollMonitor;
    waylandScrollMonitor = new WaylandScrollMonitor();

    access_private_field::WaylandScrollMonitorm_registry(*waylandScrollMonitor)  = new KWayland::Client::Registry();

    stub.set(ADDR(KWayland::Client::FakeInput, setup), common_stub);
    stub.set(ADDR(KWayland::Client::FakeInput, authenticate), authenticate_stub);
    stub.set(ADDR(KWayland::Client::Registry, bindFakeInput), bindFakeInput_stub);

    call_private_fun::WaylandScrollMonitorsetupFakeinput(*waylandScrollMonitor, 1, 1);

    delete access_private_field::WaylandScrollMonitorm_registry(*waylandScrollMonitor);
    delete waylandScrollMonitor;
    stub.reset(get_private_fun::WaylandScrollMonitorreleaseWaylandScrollThread());
    stub.reset(get_private_fun::WaylandScrollMonitorinitWaylandScrollThread());

    stub.reset(ADDR(KWayland::Client::FakeInput, setup));
    stub.reset(ADDR(KWayland::Client::FakeInput, authenticate));

}

TEST_F(WaylandScrollMonitorTest, initWaylandScrollThread)
{
    stub.set(get_private_fun::WaylandScrollMonitorinitWaylandScrollThread(), common_stub);
    stub.set(get_private_fun::WaylandScrollMonitorreleaseWaylandScrollThread(), common_stub);
    WaylandScrollMonitor *waylandScrollMonitor;
    waylandScrollMonitor = new WaylandScrollMonitor();

    stub.set(get_private_fun::WaylandScrollMonitorsetupRegistry(), common_stub);
    stub.set(ADDR(KWayland::Client::ConnectionThread, initConnection), common_stub);

    call_private_fun::WaylandScrollMonitorinitWaylandScrollThread(*waylandScrollMonitor);

    delete waylandScrollMonitor;
    stub.reset(get_private_fun::WaylandScrollMonitorreleaseWaylandScrollThread());
    stub.reset(get_private_fun::WaylandScrollMonitorinitWaylandScrollThread());

    stub.reset(get_private_fun::WaylandScrollMonitorsetupRegistry());
    stub.reset(ADDR(KWayland::Client::ConnectionThread, initConnection));
}


TEST_F(WaylandScrollMonitorTest, releaseWaylandScrollThread)
{
    /*
    stub.set(get_private_fun::WaylandScrollMonitorinitWaylandScrollThread(), common_stub);
    WaylandScrollMonitor *waylandScrollMonitor;
    waylandScrollMonitor = new WaylandScrollMonitor();
    stub.reset(get_private_fun::WaylandScrollMonitorinitWaylandScrollThread());

    access_private_field::WaylandScrollMonitorm_connection(*waylandScrollMonitor)  = new KWayland::Client::ConnectionThread();
    access_private_field::WaylandScrollMonitorm_queue(*waylandScrollMonitor)  = new KWayland::Client::EventQueue();
    access_private_field::WaylandScrollMonitorm_registry(*waylandScrollMonitor)  = new KWayland::Client::Registry();
    access_private_field::WaylandScrollMonitorm_fakeinput(*waylandScrollMonitor)  = new KWayland::Client::FakeInput();


    call_private_fun::WaylandScrollMonitorreleaseWaylandScrollThread(*waylandScrollMonitor);

//    delete access_private_field::WaylandScrollMonitorm_connection(*waylandScrollMonitor) ;
//    delete access_private_field::WaylandScrollMonitorm_queue(*waylandScrollMonitor);
//    delete access_private_field::WaylandScrollMonitorm_registry(*waylandScrollMonitor);
//    delete access_private_field::WaylandScrollMonitorm_fakeinput(*waylandScrollMonitor);

    stub.set(get_private_fun::WaylandScrollMonitorreleaseWaylandScrollThread(), common_stub);
    delete waylandScrollMonitor;
    stub.reset(get_private_fun::WaylandScrollMonitorreleaseWaylandScrollThread());
    */

}
