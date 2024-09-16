// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TEST_CDYNAMICICON_H
#define TEST_CDYNAMICICON_H

#include <QObject>
#include <gtest/gtest.h>
#include "customWidget/cdynamicicon.h"

class test_cdynamicicon: public::QObject, public::testing::Test
{
public:
    test_cdynamicicon();
    ~test_cdynamicicon();
protected:
    CDynamicIcon *mDynamicicon = nullptr;
};

#endif // TEST_CDYNAMICICON_H
