// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "dbus/dbus_common.h"
#include "dbus/dbus_properties_interface.h"
#include "dbus/environment_file.h"
#include "dbus/systemd1_service_interface.h"

//qt
#include <QVariant>
#include <QDBusInterface>
#include <QByteArray>
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QtDBus>
#include <QObject>
#include <QTest>

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

/***************************************STUB begin*********************************************/
QDBusMessage::MessageType stub_getEnvironmentFiles_reply()
{
    return QDBusMessage::ReplyMessage;
}

QString stub_reply_signature_asb()
{
    return "a(sb)";
}
/***************************************STUB end**********************************************/

class UT_Systemd1ServiceInterface : public ::testing::Test
{
public:
    UT_Systemd1ServiceInterface() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new Systemd1ServiceInterface("org.freedesktop.systemd1", "/org/freedesktop/systemd1/unit/virtlogd_2eservice",
                                                QDBusConnection::sessionBus());
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    Systemd1ServiceInterface *m_tester;
};

TEST_F(UT_Systemd1ServiceInterface, initTest)
{

}

TEST_F(UT_Systemd1ServiceInterface, test_getMainPID_01)
{
    QPair<ErrorContext, quint32> reply = m_tester->getMainPID();

}

TEST_F(UT_Systemd1ServiceInterface, test_getMemoryCurrent_01)
{
    QPair<ErrorContext, quint64> reply = m_tester->getMemoryCurrent();
}

TEST_F(UT_Systemd1ServiceInterface, test_getControlGroup_01)
{
    QPair<ErrorContext, QString> reply = m_tester->getControlGroup();

}

TEST_F(UT_Systemd1ServiceInterface, test_getEnvironmentFiles_01)
{
    QPair<ErrorContext, EnvironmentFileList> reply = m_tester->getEnvironmentFiles();
}

TEST_F(UT_Systemd1ServiceInterface, test_getEnvironmentFiles_02)
{
    Stub stub;
    stub.set(ADDR(QDBusMessage, type), stub_getEnvironmentFiles_reply);
    stub.set(ADDR(QDBusMessage, signature), stub_reply_signature_asb);
    QPair<ErrorContext, EnvironmentFileList> reply = m_tester->getEnvironmentFiles();
}

