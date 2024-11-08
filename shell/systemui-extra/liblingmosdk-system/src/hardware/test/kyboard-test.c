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

#include "stdio.h"
#include "../libkyboard.h"

int main()
{
    
    char *name = kdk_board_get_name();
    char *vendor = kdk_board_get_vendor();
    char *date = kdk_board_get_date();
    char *serial = kdk_board_get_serial();
    printf("name : %s", name);
    printf("vendor : %s", vendor);
    printf("date : %s", date);
    printf("serial : %s", serial);
    kdk_board_free(name);
    kdk_board_free(vendor);
    kdk_board_free(date);
    kdk_board_free(serial);
    return 0;
}
