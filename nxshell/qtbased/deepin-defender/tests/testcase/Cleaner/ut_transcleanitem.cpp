// SPDX-FileCopyrightText: 2021 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_transcleanitem.h"
#include "window/modules/cleaner/trashcleanitem.h"
#include "window/modules/cleaner/widgets/cleanerresultitemwidget.h"

#include "gtest/gtest.h"

#include <QCheckBox>
#include <QThread>
#include <QProcess>

// 由于在UT_TrashCleanWidget的UI操作中，已经走完了完整流程
// 此用例应作为补充
// 针对根、父、子类的各种勾选行为判断是否正确
// 插入空结点是否有影响

const QString test_file_path = "PATH/cleanerTestDir/template";

TransCleanItemTest::TransCleanItemTest()
{
}

void TransCleanItemTest::SetUp()
{
}

void TransCleanItemTest::TearDown()
{
}

// 目前在UI上只使用三级结构
//    |--root
//       |--parent
//          |--child
//          |--NULL
// 可以插入空的子结点
// root parent 结点仅作为逻辑结点，起归类作用，不保存扫描路径
// 也可以仅在root下插入child
TEST_F(TransCleanItemTest, Sample)
{
    TrashCleanItem rootNode;
    TrashCleanItem parentNode(&rootNode);
    TrashCleanItem childNode(&parentNode);
    parentNode.addChild(nullptr);

    QString title = "title";
    parentNode.setTitle("title");
    parentNode.setTip("tip");
    EXPECT_EQ(title, parentNode.title());

    QStringList filePaths = QStringList() << test_file_path;
    title = "childTitle";
    childNode.setTitle(title);
    EXPECT_EQ(title, childNode.title());
    childNode.setFilePaths(filePaths);
}

// 默认值检查
TEST_F(TransCleanItemTest, Default_Value)
{
    TrashCleanItem rootNode;
    TrashCleanItem parentNode(&rootNode);
    TrashCleanItem childNode(&parentNode);

    EXPECT_TRUE(childNode.isSelected());
    EXPECT_FALSE(childNode.isShown());

    EXPECT_EQ(&rootNode, parentNode.parent());
    EXPECT_EQ(&parentNode, childNode.parent());

    EXPECT_EQ(quint64(0), childNode.totalFileSize());
    EXPECT_EQ(uint(0), childNode.totalFileAmount());
    EXPECT_EQ(quint64(0), childNode.itemFileSize());
    EXPECT_EQ(uint(0), childNode.itemFileAmount());
    EXPECT_NE(nullptr, childNode.itemWidget());
}

// root parent child 的勾选行为：
// root下任意结点被勾选时，root项被勾选
// child被勾选时，parent被勾选
// parent被勾选时，root、parent下的child被勾选
//
//|--root
//  |--parent1
//      |--child1
//  |--parent2
//      |--child2
//      |--NULL
TEST_F(TransCleanItemTest, Selete_Item)
{
    TrashCleanItem rootNode;
    TrashCleanItem parentNode1(&rootNode);
    TrashCleanItem childNode1(&parentNode1);
    TrashCleanItem parentNode2(&rootNode);
    TrashCleanItem childNode2(&parentNode2);
    parentNode2.addChild(nullptr);

    // 默认都是选中
    // 取消root选中，所有项都是未选中状态
    // 以下操作模拟界面上的选中行为

    // 取消选中root
    rootNode.itemWidget()->setCheckBoxStatus(false);
    rootNode.itemWidget()->childSelected();

    EXPECT_FALSE(parentNode1.isSelected());
    EXPECT_FALSE(childNode2.isSelected());

    // 重新选中root
    rootNode.itemWidget()->setCheckBoxStatus(true);
    rootNode.itemWidget()->childSelected();
    EXPECT_TRUE(parentNode1.isSelected());
    EXPECT_TRUE(childNode2.isSelected());

    // 取消选中parent1,此时child1也被取消
    parentNode1.itemWidget()->setCheckBoxStatus(false);
    parentNode1.itemWidget()->childSelected();
    EXPECT_TRUE(rootNode.isSelected());
    EXPECT_FALSE(childNode1.isSelected());

    // 取消选中parent2,此时所有项目都被反选，root也取消选中
    parentNode2.itemWidget()->setCheckBoxStatus(false);
    parentNode2.itemWidget()->childSelected();

    EXPECT_FALSE(rootNode.isSelected());
    EXPECT_FALSE(childNode2.isSelected());

    // 再次选中child2,此时parent2 root被选中
    childNode2.itemWidget()->setCheckBoxStatus(true);
    childNode2.itemWidget()->childSelected();

    EXPECT_TRUE(rootNode.isSelected());
    EXPECT_TRUE(parentNode2.isSelected());
}

// 将大小转化为带进制单位的字符串
// 转化依次为：
// 13BYTE
// 13KB
// 13MB
// 13GB
TEST_F(TransCleanItemTest, Caculate_File_Size)
{
    const int num = 13;
    quint64 byte_size_13 = num;
    quint64 kb_size_13 = byte_size_13 * 1024;
    quint64 mb_size_13 = kb_size_13 * 1024;
    quint64 gb_size_13 = mb_size_13 * 1024;

    TrashCleanItem item;
    QString byte = item.fileSizeToString(byte_size_13);
    QString kb = item.fileSizeToString(kb_size_13);
    QString mb = item.fileSizeToString(mb_size_13);
    QString gb = item.fileSizeToString(gb_size_13);
    EXPECT_EQ(QString("13.00 Byte"), byte);
    EXPECT_EQ(QString("13.00 KB"), kb);
    EXPECT_EQ(QString("13.00 MB"), mb);
    EXPECT_EQ(QString("13.00 GB"), gb);
}

TEST_F(TransCleanItemTest, Disable_Checkbox)
{
    TrashCleanItem rootNode;
    TrashCleanItem parentNode1(&rootNode);
    TrashCleanItem childNode1(&parentNode1);
    TrashCleanItem parentNode2(&rootNode);
    TrashCleanItem childNode2(&parentNode2);

    // 默认均为可勾选，且已被勾选
    // 子项全部反选，父项与根项被反选
    childNode1.setSelected(false);
    childNode1.stageChanged(false);
    childNode2.setSelected(false);
    childNode2.stageChanged(false);

    EXPECT_FALSE(childNode2.isSelected());
    EXPECT_FALSE(childNode1.isSelected());
    EXPECT_FALSE(parentNode1.isSelected());
    EXPECT_FALSE(parentNode2.isSelected());
    EXPECT_FALSE(rootNode.isSelected());

    // 先全选
    rootNode.setSelected(true);
    rootNode.setChildSelected(true);
    EXPECT_TRUE(childNode2.isSelected());
    EXPECT_TRUE(childNode1.isSelected());

    // 阻止子项被选择
    childNode1.setItemUncheckable();
    childNode2.setItemUncheckable();

    childNode1.setSelected(false);
    childNode1.stageChanged(false);
    childNode2.setSelected(false);
    childNode2.stageChanged(false);
    EXPECT_TRUE(parentNode1.isSelected());
    EXPECT_TRUE(parentNode2.isSelected());
    EXPECT_TRUE(rootNode.isSelected());
}

////// 删除指定结点下的所有子结点
//TEST_F(TransCleanItemTest, Remove_Children)
//{
//    TrashCleanItem rootNode;
//    TrashCleanItem parentNode1(&rootNode);
//    TrashCleanItem childNode1(&parentNode1);
//    TrashCleanItem parentNode2(&rootNode);
//    TrashCleanItem childNode2(&parentNode2);
//    parentNode2.addChild(nullptr);

//    parentNode2.removeChildren();

//    EXPECT_TRUE(rootNode.isSelected());
//    EXPECT_TRUE(parentNode2.isSelected());
//}
