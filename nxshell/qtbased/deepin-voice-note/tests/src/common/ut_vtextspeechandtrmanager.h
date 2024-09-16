// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_VTEXTSPEECHANDTRMANAGER_H
#define UT_VTEXTSPEECHANDTRMANAGER_H

#include "gtest/gtest.h"
#include <QTest>
#include <QObject>

class VTextSpeechAndTrManager;
class UT_VTextSpeechAndTrManager : public QObject
    , public ::testing::Test
{
    Q_OBJECT
public:
    UT_VTextSpeechAndTrManager();

    virtual void SetUp() override;
    virtual void TearDown() override;

protected:
    VTextSpeechAndTrManager *m_vtextspeechandtrmanager {nullptr};
};

#endif // UT_VTEXTSPEECHANDTRMANAGER_H
