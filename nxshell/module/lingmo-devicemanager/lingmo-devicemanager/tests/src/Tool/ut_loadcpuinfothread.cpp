// Copyright (C) 2019 ~ 2020 UnionTech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "DeviceWidget.h"
#include "DeviceManager.h"
#include "X86Generator.h"
#include "LoadCpuInfoThread.h"
#include "ut_Head.h"
#include "stub.h"

#include <QCoreApplication>
#include <QPaintEvent>
#include <QPainter>

#include <gtest/gtest.h>

class LoadCpuInfoThread_UT : public UT_HEAD
{
public:
    void SetUp()
    {
        m_loadCpuInfoThread = new LoadCpuInfoThread;
    }
    void TearDown()
    {
        delete m_loadCpuInfoThread;
    }
    LoadCpuInfoThread *m_loadCpuInfoThread;
};

const QList<QMap<QString, QString>>& ut_LoadCpuInfoThread_cmdInfo()
{
    static QList<QMap<QString, QString>> list;
    list.clear();
    QMap<QString, QString> map;
    map.insert("/", "/");
    list.append(map);
    return list;
}

TEST_F(LoadCpuInfoThread_UT, LoadCpuInfoThread_UT_getCpuInfoFromLscpu)
{
    Stub stub;
    stub.set(ADDR(DeviceManager, cmdInfo), ut_LoadCpuInfoThread_cmdInfo);
    m_loadCpuInfoThread->run();
}
