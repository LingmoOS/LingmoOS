// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "listview.h"
#include "clipboardmodel.h"
#include "itemdelegate.h"

#include <QtTest>
#include <QDebug>
#include <QSignalSpy>

class TstListView : public testing::Test
{
public:
    void SetUp() override
    {
        list = new ListView;
        model = new ClipboardModel(list);
        delegate = new ItemDelegate;

        list->setModel(model);
        list->setItemDelegate(delegate);
    }

    void TearDown() override
    {
        delete list;
        list = nullptr;

        delete model;
        model = nullptr;

        delete delegate;
        delegate = nullptr;
    }

public:
    ListView *list = nullptr;
    ClipboardModel *model = nullptr;
    ItemDelegate *delegate = nullptr;
};

TEST_F(TstListView, scrollToTest)
{
    // nothing happend
    list->scrollTo(QModelIndex());
}

TEST_F(TstListView, keyPressTest)
{
    list->show();
    QTest::qWait(1);

    QTest::keyPress(list, Qt::Key_Up);
    QTest::keyPress(list, Qt::Key_Down);

    QTest::mouseMove(list, QPoint(list->geometry().center()));

    // other key press
    QTest::keyPress(list, Qt::Key_Tab);
    QTest::keyPress(list, Qt::Key_Backtab);
    QTest::qWait(1);

    QTest::keyPress(list->viewport(), Qt::Key_Tab);
    QTest::keyPress(list->viewport(), Qt::Key_Backtab);
    QTest::qWait(1);
}

TEST_F(TstListView, uiTest)
{
    ClipboardModel *model = new ClipboardModel(list);
    ItemDelegate *delegate = new ItemDelegate(list);

    list->setItemDelegate(delegate);
    list->setModel(model);

    list->show();
    QTest::qWait(10);

    QByteArray textbuf;
    QByteArray imagebuf;
    QByteArray filebuf;

    // 复制文本时产生的数据，用于测试
    QFile file1(":/qrc/text.buf");
    if (!file1.open(QIODevice::ReadOnly)) {
        qWarning() << "file open failed";
    } else {
        textbuf = file1.readAll();
    }
    file1.close();

    // 复制图片（非图片，图片文件属于文件类型）时产生的数据，用于测试
    QFile file2(":/qrc/image.buf");
    if (!file2.open(QIODevice::ReadOnly)) {
        qWarning() << "file open failed";
    } else {
        imagebuf = file2.readAll();
    }
    file2.close();

    // 复制文件时产生的数据，用于测试
    QFile file3(":/qrc/file.buf");
    if (!file3.open(QIODevice::ReadOnly)) {
        qWarning() << "file open failed";
    } else {
        filebuf = file3.readAll();
    }
    file3.close();

    for (int i = 0; i < 10; ++i) {
        model->dataComing(textbuf);
        model->dataComing(imagebuf);
        model->dataComing(filebuf);
    }
    // 留出时间让listview绘制
    QTest::qWait(10);

    ASSERT_EQ(model->data().size(), 30);
    ASSERT_EQ(model->data().first()->urls().size(), 3);

    QSignalSpy spy(model, &ClipboardModel::dataChanged);

    model->destroy(model->data().first());
    QTest::qWait(AnimationTime + 20);
    ASSERT_EQ(spy.count(), 1);

    QVariant vaildVar = model->data(list->indexAt(QPoint(0, 0)), 0);
    ASSERT_TRUE(vaildVar.isValid());
    QVariant invalidVar = model->data(QModelIndex(), 0);
    ASSERT_FALSE(invalidVar.isValid());

    // 测试dataReborn信号发送
    QSignalSpy rebornSpy(model, &ClipboardModel::dataReborn);
    model->reborn(model->data().last());
    ASSERT_EQ(rebornSpy.count(), 1);

    model->reborn(model->data().first());
    ASSERT_EQ(rebornSpy.count(), 2);

    model->clear();
    ASSERT_EQ(spy.count(), 2);

    //    QThread::msleep(300 + 10);
    //    ASSERT_EQ(model->data().size(), 2);
}

TEST_F(TstListView, mousePressTest)
{
    list->show();
    QTest::qWait(1);

    list->setCurrentIndex(QModelIndex());
    QTest::mousePress(list, Qt::LeftButton, Qt::NoModifier);
}
