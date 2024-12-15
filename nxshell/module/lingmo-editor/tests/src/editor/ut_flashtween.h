// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UT_FlashTween_H
#define UT_FlashTween_H
#include "gtest/gtest.h"
#include"../../src/editor/editwrapper.h"
#include"../../src/widgets/window.h"
#include"../../src/startmanager.h"
#include"../../src/editor/dtextedit.h"
#include"../../src/editor/FlashTween.h"
#include"../stub.h"
#include <QObject>
#include <QWindow>
#include <QEvent>
#include <QTimer>

class UT_FlashTween : public QObject, public::testing::Test
{
public:
    UT_FlashTween();
};

#endif // UT_FlashTween_H
