// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_VNOTEFORLDER_H
#define UT_VNOTEFORLDER_H

#include "gtest/gtest.h"
#include <QTest>
#include <QObject>

struct VNoteFolder;

class UT_VNoteFolder : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    UT_VNoteFolder();

    virtual void SetUp() override;
    virtual void TearDown() override;

protected:
    VNoteFolder *m_vnoteforlder {nullptr};
};

#endif // UT_VNOTEFORLDER_H
