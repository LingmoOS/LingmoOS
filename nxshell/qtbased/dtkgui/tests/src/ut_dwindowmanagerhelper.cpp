// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "test.h"
#include <QTest>

#include "dwindowmanagerhelper.h"
#include "dforeignwindow.h"

DGUI_USE_NAMESPACE

class TDWindowMangerHelper : public DTest
{
protected:
    void SetUp();

    DWindowManagerHelper *wm_helper;
};

void TDWindowMangerHelper::SetUp()
{
    wm_helper = DWindowManagerHelper::instance();
}

TEST_F(TDWindowMangerHelper, testStaticFunction)
{
    if (qgetenv("QT_QPA_PLATFORM").contains("offscreen"))
        return;

    QWidget *w = new QWidget;
    w->createWinId();
    //w->show();
     ASSERT_TRUE(w->windowHandle());
    enum { TestMotifFunction = DWindowManagerHelper::FUNC_RESIZE | DWindowManagerHelper::FUNC_MOVE, TestAllMotifFunction = DWindowManagerHelper::FUNC_ALL};
    enum { TestDecorations = DWindowManagerHelper::DECOR_BORDER | DWindowManagerHelper::DECOR_RESIZEH, TestAllDecorations = DWindowManagerHelper::DECOR_ALL };

    // 测试静态函数测试是否正常
    DWindowManagerHelper::setMotifFunctions(w->windowHandle(), DWindowManagerHelper::MotifFunctions(TestMotifFunction));
    DWindowManagerHelper::MotifFunctions mFuncs = DWindowManagerHelper::getMotifFunctions(w->windowHandle());
    if (wm_helper->windowManagerName() == DWindowManagerHelper::KWinWM) {
        ASSERT_EQ(mFuncs, TestMotifFunction);
    } else {
        qDebug() << "not support other wm";
    }


    mFuncs = DWindowManagerHelper::setMotifFunctions(w->windowHandle(), DWindowManagerHelper::MotifFunctions(TestAllMotifFunction), true);
    ASSERT_EQ(mFuncs, TestAllMotifFunction);

    DWindowManagerHelper::setMotifDecorations(w->windowHandle(), DWindowManagerHelper::MotifDecorations(TestDecorations));
    DWindowManagerHelper::MotifDecorations mDecos = DWindowManagerHelper::getMotifDecorations(w->windowHandle());
    if (wm_helper->windowManagerName() == DWindowManagerHelper::KWinWM)
        ASSERT_EQ(mDecos, TestDecorations);

    mDecos = DWindowManagerHelper::setMotifDecorations(w->windowHandle(), DWindowManagerHelper::MotifDecorations(TestAllDecorations), true);
    if (wm_helper->windowManagerName() == DWindowManagerHelper::KWinWM)
        ASSERT_EQ(mDecos, TestAllDecorations);

    // 没有崩溃则测试成功
    enum { TestWindowType =  DWindowManagerHelper::DesktopType | DWindowManagerHelper::MenuType };
    DWindowManagerHelper::setWmWindowTypes(w->windowHandle(), DWindowManagerHelper::WmWindowTypes(TestWindowType));
    DWindowManagerHelper::setWmClassName(QByteArrayLiteral("TestWmClass"));
    DWindowManagerHelper::popupSystemWindowMenu(w->windowHandle());

    delete w;
}

TEST_F(TDWindowMangerHelper, testFunctions)
{
    if (qgetenv("QT_QPA_PLATFORM").contains("offscreen"))
        return;

    qDebug() << wm_helper->windowManagerNameString() <<
                "\nhas blur window:" << wm_helper->hasBlurWindow() <<
                "\nhas composite:" << wm_helper->hasComposite() <<
                "\nhas not titlebar:" << wm_helper->hasNoTitlebar();

//    ASSERT_TRUE(wm_helper->hasWallpaperEffect());
    // ASSERT_FALSE(wm_helper->windowManagerNameString().isEmpty());
    if (wm_helper->windowManagerNameString().contains(QStringLiteral("DeepinGala"))) {
        ASSERT_EQ(wm_helper->windowManagerName(), DWindowManagerHelper::DeepinWM);
    } else if (wm_helper->windowManagerNameString().contains(QStringLiteral("KWin"))) {
        ASSERT_EQ(wm_helper->windowManagerName(), DWindowManagerHelper::KWinWM);
    } else {
        ASSERT_EQ(wm_helper->windowManagerName(), DWindowManagerHelper::OtherWM);
    }

    if (wm_helper->windowManagerName() == DWindowManagerHelper::KWinWM) {
        ASSERT_FALSE(wm_helper->allWindowIdList().isEmpty());
        ASSERT_FALSE(wm_helper->currentWorkspaceWindowIdList().isEmpty());
        ASSERT_FALSE(wm_helper->currentWorkspaceWindows().isEmpty());
        ASSERT_TRUE(wm_helper->windowFromPoint(wm_helper->currentWorkspaceWindows().first()->position()));
    } else {
        qDebug() << "allWindowIdList count:" << wm_helper->allWindowIdList().count() <<
                    "\ncurrentWorkspaceWindowIdList count:" << wm_helper->currentWorkspaceWindowIdList().count() <<
                    "\ncurrentWorkspaceWindows count:" << wm_helper->currentWorkspaceWindows().count() <<
                    "\nwindowFromPoint:" << wm_helper->windowFromPoint(QPoint());
    }
}
