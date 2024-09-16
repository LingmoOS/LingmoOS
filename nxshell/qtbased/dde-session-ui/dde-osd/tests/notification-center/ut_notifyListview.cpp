// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mockpersistence.h"
#define private public
#define protected public
#include "notifymodel.h"
#include "itemdelegate.h"
#include "notifylistview.h"
#include "overlapwidet.h"
#include "bubbleitem.h"
#undef private
#undef protected

#include <QAbstractItemModelTester>
#include <QFocusEvent>
#include <QApplication>
#include <QTimer>
#include <QWidget>
#include <QTest>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

class MockPersistence;
class PersistenceHelper;

class UT_NotifyListview : public testing::Test
{
public:
    void SetUp() override
    {
        persistence = new MockPersistence;
        helper = new PersistenceHelper;

        EXPECT_CALL(*persistence, getAllNotify()).
                    WillRepeatedly(testing::Invoke(helper, &PersistenceHelper::getAllNotify));

        EXPECT_CALL(*persistence, getAll()).
                    WillRepeatedly(testing::Invoke(helper, &PersistenceHelper::getAll));

        EXPECT_CALL(*persistence, addOne(testing::_)).
                    WillRepeatedly(testing::Invoke(helper, &PersistenceHelper::addOne));

        EXPECT_CALL(*persistence, addAll(testing::_)).
                    WillRepeatedly(testing::Invoke(helper, &PersistenceHelper::addAll));

        EXPECT_CALL(*persistence, removeOne(testing::_)).
                    WillRepeatedly(testing::Invoke(helper, &PersistenceHelper::removeOne));

        EXPECT_CALL(*persistence, removeApp(testing::_)).
                    WillRepeatedly(testing::Invoke(helper, &PersistenceHelper::removeApp));

        EXPECT_CALL(*persistence, recordCount()).
                    WillRepeatedly(testing::Invoke(helper, &PersistenceHelper::getRecordCount));

        EXPECT_CALL(*persistence, getById(testing::_)).
                    WillRepeatedly(testing::Invoke(helper, &PersistenceHelper::getById));

        EXPECT_CALL(*persistence, getFrom(testing::_, testing::_)).
                    WillRepeatedly(testing::Invoke(helper, &PersistenceHelper::getFrom));

        obj = new NotifyListView();
        model = new NotifyModel(nullptr, persistence, obj);
        delegate = new ItemDelegate(obj, model);
        obj->setModel(model);
        obj->setItemDelegate(delegate);
        QString appName = "deepin-editor";
        uint id = 0;
        QString appIcon = "deepin-screenshot";
        QString summary = "截图完成";
        QString body = "1.png";
        QStringList actions ;
        actions << "_open" << "查看";
        QVariantMap hints;
        int expireTimeout = -1;

        entity = std::make_shared<NotificationEntity>(appName, QString(), appIcon,
                                                                      summary, body, actions, hints,
                                                                      QString::number(QDateTime::currentMSecsSinceEpoch()),
                                                                      QString::number(id),
                                                                      QString::number(expireTimeout));
        bubbleItem = new BubbleItem(nullptr, entity);
    }

    void TearDown() override
    {
        delete obj;
        delete model;
        delete delegate;
        delete persistence;
        delete helper;
        delete bubbleItem;

        obj = nullptr;
        model = nullptr;
        delegate = nullptr;
        persistence = nullptr;
        helper = nullptr;
        bubbleItem = nullptr;
    }

public:
    NotifyListView *obj = nullptr;
    NotifyModel *model = nullptr;
    ItemDelegate *delegate = nullptr;
    MockPersistence *persistence = nullptr;
    PersistenceHelper *helper = nullptr;
    BubbleItem *bubbleItem = nullptr;
    EntityPtr entity;
};

TEST_F(UT_NotifyListview, coverageTest)
{
    auto notification = std::make_shared<NotificationEntity>("deepin-editor");
    ListItem item = model->getAppData("deepin-editor");
    model->addNotify(notification);
    for (int row = 0; row < model->rowCount(QModelIndex()); row++) {
        obj->setIndexWidget(model->index(row), (new BubbleItem(obj, entity)));
    }

    obj->show();
    obj->aniState();
    QTest::keyPress(obj, Qt::Key_Tab);
    QTest::keyPress(obj, Qt::Key_Down);
    QTest::keyPress(obj, Qt::Key_Up);
    QTest::keyPress(obj, Qt::Key_Backtab);
    QTest::keyPress(obj, Qt::Key_Return);

    ListItem listItem;
    listItem.appName = entity->appName();
    listItem.showList = QList<EntityPtr>({entity});
    listItem.hideList = QList<EntityPtr>({entity});
    auto option = QStyleOptionViewItem();
    option.rect = QRect(0,0,10,10);
    for (int row = 0; row < model->rowCount(QModelIndex()); row++) {
        QWidget *editor = obj->itemDelegate()->createEditor(obj, QStyleOptionViewItem(), model->index(row));
        obj->itemDelegate()->updateEditorGeometry(editor, QStyleOptionViewItem(), model->index(row));
    }
    obj->createExpandAnimation(0, listItem);
    obj->createRemoveAnimation(bubbleItem);
    model->cacheData(notification);
    model->freeData();
    model->removeNotify(notification);
    model->expandData("deepin-editor");
    model->collapseData();
    model->removeAppGroup("deepin-editor");
    model->removeTimeOutNotify();
    model->removeAllData();
}

