// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "watermask/lingmolicensehelper.h"

#include "watermask/licenceInterface.h"

#include <QApplication>
#include <QtConcurrent>

#include "stubext.h"
#include <gtest/gtest.h>
using namespace ddplugin_canvas;

class UT_LingmoLicenseHelper : public testing::Test
{

protected:
    virtual void SetUp() override
    {
        helper = new LingmoLicenseHelper();
    }
    virtual void TearDown() override { delete helper; stub.clear(); }

protected:
    stub_ext::StubExt stub;
    LingmoLicenseHelper *helper = nullptr;
};

TEST_F(UT_LingmoLicenseHelper,requestLicenseState)
{
     EXPECT_NO_FATAL_FAILURE(helper->requestLicenseState());
}

TEST_F(UT_LingmoLicenseHelper,initFinshed)
{
    bool connection = false;
    stub.set_lamda(&LingmoLicenseHelper::requestLicenseState,[&connection](){connection = true;});
    ComLingmoLicenseInterface *interface = new ComLingmoLicenseInterface("str_service","str_path",QDBusConnection("connection"));
    helper->initFinshed(interface);
    helper->licenseInterface->LicenseStateChange();
    EXPECT_TRUE(connection);
}

TEST_F(UT_LingmoLicenseHelper,getLicenseState)
{
    LingmoLicenseHelper self;
    self.licenseInterface = new ComLingmoLicenseInterface("str_service","str_path",QDBusConnection("connection"));
    bool connection = false;
    QObject::connect(&self,&LingmoLicenseHelper::postLicenseState,&self,[&connection](int _t1, int _t2){connection = true;});
    helper->getLicenseState(&self);
    EXPECT_TRUE(connection);
}

TEST_F(UT_LingmoLicenseHelper,getServiceProperty)
{
    ComLingmoLicenseInterface *interface = new ComLingmoLicenseInterface("str_service","str_path",QDBusConnection("connection"));
    helper->licenseInterface = interface;
    interface->setProperty("ServiceProperty",QVariant::fromValue(QString("0")));
    LingmoLicenseHelper::LicenseProperty res = helper->getServiceProperty();
    EXPECT_EQ(res,LingmoLicenseHelper::LicenseProperty::Noproperty);
}

TEST_F(UT_LingmoLicenseHelper,getAuthorizationProperty)
{
    ComLingmoLicenseInterface *interface = new ComLingmoLicenseInterface("str_service","str_path",QDBusConnection("connection"));
    helper->licenseInterface = interface;
    LingmoLicenseHelper::LicenseProperty res =  helper->getAuthorizationProperty();
    EXPECT_EQ(res,LingmoLicenseHelper::LicenseProperty::Noproperty);
}
