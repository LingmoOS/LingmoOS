// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Server.h"

extern "C" {
#include <wlr/util/log.h>
}

#include <iostream>

int main()
{
    wlr_log_init(WLR_DEBUG, NULL);

    Server dimwl;
    dimwl.addSocket("imfakewl");
    dimwl.startBackend();
    dimwl.run();
}
