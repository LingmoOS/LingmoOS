// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include "ddbussender.h"

#include <QtDBus>
#include <QDebug>

#include "fakedbus/fakedbusservice.h"

using Dtk::Core::DDBusInterface;

class ut_DDBusSender : public testing::Test
{
public:
    void SetUp() override
    {
        m_testservice = new FakeDBusService();
        m_sender = new DDBusSender;
    }
    void TearDown() override
    {
        delete m_testservice;
        delete m_sender;
    }

    FakeDBusService *m_testservice = nullptr;
    DDBusSender *m_sender = nullptr;
};


TEST_F(ut_DDBusSender, DDBusCaller)
{
    QDBusPendingReply<QString> reply = m_sender->service(m_testservice->get_service())
            .path(m_testservice->get_path())
            .interface(m_testservice->get_interface())
            .method(QString("foo"))
            .call();

    reply.waitForFinished();
    if (reply.error().isValid())
        qWarning() << reply.error().message();

    ASSERT_TRUE(reply.value() == QString("bar"));
}

TEST_F(ut_DDBusSender, DDBusProperty)
{
    auto prop = m_sender->service(m_testservice->get_service())
            .path(m_testservice->get_path())
            .interface(m_testservice->get_interface())
            .property(QString("strProperty"));

    QDBusPendingReply<QVariant> reply = prop.get();
    reply.waitForFinished();
    if (reply.error().isValid())
        qWarning() << reply.error().message();

    ASSERT_TRUE(reply.value().toString() == QString("testDBusService"));

    auto res = prop.set("myProp");
    res.waitForFinished();
    if (res.error().isValid())
        qWarning() << res.error().message();

    ASSERT_TRUE(m_testservice->strproperty() ==  QString("myProp"));
}
