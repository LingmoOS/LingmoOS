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


#include <skip_linklist.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

void print_list(lingmosdk_skiplist *list)
{
    for (int i = list->max_levels - 1; i >= 0; i --)
    {
        lingmosdk_skiplist_node *curnode = list->children[i];
        printf("Level[%d]: ", i);
        while (curnode)
        {
            printf("%d->", curnode->key);
            curnode = curnode->next[i];
        }
        printf("NULL\n");
    }
}

int test_list_order(lingmosdk_skiplist *list)
{
    lingmosdk_skiplist_node *node = list->children[0];
    while (node)
    {
        if (node->next[0])
        {
            if (node->key > node->next[0]->key)
            {
                printf("Skiplist order test failed.\n");
                return -1;
            }
        }
        node = node->next[0];
    }

    printf("Skiplist order test pass.\n");

    return 0;
}

int main()
{
    lingmosdk_skiplist *list = lingmosdk_create_skiplist();

    lingmosdk_skiplist_setmaxlevels(list, 5);
    
    srand(time(NULL));
    for (int i = 0; i < 100; i ++)
    {
        int num = random() % 500 + 1;
        lingmosdk_skiplist_insert(list, num, (lingmosdk_listdata)i);
        printf("%d has been insert.\n", num);
        sleep(1);
    }
    
    print_list(list);

    int res = test_list_order(list);

    lingmosdk_destroy_skiplist(list);

    return res;
}
