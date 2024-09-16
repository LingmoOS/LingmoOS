// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stub.h"
#include "addr_pri.h"
#include <QDebug>
#include "../../../src/dde-dock-plugins/recordtime/dbusservice.h"

//void stub_start(){}

namespace  {
class TestDBusService : public testing::Test
{

public:
    Stub stub;
    void SetUp() override
    {
        m_dBusService = new DBusService(nullptr);
//        stub.set(ADDR(DBusService,start), stub_start);
    }
    void TearDown() override
    {
        if (m_dBusService) {
            delete  m_dBusService;
            m_dBusService = nullptr;
        }
    }

public:
    DBusService *m_dBusService;
};
}

TEST_F(TestDBusService, onStart)
{
    m_dBusService->onStart();
}

TEST_F(TestDBusService, onStop)
{
    m_dBusService->onStop();
}

TEST_F(TestDBusService, onRecording)
{
    m_dBusService->onRecording();
}

TEST_F(TestDBusService, onPause)
{
    m_dBusService->onPause();
}
