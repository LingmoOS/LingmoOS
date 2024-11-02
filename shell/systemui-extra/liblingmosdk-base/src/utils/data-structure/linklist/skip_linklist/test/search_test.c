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

int test_list_search(lingmosdk_skiplist *list)
{
    for (int i = 0; i < 5; i ++)
    {
        int target = random() % 50 + 1;
        lingmosdk_listdata data = lingmosdk_skiplist_search(list, target);
        printf("Searching for %d, data is %d\n", target, data.num);

        lingmosdk_skiplist_node *curnode = list->children[0];
        short finded = 0;
        while (curnode)
        {
            if (curnode->key == target)
            {
                finded = 1;
                break;
            }
            curnode = curnode->next[0];
        }
        
        if (finded && data.num == -1)
        {
            printf("Skiplist search test failed.\n");
            return -1;
        }
    }

    printf("Skiplist search test pass.\n");

    return 0;
}

int main()
{
    lingmosdk_skiplist *list = lingmosdk_create_skiplist();

    lingmosdk_skiplist_setmaxlevels(list, 5);
    
    srand(time(NULL));
    for (int i = 0; i < 20; i ++)
    {
        int num = random() % 50 + 1;
        lingmosdk_listdata data;
        data.num = i;
        lingmosdk_skiplist_insert(list, num, data);
        printf("[%d] %d has been insert.\n", i, num);
        sleep(1);
    }
    
    print_list(list);

    int res = test_list_search(list);

    lingmosdk_destroy_skiplist(list);

    return res;
}
