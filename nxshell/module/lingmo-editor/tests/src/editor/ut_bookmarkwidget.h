// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEST_BOOKMARKWIDGET_H
#define TEST_BOOKMARKWIDGET_H
#include"../../src/editor/bookmarkwidget.h"
#include "gtest/gtest.h"
#include"../../src/editor/editwrapper.h"
#include"../../src/widgets/window.h"
#include"../../src/startmanager.h"
#include"../../src/editor/dtextedit.h"
#include"../../src/editor/FlashTween.h"
#include"../stub.h"
#include <QObject>
#include<QWindow>
#include<QEvent>


class test_bookmarkwidget: public QObject, public::testing::Test
{
public:
    test_bookmarkwidget();
};

#endif // TEST_BOOKMARKWIDGET_H
