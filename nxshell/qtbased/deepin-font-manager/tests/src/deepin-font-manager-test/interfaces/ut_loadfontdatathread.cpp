// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loadfontdatathread.h"
#include "dfontpreviewlistview.h"
#include "dfmdbmanager.h"
#include "../third-party/stub/stub.h"

#include <QTimer>
#include <QSignalSpy>

#include <gtest/gtest.h>



namespace {
class TestLoadFontDataThread : public testing::Test
{

protected:
    void SetUp()
    {
        fpm = new LoadFontDataThread(list);
    }
    void TearDown()
    {
        delete fpm;
    }
    // Some expensive resource shared by all tests.
    LoadFontDataThread *fpm;
    QList<QMap<QString, QString>> list;
};

QList<DFontPreviewItemData> stub_getFontInfo(void *obj, QList<QMap<QString, QString>>, QList<DFontPreviewItemData> *)
{
    QList<DFontPreviewItemData> list;
    DFontPreviewItemData data;
    data.appFontId = 1;

    list << data;

    return list;
}

}

TEST_F(TestLoadFontDataThread, checkDowork)
{
    Stub stub;
    stub.set((QList<DFontPreviewItemData>(DFMDBManager::*)(QList<QMap<QString, QString>>, QList<DFontPreviewItemData> *))
             ADDR(DFMDBManager, getFontInfo), stub_getFontInfo);

    DFontPreviewListDataThread::instance()->m_fontModelList.clear();
    DFontPreviewItemData ptiemdata;
    fpm->m_delFontInfoList.append(ptiemdata);
    
    fpm->run();
    EXPECT_TRUE(1 == DFontPreviewListDataThread::instance()->m_fontModelList.count());
    EXPECT_FALSE(SignalManager::m_isOnLoad);
    EXPECT_TRUE(SignalManager::m_isDataLoadFinish);
}
