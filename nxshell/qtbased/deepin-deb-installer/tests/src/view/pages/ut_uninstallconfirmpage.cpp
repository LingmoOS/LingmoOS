// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "../deb-installer/view/pages/uninstallconfirmpage.h"
#include "../deb-installer/view/pages/debinstaller.h"

#include <stub.h>
#include <ut_Head.h>

#include <QDebug>
#include <QEvent>

#include <gtest/gtest.h>
using namespace QApt;

QEvent::Type stud_uninstalltype()
{
    return QEvent::MouseButtonRelease;
}

class UnInstallConfirmPage_UT : public UT_HEAD
{
public:
    // 添加日志
    static void SetUpTestCase() { qDebug() << "SetUpTestCase" << endl; }
    static void TearDownTestCase() { qDebug() << "TearDownTestCase" << endl; }
    void SetUp()  // TEST跑之前会执行SetUp
    {
        uninstallPage = new UninstallConfirmPage();
        usleep(100 * 1000);
        qDebug() << "SetUp" << endl;
    }
    void TearDown()  // TEST跑完之后会执行TearDown
    {
        delete uninstallPage;
    }
    UninstallConfirmPage *uninstallPage;
};

TEST_F(UnInstallConfirmPage_UT, total_UT)
{
    Stub stub;
    stub.set(ADDR(QEvent, type), stud_uninstalltype);
    uninstallPage->installEventFilter(uninstallPage);
    uninstallPage->slotShowDetail();
    uninstallPage->slotHideDetail();
    EXPECT_FALSE(uninstallPage->m_infoWrapperWidget->isVisible());
    EXPECT_FALSE(uninstallPage->m_dependsInfomation->isVisible());
}
