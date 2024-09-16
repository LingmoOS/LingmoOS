// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define private public
#include "menuworker.h"
#include "menudialog.h"
#undef private

#include <QTest>
#include <QMenu>

#include <gtest/gtest.h>

class Tst_MenuWorker : public testing::Test
{};

TEST_F(Tst_MenuWorker, menuWorker_test)
{
    MenuWorker worker;

    QSignalMapper *signalMapper = new QSignalMapper(worker.m_menu);
    worker.creatMenuByAppItem(worker.m_menu, signalMapper);

    for (int index = 0; index < 7; index++)
        signalMapper->mapped(1);

    worker.m_menu->aboutToHide();

    const QModelIndex index;
    worker.setCurrentModelIndex(index);
    QVERIFY(index == worker.getCurrentModelIndex());

    worker.isMenuVisible();
    worker.isMenuShown();
    worker.onHideMenu();
}

TEST_F(Tst_MenuWorker, menu_action_test)
{
    Menu menu;
    for (int i = 0; i < 5; i++) {
        QAction *action = new QAction(&menu);
        menu.addAction(action);
    }

    menu.setActiveAction(menu.actions().at(0));

    for (int i = 0; i < menu.actions().size(); i++)
        menu.moveUp(i);

    for (int i = 0; i < menu.actions().size(); i++)
        menu.moveDown(i);

    menu.openItem();
}
