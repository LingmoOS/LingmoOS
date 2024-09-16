// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "appslistmodel.h"
#include "applistview.h"
#include "applistdelegate.h"
#include "windowedframe.h"
#undef private

#include <QTest>
#include <QApplication>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <DGuiApplicationHelper>

#include <gtest/gtest.h>

/**使用小窗口类WindowedFrame间接测试AppListView的
 * 接口, 因为AppListView的视图代码是在小窗口类初始
 * 化列表中创建的, 规避该类appListView_test中偶现的
 * 崩溃问题
 * @brief The Tst_Applistview class
 */
class Tst_Applistview : public testing::Test
{
public:
    void SetUp() override
    {
        m_windowFrame = new WindowedFrame;
    }

    void TearDown() override
    {
        delete m_windowFrame;
    }

public:
    WindowedFrame *m_windowFrame;
};

TEST_F(Tst_Applistview, appDelegate_test)
{
    AppListView *appListView = m_windowFrame->m_appsView;

    appListView->setModel(m_windowFrame->m_appsModel);

    AppListDelegate delegate(appListView);
    appListView->setItemDelegate(&delegate);
    AppListDelegate * viewDelegate = static_cast<AppListDelegate *>(appListView->itemDelegate());
    if (viewDelegate)
        viewDelegate->setActived(true);
}

TEST_F(Tst_Applistview, event_test)
{
    AppListView *appListView = m_windowFrame->m_appsView;

    AppsListModel model(AppsListModel::All);
    QModelIndex index;
    model.insertRow(0, index);
    appListView->setModel(&model);

    QWheelEvent wheelEvent(QPointF(0, 0), 0, Qt::MiddleButton, Qt::ControlModifier);
    QApplication::sendEvent(appListView, &wheelEvent);

    QMouseEvent mouseMoveEvent(QEvent::MouseMove, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(appListView, &mouseMoveEvent);

    QMouseEvent mousePress(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(appListView, &mousePress);

    appListView->hasAutoScroll();
    QMouseEvent releaseEvent(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(appListView, &releaseEvent);

    QMimeData mimeData;
    mimeData.setData("test", "test");

    QDragEnterEvent dragEnterEvent(QPoint(0, 1), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(appListView, &dragEnterEvent);

    QDragMoveEvent dragMoveEvent(QPoint(0, 2), Qt::MoveAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(appListView, &dragMoveEvent);

    QDragLeaveEvent dragLeaveEvent;
    QApplication::sendEvent(appListView, &dragLeaveEvent);

    QDropEvent dropEvent(QPointF(0, 0), Qt::CopyAction, &mimeData, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(appListView, &dropEvent);

    QEvent enterEvent(QEvent::Enter);
    QApplication::sendEvent(appListView, &enterEvent);

    QEvent leaveEvent(QEvent::Leave);
    QApplication::sendEvent(appListView, &leaveEvent);

    appListView->handleScrollValueChanged();
    appListView->handleScrollFinished();
    appListView->prepareDropSwap();
    appListView->dropSwap();
    appListView->menuHide();
}

TEST_F(Tst_Applistview, appListView_test)
{
    AppListView *appListView = m_windowFrame->m_appsView;

    AppsListModel model(AppsListModel::All);
    QModelIndex index;
    model.insertRow(0, index);
    appListView->setModel(&model);
}

TEST_F(Tst_Applistview, appListDelegate_test)
{
    AppListView *appListView = m_windowFrame->m_appsView;
    AppListDelegate appListDelegate(appListView);
    appListDelegate.setActived(false);

    QPixmap pix;
    appListDelegate.dropShadow(pix, 5, QColor(Qt::red), QPoint(10, 10));

    pix.load("/usr/share/backgrounds/default_background.jpg");
    appListDelegate.dropShadow(pix, 5, QColor(Qt::red), QPoint(10, 10));
}
