// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_FILECLEANUPWORKER_H
#define UT_FILECLEANUPWORKER_H

#include "filecleanupworker.h"
#include "common/vnoteitem.h"
#include "gtest/gtest.h"
#include <QTest>
#include <QObject>

class UT_FileCleanupWorker : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    UT_FileCleanupWorker();

protected:
    virtual void SetUp() override;
    virtual void TearDown() override;

private:
    VNOTE_ALL_NOTES_MAP *qspAllNotesMap {nullptr};
    VNOTE_ITEMS_MAP *voiceItem {nullptr};
};

#endif // UT_FILECLEANUPWORKER_H
