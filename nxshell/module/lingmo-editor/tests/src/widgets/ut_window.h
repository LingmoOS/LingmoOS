// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_Window_H
#define UT_Window_H
#include "gtest/gtest.h"
#include "../../src/editor/editwrapper.h"
#include "../../src/widgets/window.h"
#include "../../src/startmanager.h"
#include "../../src/editor/dtextedit.h"
#include "../stub.h"
#include "../../src/controls/replacebar.h"
#include <QObject>
#include<QWindow>
#include<QEvent>


class UT_Window: public QObject, public::testing::Test
{
public:
    UT_Window();
    EditWrapper * edit;
    Window * window;
};

#endif // UT_Window_H
