// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <QStringList>
#include <QVariant>
#include <gtest/gtest.h>
#include "../../src/dbusinterface/dbusnotify.h"


using namespace testing;

class DBusNotifyTest:public testing::Test{

public:
    DBusNotify *notifyDBus;
    virtual void SetUp() override{
        std::cout << "start DBusNotifyTest" << std::endl;
        notifyDBus = new DBusNotify(nullptr);
        notifyDBus->GetCapbilities();
        QString out1;
        QString out2;
        QString out3;
        notifyDBus->GetServerInformation(out1, out2, out3);
        notifyDBus->GetServerInformation();
    }

    virtual void TearDown() override{
        notifyDBus->GetAllRecords();
        notifyDBus->RemoveRecord("ut-deepin-screen-recorder");
        notifyDBus->ClearRecords();
        delete  notifyDBus;
        std::cout << "end DBusNotifyTest" << std::endl;
    }
};

TEST_F(DBusNotifyTest, Notify)
{
    notifyDBus->Notify("ut-deepin-screen-recorder", 0,  "deepin-screen-recorder", "",
                       "ut-deepin-screen-recorder", QStringList(), QVariantMap(), 3000);
}
TEST_F(DBusNotifyTest, CloseNotification)
{
    notifyDBus->CloseNotification(0);
}
