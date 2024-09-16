// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_Setting_H
#define UT_Setting_H

#include "gtest/gtest.h"
#include <QTest>
#include <QObject>
#include "../../src/widgets/window.h"
#include "../../src/common/utils.h"

class Settings;
class KeySequenceEdit;
class UT_Setting : public QObject
    , public ::testing::Test
{
    Q_OBJECT

public:
    UT_Setting();
    virtual void SetUp() override;
    virtual void TearDown() override;
    Settings *m_setting;
};

#endif // UT_Setting_H
