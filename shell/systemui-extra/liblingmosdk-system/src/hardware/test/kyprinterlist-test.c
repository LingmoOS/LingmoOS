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

#include "../libkyprinter.h"
#include <stdio.h>

int main()
{
    int index = 0;
    //获取全部打印机列表
    char **printers = kdk_printer_get_list();
    while (printers[index])
    {
        int status = kdk_printer_get_status(printers[index]);
        printf("%zd: %s , %d\n", index + 1, printers[index], status);
        index++;
    }
    kdk_printer_freeall(printers);
    //获取可用打印机列表
    index = 0;
    char **available_printers = kdk_printer_get_available_list();
    while (available_printers[index])
    {
        int status = kdk_printer_get_status(available_printers[index]);
        printf("%zd: %s , %d\n", index + 1, available_printers[index], status);
        index++;
    }
    kdk_printer_freeall(available_printers);

    return 0;
}
