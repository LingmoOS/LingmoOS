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


#include <sdkmarcos.h>
#include "skip_linklist.h"
#include <stdlib.h>
#include <time.h>

static lingmosdk_skiplist_node* malloc_skiplist_node(unsigned int maxlvl)
{
    lingmosdk_skiplist_node *res = malloc(sizeof(lingmosdk_skiplist_node));
    ASSERT_NOT_NULL(res, NULL);

    res->data.num = 0;
    res->key = 0;

    res->next = malloc(sizeof(lingmosdk_skiplist_node*) * maxlvl);
    if (! res->next)
    {
        free(res);
        return NULL;
    }

    for (int i = 0; i < maxlvl; i ++)
        res->next[i] = NULL;

    return res;
}

lingmosdk_skiplist* lingmosdk_create_skiplist()
{
    lingmosdk_skiplist *res = malloc(sizeof(lingmosdk_skiplist));
    ASSERT_NOT_NULL(res, NULL);

    res->counts = 0;
    res->max_levels = 3;
    res->children   = calloc(0, sizeof(lingmosdk_skiplist_node *) * res->max_levels);
    if (!res->children)
    {
        free(res);
        return NULL;
    }

    return res;
}

void lingmosdk_destroy_skiplist(lingmosdk_skiplist *list)
{
    // delete all node
    lingmosdk_skiplist_node *curnode = list->children[0];
    while (curnode)
    {
        list->children[0] = curnode->next[0];

        SAFE_FREE(curnode->next);
        SAFE_FREE(curnode);

        curnode = list->children[0];
        list->counts --;
    }

    SAFE_FREE(list->children);
    SAFE_FREE(list);
}

int lingmosdk_skiplist_setmaxlevels(lingmosdk_skiplist *list, unsigned int maxlevels)
{
    if (!list || list->counts)
        return -1;
    
    list->max_levels    = maxlevels;
    lingmosdk_skiplist_node **tmp   = list->children;
    list->children  = realloc(list->children, sizeof(lingmosdk_skiplist_node *) * list->max_levels);
    if (!list->children)
    {
        list->children  = tmp;
        return -1;
    }

    for (int i = 0; i < maxlevels; i ++)
        list->children[i]   = NULL;

    return 0;
}

int lingmosdk_skiplist_insert(lingmosdk_skiplist *list, int key, lingmosdk_listdata data)
{
    lingmosdk_skiplist_node *node = malloc_skiplist_node(list->max_levels);
    ASSERT_NOT_NULL(node, -1);

    node->key   = key;
    node->data  = data;

    if (! list->children[0] || list->children[0]->key > key)
    {
        node->next[0] = list->children[0];
        list->children[0] = node;
    }
    else
    {
        lingmosdk_skiplist_node *prevnode = NULL, *curnode = NULL;
        // 找到插入位置
        for (int i = list->max_levels - 1; i >= 0; i --)
        {
            if (prevnode)
                curnode = prevnode;
            else
                curnode = list->children[i];
            
            if (! curnode || curnode->key > key)
                continue;
            
            while (curnode->next[i] && curnode->next[i]->key <= key)
                curnode = curnode->next[i];
            
            prevnode = curnode;
        }

        if (prevnode)
        {
            node->next[0] = prevnode->next[0];
            prevnode->next[0] = node;
        }
        else
        {
            node->next[0] = list->children[0];
            list->children[0] = node;
        }
    }

    // 决定是否升层
    srand(time(NULL));
    for (int i = 1; i < list->max_levels; i ++)
    {
        if (random() & 1)
            break;
        
        if (! list->children[i] || list->children[i]->key > key)
        {
            node->next[i] = list->children[i];
            list->children[i] = node;
        }
        else
        {
            lingmosdk_skiplist_node *prevnode = list->children[i];
            while (prevnode->next[i] && prevnode->next[i]->key <= key)
                prevnode = prevnode->next[i];
            
            node->next[i] = prevnode->next[i];
            prevnode->next[i] = node;
        }
    }

    list->counts ++;
    return 0;
}

int lingmosdk_skiplist_delete(lingmosdk_skiplist *list, int key)
{
    lingmosdk_skiplist_node *node = NULL;

    if (! list->children[0] || list->children[0]->key > key)
    {
        // no target found
        return -1;
    }
    else
    {
        lingmosdk_skiplist_node *prevnode = NULL, *curnode = NULL;
        // 找到删除位置
        for (int i = list->max_levels - 1; i >= 0; i --)
        {
            if (prevnode)
                curnode = prevnode;
            else
                curnode = list->children[i];

            if(curnode && curnode->key == key)
            {
                node = curnode;
                list->children[i] = curnode->next[i];
                continue;
            }
            
            if (! curnode || curnode->key > key)
                continue;
            
            while (curnode->next[i] && curnode->next[i]->key < key)
                curnode = curnode->next[i];
            
            prevnode = curnode;

            if (prevnode->next[i] && prevnode->next[i]->key == key)
            {
                node = prevnode->next[i];
                prevnode->next[i] = prevnode->next[i]->next[i];
            }
        }
    }

    if (node)
    {
        SAFE_FREE(node->next);
        SAFE_FREE(node);
        list->counts --;
        return 0;
    }

    return -1;
}

lingmosdk_listdata lingmosdk_skiplist_search(lingmosdk_skiplist *list, int key)
{
    lingmosdk_skiplist_node *curnode = NULL, *res = NULL;

    for (int i = list->max_levels - 1; i >= 0; i --)
    {
        if (!list->children[i] || list->children[i]->key > key)
            continue;
        
        if (! curnode)
            curnode = list->children[i];
        
        while (curnode->next[i] && curnode->next[i]->key <= key)
            curnode = curnode->next[i];
        
        if (curnode && curnode->key == key)
        {
            res = curnode;
            break;
        }
    }

    if (res)
        return res->data;
    
    return (lingmosdk_listdata)-1;
}
