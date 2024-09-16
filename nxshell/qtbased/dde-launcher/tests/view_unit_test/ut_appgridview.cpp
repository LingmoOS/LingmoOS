// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "applistdelegate.h"

#define private public
#include "appgridview.h"
#include "fullscreenframe.h"
#undef private

#include <QApplication>
#include <QDropEvent>
#include <QDragEnterEvent>
#include <QTest>

#include <gtest/gtest.h>

class Tst_Appgridview : public testing::Test
{
public:
    void SetUp() override
    {
        m_widget = new AppGridView;
    }

    void TearDown() override
    {
        delete m_widget;
        m_widget = nullptr;
    }

public:
    AppGridView *m_widget;
};

TEST_F(Tst_Appgridview, appGridView_test)
{
    m_widget->indexYOffset(QModelIndex());
    QWidget *widget = new QWidget(m_widget);
    m_widget->setContainerBox(widget);

    m_widget->fitToContent();

    m_widget->creatSrcPix(QModelIndex(), QString("dde-calendar"));
    m_widget->creatSrcPix(QModelIndex(), QString("deepin-editor"));

    m_widget->appIconRect(QModelIndex());

    QMouseEvent leftMousePress(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(m_widget, &leftMousePress);

    QMouseEvent rightMousePress(QEvent::MouseButtonPress, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::RightButton, Qt::RightButton, Qt::NoModifier, Qt::MouseEventSynthesizedByQt);
    QApplication::sendEvent(m_widget, &rightMousePress);

    QMouseEvent moveEvent(QEvent::MouseMove, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(m_widget, &moveEvent);

    QMouseEvent leftReleaseEvent(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(m_widget, &leftReleaseEvent);

    QMouseEvent rightReleaseEvent(QEvent::MouseButtonRelease, QPointF(0, 0), QPointF(0, 1), QPointF(1, 1), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    QApplication::sendEvent(m_widget, &rightReleaseEvent);

    QMimeData data;
    data.setText("test");

    QDragMoveEvent dragMoveEvent(QPoint(0, 10), Qt::DropAction::CopyAction, &data, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(m_widget, &dragMoveEvent);

    QDropEvent dropEvent(QPointF(0, 20), Qt::CopyAction, &data, Qt::LeftButton, Qt::NoModifier, QEvent::Drop);
    QApplication::sendEvent(m_widget, &dropEvent);

    QDragLeaveEvent dragLeaveEvent;
    QApplication::sendEvent(m_widget, &dragLeaveEvent);

    m_widget->m_calcUtil->setDisplayMode(GROUP_BY_CATEGORY);
    QDragLeaveEvent categoryDragLeaveEvent;
    QApplication::sendEvent(m_widget, &categoryDragLeaveEvent);
}

TEST_F(Tst_Appgridview, appItemDelegate_test)
{
    AppItemDelegate appItemDelegate;

    QRect rect(QPoint(10, 10), QSize(20, 30));
    appItemDelegate.itemBoundingRect(rect);

    rect.setSize(QSize(30, 20));
    appItemDelegate.itemBoundingRect(rect);

    rect.setSize(QSize(20, 20));
    appItemDelegate.itemBoundingRect(rect);
}

TEST_F(Tst_Appgridview, listModel_test)
{
    AppsListModel *appsListModel = static_cast<AppsListModel *>(m_widget->model());

    if (appsListModel) {
        appsListModel->setDragDropIndex(QModelIndex());
        appsListModel->setDragDropIndex(QModelIndex());
        appsListModel->setDraggingIndex(QModelIndex());
        appsListModel->dropInsert("deepin-editor", 0);
        appsListModel->dropSwap(0);
        appsListModel->clearDraggingIndex();
        appsListModel->layoutChanged(AppsListModel::Category);
        appsListModel->indexDragging(QModelIndex());

        ItemInfo info;
        appsListModel->itemDataChanged(info);
    }
}

TEST_F(Tst_Appgridview, itemDelegate_test)
{
    AppItemDelegate delegate(m_widget);

    m_widget->setItemDelegate(&delegate);

    QRect boundRect(QPoint(10, 10), QSize(20, 20));
    delegate.itemTextRect(boundRect, boundRect, true);
}
