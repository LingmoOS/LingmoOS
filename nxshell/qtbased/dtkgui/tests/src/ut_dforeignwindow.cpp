// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test.h"
#include "dforeignwindow.h"

#include <dwindowmanagerhelper.h>
#include <QSignalSpy>
#include <QDebug>

DGUI_BEGIN_NAMESPACE

#define WmClass "_d_WmClass"
#define ProcessId "_d_ProcessId"

class TDForeignWindow : public DTest
{
protected:
    virtual void SetUp()
    {
        const QVector<quint32> &currentIdList = DWindowManagerHelper::instance()->currentWorkspaceWindowIdList();
        foreignWindows.clear();
        for (quint32 currentId : qAsConst(currentIdList)) {
            foreignWindows.append(DForeignWindow::fromWinId(currentId));
        }
    }
    virtual void TearDown()
    {
        qDeleteAll(foreignWindows);
        foreignWindows.clear();
    }

    QList<DForeignWindow *> foreignWindows;
};

TEST_F(TDForeignWindow, wmClass)
{
    for (auto foreignWindow : qAsConst(foreignWindows))
        ASSERT_NE(foreignWindow->wmClass(), QString());
}

TEST_F(TDForeignWindow, pid)
{
    for (auto foreignWindow : qAsConst(foreignWindows))
        ASSERT_NE(foreignWindow->pid(), 0);
}

TEST_F(TDForeignWindow, event)
{
    QDynamicPropertyChangeEvent wmevent(WmClass);
    QDynamicPropertyChangeEvent pidevent(ProcessId);

    for (auto foreignWindow : qAsConst(foreignWindows)) {
        QSignalSpy wmspy(foreignWindow, SIGNAL(wmClassChanged()));
        ASSERT_TRUE(foreignWindow->event(&wmevent));
        ASSERT_EQ(wmspy.count(), 1);

        QSignalSpy pidspy(foreignWindow, SIGNAL(pidChanged()));
        ASSERT_TRUE(foreignWindow->event(&pidevent));
        ASSERT_EQ(pidspy.count(), 1);
    }
}

DGUI_END_NAMESPACE
