// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_VNOTEITEMOPER_H
#define UT_VNOTEITEMOPER_H

#include "gtest/gtest.h"
#include <QTest>
#include <QObject>

class VNoteItemOper;
struct VNoteItem;
class UT_VNoteItemOper : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    UT_VNoteItemOper();

    virtual void SetUp() override;
    virtual void TearDown() override;
    VNoteItem *m_note {nullptr};

protected:
    VNoteItemOper *m_vnoteitemoper {nullptr};
};

#endif // UT_VNOTEITEMOPER_H
