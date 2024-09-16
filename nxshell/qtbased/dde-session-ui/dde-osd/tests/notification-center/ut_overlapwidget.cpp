// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#define protected public
#include "notification-center/overlapwidet.h"
#include "notifymodel.h"
#include "notifylistview.h"
#include "notificationentity.h"
#undef private
#undef protected

#include <QApplication>
#include <QTest>

#include <gtest/gtest.h>

class UT_OverLapWidet : public testing::Test
{
public:
    void SetUp() override
    {
        model = new NotifyModel();
        entityPtr = EntityPtr(new NotificationEntity("deepin-editor", "0", "deepin-editor", "", ""));
        obj = new OverLapWidet(model, entityPtr);
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;

        delete model;
        model = nullptr;
    }

public:
    OverLapWidet *obj = nullptr;
    NotifyModel *model = nullptr;
    EntityPtr entityPtr;
};

TEST_F(UT_OverLapWidet, coverageTest)
{
    model->addNotify(entityPtr);
    NotifyListView listview;
    listview.setModel(model);
    model->addNotify(entityPtr);
    model->addAppData(entityPtr);
    for (int row = 0; row < model->rowCount(QModelIndex()); row++) {
        listview.setIndexWidget(model->index(row), (new BubbleItem(obj, entityPtr)));
    }
    obj->setParentView(&listview);
    entityPtr->setHideCount(3);
    obj->initOverlap();
    obj->hideOverlapBubble();
    obj->getFaceItem();
    obj->bubbleElements();
    obj->update();
    obj->show();
    obj->expandAppGroup();

    QEvent event = QEvent(QEvent::FocusIn);
    qApp->sendEvent(obj, &event);
    event = QEvent(QEvent::FocusOut);
    qApp->sendEvent(obj, &event);
    QTest::mouseRelease(obj, Qt::LeftButton);

    for(auto child : obj->children()) {
        HalfRoundedRectWidget *w = qobject_cast<HalfRoundedRectWidget *>(child);
        if (w) {
            QPaintEvent paintevent(QRect(0,0,1,1));
            w->paintEvent(&paintevent);
            break;
        }
    }
}
