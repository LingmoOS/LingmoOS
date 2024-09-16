// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "dbus/systemd1_manager_interface.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>


/***************************************STUB begin*********************************************/
QDBusMessage::MessageType stub_listUnitFiles_reply()
{
    return QDBusMessage::ReplyMessage;
}

QString stub_reply_signature()
{
    return "o";
}

QString stub_reply_signature_s()
{
    return "s";
}

QString stub_reply_signature_bass()
{
    return "ba(sss)";
}

QString stub_reply_signature_ass()
{
    return "a(sss)";
}
/***************************************STUB end**********************************************/



class UT_Systemd1ManagerInterface: public ::testing::Test
{
public:
    UT_Systemd1ManagerInterface() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static QDBusConnection con("popup");
        m_tester = new Systemd1ManagerInterface("com.deepin.systemMonitorPluginPopup", "/com/deepin/systemMonitorPluginPopup", con);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    Systemd1ManagerInterface *m_tester;
};

TEST_F(UT_Systemd1ManagerInterface, initTest)
{
}

TEST_F(UT_Systemd1ManagerInterface, test_ListUnitFiles_01)
{
    m_tester->ListUnitFiles();
}

TEST_F(UT_Systemd1ManagerInterface, test_ListUnitFiles_02)
{
    Stub stub;
    stub.set(ADDR(QDBusMessage, type), stub_listUnitFiles_reply);
    m_tester->ListUnitFiles();
}

TEST_F(UT_Systemd1ManagerInterface, test_ListUnits_01)
{
    m_tester->ListUnits();
}

TEST_F(UT_Systemd1ManagerInterface, test_ListUnits_02)
{
    Stub stub;
    stub.set(ADDR(QDBusMessage, type), stub_listUnitFiles_reply);
    m_tester->ListUnits();
}

TEST_F(UT_Systemd1ManagerInterface, test_GetUnit_01)
{
    m_tester->GetUnit("com.deepin.systemMonitorPluginPopup");
}

TEST_F(UT_Systemd1ManagerInterface, test_GetUnit_02)
{
    Stub stub;
    stub.set(ADDR(QDBusMessage, type), stub_listUnitFiles_reply);
    stub.set(ADDR(QDBusMessage, signature), stub_reply_signature);
    m_tester->GetUnit("com.deepin.systemMonitorPluginPopup");
}

TEST_F(UT_Systemd1ManagerInterface, test_StartUnit_01)
{
    m_tester->StartUnit("com.deepin.systemMonitorPluginPopup", "1");
}

TEST_F(UT_Systemd1ManagerInterface, test_StartUnit_02)
{
    Stub stub;
    stub.set(ADDR(QDBusMessage, type), stub_listUnitFiles_reply);
    stub.set(ADDR(QDBusMessage, signature), stub_reply_signature);
    m_tester->StartUnit("com.deepin.systemMonitorPluginPopup", "1");
}

TEST_F(UT_Systemd1ManagerInterface, test_StopUnit_01)
{
    m_tester->StopUnit("com.deepin.systemMonitorPluginPopup", "1");
}

TEST_F(UT_Systemd1ManagerInterface, test_StopUnit_02)
{
    Stub stub;
    stub.set(ADDR(QDBusMessage, type), stub_listUnitFiles_reply);
    stub.set(ADDR(QDBusMessage, signature), stub_reply_signature);
    m_tester->StopUnit("com.deepin.systemMonitorPluginPopup", "1");
}

TEST_F(UT_Systemd1ManagerInterface, test_RestartUnit_01)
{
    m_tester->RestartUnit("com.deepin.systemMonitorPluginPopup", "1");
}

TEST_F(UT_Systemd1ManagerInterface, test_RestartUnit_02)
{
    Stub stub;
    stub.set(ADDR(QDBusMessage, type), stub_listUnitFiles_reply);
    stub.set(ADDR(QDBusMessage, signature), stub_reply_signature);
    m_tester->RestartUnit("com.deepin.systemMonitorPluginPopup", "1");
}

TEST_F(UT_Systemd1ManagerInterface, test_GetUnitFileState_01)
{
    m_tester->GetUnitFileState("com.deepin.systemMonitorPluginPopup");
}

TEST_F(UT_Systemd1ManagerInterface, test_GetUnitFileState_02)
{
    Stub stub;
    stub.set(ADDR(QDBusMessage, type), stub_listUnitFiles_reply);
    stub.set(ADDR(QDBusMessage, signature), stub_reply_signature_s);
    m_tester->GetUnitFileState("com.deepin.systemMonitorPluginPopup");
}

TEST_F(UT_Systemd1ManagerInterface, test_EnableUnitFiles_01)
{
    m_tester->EnableUnitFiles(QStringList() << "1" << "2", false, true);
}

TEST_F(UT_Systemd1ManagerInterface, test_EnableUnitFiles_02)
{
    Stub stub;
    stub.set(ADDR(QDBusMessage, type), stub_listUnitFiles_reply);
    stub.set(ADDR(QDBusMessage, signature), stub_reply_signature_bass);
    m_tester->EnableUnitFiles(QStringList() << "1" << "2", false, true);
}

TEST_F(UT_Systemd1ManagerInterface, test_DisableUnitFiles_01)
{
    m_tester->DisableUnitFiles(QStringList() << "1" << "2", false);
}

TEST_F(UT_Systemd1ManagerInterface, test_DisableUnitFiles_02)
{
    Stub stub;
    stub.set(ADDR(QDBusMessage, type), stub_listUnitFiles_reply);
    stub.set(ADDR(QDBusMessage, signature), stub_reply_signature_ass);
    m_tester->DisableUnitFiles(QStringList() << "1" << "2", false);
}
