// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include "gtest/gtest.h"
#include "window/imaddwindow.h"
class ut_imaddwindow : public ::testing::Test
{
protected:
    ut_imaddwindow()
    {
    }

    virtual ~ut_imaddwindow()
    {
    }

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(ut_imaddwindow, initUI)
{
    IMAddWindow iMAddWindow;
    iMAddWindow.initUI();
}

TEST_F(ut_imaddwindow, initConnect)
{
    IMAddWindow iMAddWindow;
    iMAddWindow.initConnect();
}

TEST_F(ut_imaddwindow, onAddIM)
{
    IMAddWindow iMAddWindow;
    iMAddWindow.initUI();
    iMAddWindow.onAddIM();
}

TEST_F(ut_imaddwindow, updateUI)
{
    IMAddWindow iMAddWindow;
    iMAddWindow.initUI();
    iMAddWindow.updateUI();
}
TEST_F(ut_imaddwindow, onOpenStore)
{
    IMAddWindow iMAddWindow;
    iMAddWindow.onOpenStore();

    sleep(4);
    FILE *fp;
    int pid = -1;
    char unused[70];
    char buf[150];
    char command[150];
    sprintf(command,
            "ps -ef | grep deepin-home-appstore-client | grep -v grep");
    if ((fp = popen(command, "r")) == NULL)
        return ;
    if ((fgets(buf, 150, fp)) != NULL) {
        sscanf(buf, "%69s\t%d\t%69s", unused, &pid, unused);
    }
    pclose(fp);
    if(pid == 32767)
        return ;
    sprintf(command, "kill -9 %d", pid);
    if ((fp = popen(command, "r")) == NULL)
        return ;
    pclose(fp);

    sprintf(command,
            "ps -ef | grep deepin-home-appstore-client | grep -v grep");
    if ((fp = popen(command, "r")) == NULL)
        return ;
    if ((fgets(buf, 150, fp)) != NULL) {
        sscanf(buf, "%69s\t%d\t%69s", unused, &pid, unused);
    }
    pclose(fp);
    if(pid == 32767)
        return ;
    sprintf(command, "kill -9 %d", pid);
    if ((fp = popen(command, "r")) == NULL)
        return ;
    pclose(fp);
    return ;
}
TEST_F(ut_imaddwindow, doRemoveSeleteIm)
{
    IMAddWindow iMAddWindow;
    FcitxQtInputMethodItem item;
    iMAddWindow.doRemoveSeleteIm(item);
}
