// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_StartManager_H
#define UT_StartManager_H
#include"../../src/startmanager.h"
#include"../../src/editor/dtextedit.h"
#include"../../src/common/settings.h"
#include"../../src/widgets/window.h"
#include"../../src/editor/editwrapper.h"
#include"../../src/controls/tabbar.h"
#include "gtest/gtest.h"
#include <QObject>

class UT_StartManager: public QObject, public::testing::Test
{
        Q_OBJECT
public:
    UT_StartManager();
};

#endif // UT_StartManager_H
