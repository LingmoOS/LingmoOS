// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test.h"
#include "dwindowgroupleader.h"
#include <QDebug>

#include <QTest>
#include <QWidget>

DGUI_USE_NAMESPACE


class TDWindowGroupLeader : public DTest
{
protected:
    void SetUp();
    void TearDown();

    DWindowGroupLeader *groupLeader;
    QWindow *window1;
    QWindow *window2;
};

void TDWindowGroupLeader::SetUp()
{
    groupLeader = new DWindowGroupLeader;
    window1 = new QWindow;
    window2 = new QWindow;

    window1->create();
    window2->create();
}

void TDWindowGroupLeader::TearDown()
{
    delete groupLeader;
    delete window1;
    delete window2;
}

TEST_F(TDWindowGroupLeader, testFunctions)
{
    if (qgetenv("QT_QPA_PLATFORM").contains("offscreen"))
        return;

    // 测试多种情况添加window时数据是否正常
    groupLeader->addWindow(window1);
    groupLeader->addWindow(window2);
    qDebug() << "addWindow 1 & 2";

    qDebug() << "groupLeaderId" << groupLeader->groupLeaderId();
    qDebug() << "groupLeaderId" << groupLeader->clientLeaderId();

    groupLeader->removeWindow(window1);
    qDebug() << "removeWindow 1";

    qDebug() << "groupLeaderId" << groupLeader->groupLeaderId();
    qDebug() << "groupLeaderId" << groupLeader->clientLeaderId();

    groupLeader->removeWindow(window2);
    qDebug() << "removeWindow 2";
    qDebug() << "groupLeaderId" << groupLeader->groupLeaderId();
    qDebug() << "groupLeaderId" << groupLeader->clientLeaderId();
}
