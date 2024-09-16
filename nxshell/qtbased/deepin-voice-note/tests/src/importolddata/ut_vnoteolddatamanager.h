// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_VNOTEOLDDATAMANAGER_H
#define UT_VNOTEOLDDATAMANAGER_H

#include "gtest/gtest.h"
#include <QTest>
#include <QObject>

class VNoteOldDataManager;
class UT_VNoteOldDataManager : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    UT_VNoteOldDataManager();

    virtual void SetUp() override;
    virtual void TearDown() override;

protected:
    VNoteOldDataManager *m_vnoteolddatamanager {nullptr};
};

#endif // UT_VNOTEOLDDATAMANAGER_H
