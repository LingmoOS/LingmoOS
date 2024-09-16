// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include <mprisplayer.h>

int main(int argc, char *argv[])
{
    MprisPlayer *mprisPlayer = new MprisPlayer();
    // deepin fork mpris
    mprisPlayer->setCanShowInUI(false);
    return 0;
}
