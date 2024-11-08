/*
 * liblingmosdk-system's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */

#include <libkylockscreen.h>
#include <stdio.h>

int main(void)
{
    char appName[] = "test-package";
    char reason[] = "test-reason";

    uint32_t flag;
    if ((flag = kdk_set_inhibit_lockscreen(appName , reason)) == 0) {
        printf("set inhibit lock screen fail !\n");
        return -1;
    }

    if (kdk_un_inhibit_lockscreen(flag)) {
        printf("un inhibit lock screen fail !\n");
        return -1;
    }

    return 0;
}
