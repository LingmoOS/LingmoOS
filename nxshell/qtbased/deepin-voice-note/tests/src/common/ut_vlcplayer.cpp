// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ut_vlcplayer.h"
#include "common/vlcpalyer.h"

UT_VlcPalyer::UT_VlcPalyer()
{
}

TEST_F(UT_VlcPalyer, UT_VlcPalyer_setPosition_001)
{
    VlcPalyer player;
    player.setPosition(1);
}

TEST_F(UT_VlcPalyer, UT_VlcPalyer_play_001)
{
    VlcPalyer player;
    player.play();
}

TEST_F(UT_VlcPalyer, UT_VlcPalyer_pause_001)
{
    VlcPalyer player;
    player.pause();
}

TEST_F(UT_VlcPalyer, UT_VlcPalyer_stop_001)
{
    VlcPalyer player;
    player.stop();
}

TEST_F(UT_VlcPalyer, UT_VlcPalyer_getState_001)
{
    VlcPalyer player;
    player.getState();
}
