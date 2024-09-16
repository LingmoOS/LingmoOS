// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QCoreApplication>
#include <gtest/gtest.h>
#include "global.h"
#include <QTimer>
#include <QDBusMetaType>

namespace {
void registerComplexDbusType()  // FIXME: test shouldn't associate with DBus
{
    qRegisterMetaType<ObjectInterfaceMap>();
    qDBusRegisterMetaType<ObjectInterfaceMap>();
    qRegisterMetaType<ObjectMap>();
    qDBusRegisterMetaType<ObjectMap>();
    qDBusRegisterMetaType<QStringMap>();
    qRegisterMetaType<PropMap>();
    qDBusRegisterMetaType<PropMap>();
    qDBusRegisterMetaType<QDBusObjectPath>();
}
}  // namespace

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    registerComplexDbusType();

    ::testing::InitGoogleTest(&argc, argv);
    int ret{0};
    QTimer::singleShot(0, &app, [&ret] {
        ret = RUN_ALL_TESTS();
        QCoreApplication::quit();
    });
    QCoreApplication::exec();
    return ret;
}
