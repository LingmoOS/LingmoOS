// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "SideBarImageViewModel.h"
#include "DocSheet.h"
#include "SideBarImageViewModel.h"
#include "PageRenderThread.h"

#include "stub.h"

#include <gtest/gtest.h>
#include <QTest>
#include <QListView>

class TestImagePageInfo_t : public ::testing::Test
{
public:
    TestImagePageInfo_t(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ImagePageInfo_t();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    ImagePageInfo_t *m_tester;
};

TEST_F(TestImagePageInfo_t, initTest)
{

}

TEST_F(TestImagePageInfo_t, test1)
{
    ImagePageInfo_t temp;
    m_tester == &temp;
}

TEST_F(TestImagePageInfo_t, test2)
{
    ImagePageInfo_t temp;
    m_tester < &temp;
}

TEST_F(TestImagePageInfo_t, test3)
{
    ImagePageInfo_t temp;
    m_tester > &temp;
}



class TestSideBarImageViewModel : public ::testing::Test
{
public:
    TestSideBarImageViewModel() {}

public:
    virtual void SetUp()
    {
        QString strPath = UTSOURCEDIR;
        strPath += "/files/1.pdf";
        m_sheet = new DocSheet(Dr::PDF, strPath, nullptr);
        m_tester = new SideBarImageViewModel(m_sheet);
    }

    virtual void TearDown()
    {
        delete m_tester;
        delete m_sheet;
    }

protected:
    DocSheet *m_sheet = nullptr;
    SideBarImageViewModel *m_tester = nullptr;
};

TEST_F(TestSideBarImageViewModel, inittest)
{

}

TEST_F(TestSideBarImageViewModel, testresetData)
{
    m_tester->resetData();
    EXPECT_TRUE(m_tester->m_pagelst.count() == 0);
}

TEST_F(TestSideBarImageViewModel, testinitModelLst)
{
    m_tester->initModelLst(QList<ImagePageInfo_t>() << ImagePageInfo_t(), true);
    EXPECT_TRUE(m_tester->m_pagelst.count() == 1);
}

TEST_F(TestSideBarImageViewModel, testchangeModelData)
{
    m_tester->changeModelData(QList<ImagePageInfo_t>() << ImagePageInfo_t());
    EXPECT_TRUE(m_tester->m_pagelst.count() == 1);
}

TEST_F(TestSideBarImageViewModel, testsetBookMarkVisible)
{
    m_tester->setBookMarkVisible(0, true, true);
    EXPECT_TRUE(m_tester->m_cacheBookMarkMap.count() == 1);
    EXPECT_TRUE(m_tester->m_cacheBookMarkMap[0] == true);
}

TEST_F(TestSideBarImageViewModel, testrowCount)
{
    m_tester->m_pagelst << ImagePageInfo_t();
    EXPECT_TRUE(m_tester->rowCount(QModelIndex()) == 1);
}

TEST_F(TestSideBarImageViewModel, testcolumnCount)
{
    EXPECT_TRUE(m_tester->columnCount(QModelIndex()) == 1);
}

TEST_F(TestSideBarImageViewModel, testdata)
{
    EXPECT_TRUE(m_tester->data(QModelIndex(), Qt::DisplayRole) == QVariant());
}

TEST_F(TestSideBarImageViewModel, testsetData)
{
    EXPECT_TRUE(m_tester->setData(QModelIndex(), "", Qt::DisplayRole) == false);
}

TEST_F(TestSideBarImageViewModel, testgetModelIndexForPageIndex)
{
    m_tester->m_pagelst << ImagePageInfo_t(0);
    EXPECT_TRUE(m_tester->getModelIndexForPageIndex(0).count() == 1);
}

TEST_F(TestSideBarImageViewModel, testgetPageIndexForModelIndex)
{
    m_tester->m_pagelst << ImagePageInfo_t(0);
    EXPECT_TRUE(m_tester->getPageIndexForModelIndex(0) == 0);
}

TEST_F(TestSideBarImageViewModel, testonUpdateImage)
{
    m_tester->onUpdateImage(0);
}

TEST_F(TestSideBarImageViewModel, testinsertPageIndex)
{
    m_tester->insertPageIndex(0);
    EXPECT_TRUE(m_tester->m_pagelst.count() == 1);
}

TEST_F(TestSideBarImageViewModel, testinsertPageIndex1)
{
    m_tester->insertPageIndex(ImagePageInfo_t());
    EXPECT_TRUE(m_tester->m_pagelst.count() == 1);
}

TEST_F(TestSideBarImageViewModel, testremovePageIndex)
{
    m_tester->m_pagelst << ImagePageInfo_t(0);
    m_tester->removePageIndex(0);
    EXPECT_TRUE(m_tester->m_pagelst.count() == 0);
}

TEST_F(TestSideBarImageViewModel, testremoveItemForAnno)
{
    m_tester->removeItemForAnno(nullptr);
    EXPECT_TRUE(m_tester->m_pagelst.count() == 0);
}

TEST_F(TestSideBarImageViewModel, testgetModelIndexImageInfo)
{
    m_tester->m_pagelst << ImagePageInfo_t(0);
    ImagePageInfo_t temp;
    m_tester->getModelIndexImageInfo(0, temp);
    EXPECT_TRUE(temp == ImagePageInfo_t(0));
}

TEST_F(TestSideBarImageViewModel, testfindItemForAnno)
{
    EXPECT_TRUE(m_tester->findItemForAnno(nullptr) == -1);
}

TEST_F(TestSideBarImageViewModel, testhandleRenderThumbnail)
{
    m_tester->handleRenderThumbnail(0, QPixmap());
}
