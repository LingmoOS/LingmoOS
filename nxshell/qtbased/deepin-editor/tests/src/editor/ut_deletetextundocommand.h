// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEST_TEXTUNDOCOMMAND_H
#define TEST_TEXTUNDOCOMMAND_H
#include"../../src/editor/bookmarkwidget.h"
#include "gtest/gtest.h"
#include"../../src/editor/editwrapper.h"
#include"../../src/widgets/window.h"
#include"../../src/startmanager.h"
#include"../../src/editor/dtextedit.h"
#include"../../src/editor/FlashTween.h"
#include"../../src/editor/deletetextundocommand.h"
#include"../stub.h"
#include <QObject>
#include <QWindow>
#include <QEvent>


class UT_Deletetextundocommond: public QObject, public::testing::Test
{
public:
    UT_Deletetextundocommond();
};

#endif // TEST_TEXTUNDOCOMMAND_H
