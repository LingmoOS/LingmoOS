// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CUSTOMFRAME_H
#define TEST_CUSTOMFRAME_H

#include <QObject>
#include <gtest/gtest.h>
#include "customWidget/customframe.h"

class test_customframe: public::QObject, public::testing::Test
{
public:
    test_customframe();
    ~test_customframe();
protected:
    CustomFrame *mCustomFrame = nullptr;
};

#endif // TEST_CUSTOMFRAME_H
