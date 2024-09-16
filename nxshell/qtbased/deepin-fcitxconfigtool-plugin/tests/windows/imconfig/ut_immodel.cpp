// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include "gtest/gtest.h"
#include "window/immodel/immodel.h"
class ut_immodel : public ::testing::Test
{
protected:
    ut_immodel()
    {
    }

    virtual ~ut_immodel()
    {
    }

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(ut_immodel, setEdit)
{
    IMModel *immodel = IMModel::instance();
    immodel->setEdit(true);
}

TEST_F(ut_immodel, isEdit)
{
    IMModel *immodel = IMModel::instance();
    immodel->setEdit(true);
    immodel->isEdit();
}

TEST_F(ut_immodel, getIMIndex)
{
    IMModel *immodel = IMModel::instance();
    immodel->getIMIndex("iflyime");
}

TEST_F(ut_immodel, onConfigShow)
{
    IMModel *immodel = IMModel::instance();
    FcitxQtInputMethodItem item;
    item.setUniqueName("iflyime");
    immodel->onConfigShow(item);
}
TEST_F(ut_immodel, getIM)
{
    IMModel *immodel = IMModel::instance();
    FcitxQtInputMethodItem item;
    item.setName("iflyime");
    immodel->getIMIndex(item);
}

TEST_F(ut_immodel, getAvailIMList)
{
    IMModel *immodel = IMModel::instance();
    FcitxQtInputMethodItemList itemList = immodel->getAvailIMList();
}

TEST_F(ut_immodel, getCurIMList)
{
    IMModel *immodel = IMModel::instance();
    FcitxQtInputMethodItemList itemList = immodel->getCurIMList();
}

TEST_F(ut_immodel, onUpdateIMList)
{
    IMModel *immodel = IMModel::instance();
    immodel->onUpdateIMList();
}

TEST_F(ut_immodel, onAddIMItem)
{
    IMModel *immodel = IMModel::instance();
    FcitxQtInputMethodItem item;
    item.setName("iflyime");
    item.setUniqueName("iflyime");
    item.setLangCode("iflyime");
    item.setEnabled(false);
    immodel->onAddIMItem(item);
}

TEST_F(ut_immodel, onDeleteItem)
{
    IMModel *immodel = IMModel::instance();
    FcitxQtInputMethodItem item;
    item.setName("iflyime");
    item.setUniqueName("iflyime");
    item.setLangCode("iflyime");
    item.setEnabled(true);
    immodel->onAddIMItem(item);
    immodel->onUpdateIMList();
    immodel->onDeleteItem(item);
}

TEST_F(ut_immodel, onItemUp)
{
    IMModel *immodel = IMModel::instance();
    FcitxQtInputMethodItem item1, item2;
    item1.setName("iflyime");
    item1.setUniqueName("iflyime");
    item1.setLangCode("iflyime");
    item1.setEnabled(false);
    item2.setName("chineseime");
    item2.setName("chineseime");
    item2.setUniqueName("chineseime");
    item2.setLangCode("chineseime");
    item2.setEnabled(false);
    immodel->onAddIMItem(item1);
    immodel->onAddIMItem(item2);
}

TEST_F(ut_immodel, onItemDown)
{
    IMModel *immodel = IMModel::instance();
    FcitxQtInputMethodItem item1, item2,item3;
    item1.setName("iflyime");
    item1.setUniqueName("iflyime");
    item1.setLangCode("zh");
    item1.setEnabled(false);

    item2.setName("chineseime");
    item2.setUniqueName("chineseime");
    item2.setLangCode("zh");
    item2.setEnabled(false);

    item3.setName("huayupy");
    item3.setUniqueName("huayupy");
    item3.setLangCode("zh");
    item3.setEnabled(false);

    immodel->onAddIMItem(item1);
    immodel->onAddIMItem(item2);
    immodel->onAddIMItem(item3);
    immodel->onItemDown(item2);
}

TEST_F(ut_immodel, equals)
{
//    FcitxQtInputMethodItem item1,item2;
//    item1.setName("pinyin");
//    item1.setLangCode("pinyin");
//    item1.setUniqueName("pinyin");
//    item2.setName("pinyin");
//    item2.setLangCode("pinyin");
//    item2.setUniqueName("pinyin");
//    EXPECT_TRUE(item1 == item2);
}
