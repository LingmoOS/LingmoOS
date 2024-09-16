// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <gtest/gtest.h>
#include <ddbussender.h>
#include <dexportedinterface.h>

#include <QtDBus>

DCORE_USE_NAMESPACE

class ut_DExportedInterface : public testing::Test
{
public:
    static void SetUpTestSuite() {
        QDBusConnection::sessionBus().registerService("org.deepin.ExpIntfTest");
        _infc = new DUtil::DExportedInterface;
        _infc->registerAction("quit", "quit the application", [](QString)->QVariant {
            qDebug() << "quit? No Way!";
            return QVariant();
        });

        _infc->registerAction("answer", "answer to the ultimate question of life, the universe, and everything",
                           [](QString)->QVariant {
            return QVariant(42);
        });
    }
    static void TearDownTestSuite() {
        QDBusConnection::sessionBus().unregisterService("org.deepin.ExpIntfTest");
        delete _infc;
    }

    static DUtil::DExportedInterface *_infc;
};

DUtil::DExportedInterface *ut_DExportedInterface::_infc = nullptr;

TEST_F(ut_DExportedInterface, list)
{
    auto caller = DDBusSender().service("org.deepin.ExpIntfTest")
            .path("/")
            .interface("com.deepin.ExportedInterface")
            .method("list");

    auto msg = caller.call();
    msg.waitForFinished();

    QDBusReply<QStringList> reply = msg;

    EXPECT_EQ(reply.value().size(), 2);
}

TEST_F(ut_DExportedInterface, help)
{
    auto caller = DDBusSender().service("org.deepin.ExpIntfTest")
            .path("/")
            .interface("com.deepin.ExportedInterface")
            .method("help")
            .arg(QString("quit"));

    auto msg = caller.call();
    msg.waitForFinished();

    QDBusReply<QString> reply = msg;

    EXPECT_EQ(reply.value(), "quit: quit the application");
}

TEST_F(ut_DExportedInterface, invoke)
{
    auto caller = DDBusSender().service("org.deepin.ExpIntfTest")
            .path("/")
            .interface("com.deepin.ExportedInterface")
            .method("invoke")
            .arg(QString("answer"))
            .arg(QString("balabala"));

    auto msg = caller.call();
    msg.waitForFinished();

    QDBusReply<QVariant> reply = msg;

    EXPECT_EQ(reply.value(), 42);

    caller = DDBusSender().service("org.deepin.ExpIntfTest")
                .path("/")
                .interface("com.deepin.ExportedInterface")
                .method("invoke")
                .arg(QString("unkownAction")) // invoke an unregistered action
                .arg(QString("balabala"));

    msg = caller.call();
    msg.waitForFinished();
    EXPECT_TRUE(msg.isError());
    EXPECT_TRUE(msg.error().type() == QDBusError::ErrorType::InvalidArgs);
}
