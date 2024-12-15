// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "blurboxwidget.h"
#include "appslistmodel.h"

#define private public
#include "scrollwidgetagent.h"
#undef private

#include <QApplication>
#include <QMouseEvent>
#include <QKeyEvent>

#include <gtest/gtest.h>

class Tst_Scrollwidgetagent : public testing::Test
{};

TEST_F(Tst_Scrollwidgetagent, scrollWidgetAgent_test)
{
    ScrollWidgetAgent agent;
    QWidget *w = new QWidget;
    agent.setControlWidget(w);

    BlurBoxWidget *blueBox = new BlurBoxWidget(AppsListModel::Chat, const_cast<char *>("Chat"), w);
    agent.setPosType(PosType::Pos_LL);
    agent.setBlurBoxWidget(blueBox);
    agent.setPosType(PosType::Pos_L);
    agent.setBlurBoxWidget(blueBox);
    agent.setPosType(PosType::Pos_M);
    agent.setBlurBoxWidget(blueBox);
    agent.setPosType(PosType::Pos_R);
    agent.setBlurBoxWidget(blueBox);
    agent.setPosType(PosType::Pos_RR);
    agent.setBlurBoxWidget(blueBox);
}
