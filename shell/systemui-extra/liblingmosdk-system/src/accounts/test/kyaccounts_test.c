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

#include <stdio.h>
#include "../libkyaccounts.h"
#include <stdlib.h>
#include <string.h>

int main()
{

    char* ret = kdk_system_create_user("test", "test-pc", 0);
    printf("ret = %s\n", ret);
    free(ret);

    unsigned int err_num;
    bool pass = kdk_system_change_password("test", "qwer$\"1234\"", &err_num);
    printf("pass = %d， err_num = %d\n", pass, err_num);

    bool user = kdk_system_check_has_user("test");
    printf("user = %d\n", user);

    // char *pencrypt = kdk_login_get_public_encrypt();
    // printf("pencrypt = %s\n", pencrypt);

    // char *password = "hhhkkkkjjuojk1234";

    // int len = strlen(password);
    // unsigned char *byte_array = (unsigned char *)malloc(len + 1);
    // if (!byte_array)
    // {
    //     return false;
    // }

    // for (int i = 0; i < len; i++) {  
    //     byte_array[i] = (unsigned char)password[i];
    //     printf("byte_array[i] = %d\n", byte_array[i]);
    // }  
    // byte_array[len] = '\0';

    // bool result = kdk_login_send_password("test", byte_array, len);
    // printf("result = %d\n", result);
}