/*
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


#include <libkyconf.h>
#include <stdio.h>
#include <string.h>

int main()
{
    char **keylist = NULL;
    int id = kdk_conf_init("struct.conf");
    ASSERT_NOT_NULL(id, -1);

    char **grouplist = kdk_conf_list_group(id);
    ASSERT_NOT_NULL(grouplist, -1);

    char *tmpgroup;
    int index = 0;
    while ((tmpgroup = grouplist[index]))
    {
        printf("Group: %s\n", tmpgroup);
        keylist = kdk_conf_list_key(id, tmpgroup);
        ASSERT_NOT_NULL(keylist, -1);
        char *tmpkey;
        int k_index = 0;
        while ((tmpkey = keylist[k_index]))
        {
            const char *tmpval = kdk_conf_get_value(id, tmpgroup, tmpkey);
            printf("%s = %s\n", tmpkey , tmpval);
            k_index ++;
        }
        kdk_config_freeall(keylist);

        index ++;
    }

    kdk_conf_destroy(id);
    kdk_config_freeall(grouplist);

    return 0;
}
