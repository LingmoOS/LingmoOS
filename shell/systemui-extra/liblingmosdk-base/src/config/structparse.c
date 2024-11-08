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


#include "structparse.h"
#include <sdkmarcos.h>
#include <cstring-extension.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define LINEMAX 1024


static KconfigureDataNode* get_group_node(structParse *parse, const char *group)
{
    KconfigureDataNode **grouplist = parse->content.groupList;
    size_t groupSize = parse->content.curGroupSize;
    for (size_t i = 0; i < groupSize; i ++)
    {
        if (strcmp(grouplist[i]->group, group) == 0)
            return grouplist[i];
    }
    return NULL;
}

static KconfigureDataNode* get_key_node(KconfigureDataNode *groupnode, const char *key)
{
    for (int i = 0; i < groupnode->children_nums; i ++)
    {
        if (strcmp(groupnode->children[i]->key, key) == 0)
            return groupnode->children[i];
    }
    return NULL;
}

static KconfigureDataNode* append_group(structParse *parse, const char *group)
{
    if (parse->content.maxGroupSize <= parse->content.curGroupSize)
    {
        KconfigureDataNode **newGroupList = (KconfigureDataNode **)realloc(parse->content.groupList, (parse->content.maxGroupSize + 4) * sizeof(KconfigureDataNode *));
        if (!newGroupList)
        {
            parse->content.curGroupSize --;
            return NULL;
        }

        parse->content.maxGroupSize += 4;
        parse->content.groupList = newGroupList;
    }
    KconfigureDataNode *groupnode = (KconfigureDataNode *)calloc(1, sizeof(KconfigureDataNode));
    if (!groupnode)
        return NULL;

    groupnode->nodeType = KconfigureNodeGroup;
    groupnode->group = (char *)group;
    parse->content.groupList[parse->content.curGroupSize] = groupnode;
    parse->content.curGroupSize ++;

    return groupnode;
}

static KconfigureDataNode* append_key(KconfigureDataNode *groupnode, const char *key, const char *value)
{
    KconfigureDataNode *keynode = NULL;
    for (size_t i = 0; i < groupnode->children_nums; i ++)
    {
        if (strcmp(groupnode->children[i]->key, key) == 0)
        {
            keynode = groupnode->children[i];
            break;
        }
    }

    // 新建key
    if (! keynode)
    {
        if (groupnode->max_children_nums <= groupnode->children_nums)
        {
            KconfigureDataNode **newKeyList = (KconfigureDataNode **)realloc(groupnode->children, (groupnode->max_children_nums + 4) * sizeof(KconfigureDataNode *));
            if (! newKeyList)
                return NULL;
            groupnode->max_children_nums += 4;
            groupnode->children = newKeyList;
        }
    
        keynode = (KconfigureDataNode *)calloc(1, sizeof(KconfigureDataNode));
        if (!keynode)
            return NULL;
        groupnode->children[groupnode->children_nums] = keynode;
        keynode->group = strdup(groupnode->group);
        keynode->key = (char *)key;
        groupnode->children_nums ++;
    }

    keynode->value = (char *)value;
    // keynode->valType = KconfigureString;

    return keynode;
}

static int parseFile(structParse *parse, const char *filepath)
{
    FILE *cfp   = fopen(filepath, "rt");
    ASSERT_NOT_NULL(cfp, -1);

    KconfigureDataNode *cur_group_node = NULL;
    char *tmp_buf = NULL;
    char buf[LINEMAX];
    while (feof(cfp) == 0)
    {
        if (fgets(buf, LINEMAX, cfp) == NULL)
            break;
#ifdef  __linux__
        strstrip(buf, '\n');
        strskipblank(buf);
#elif defined __win32__
        strstrip(buf, '\n');
        strstrip(buf, '\r');
#endif
        if (strlen(buf) == 0)
            continue;

        // Comments
        if (strstartswith(buf, "#") == 0)
            continue;

        // Group
        if (strstartswith(buf, "[") == 0 && strendwith(buf, "]") == 0)
        {
            tmp_buf = strndup(&buf[1], strlen(buf) - 2);
            cur_group_node = append_group(parse, tmp_buf);
            if (!cur_group_node)
            {
                // err output
                fclose(cfp);
                free(tmp_buf);
                return -1;
            }
            continue;
        }

        // Key
        int depos = strfirstof(buf, parse->delimiter.assignDelimiter);
        char *keyStr, *valStr;
        if (depos < 0)
        {
            // 没有赋值符号，可能是什么情况？
            keyStr = buf;
            valStr = "";
            strstripspace(keyStr);
        }
        else
        {
            buf[depos] = 0;
            keyStr = buf;
            valStr = &buf[depos + 1];
            strstripspace(keyStr);
            strstripspace(valStr);
        }

        KconfigureDataNode *groupnode = cur_group_node;
        if (!groupnode)
        {
            // Default group
            char *defaultnode = malloc(36);
            if (! defaultnode)
            {
                // OOM
                fclose(cfp);
                free(cur_group_node);
                free(tmp_buf);
                return -1;
            }
            strcpy(defaultnode, "KDK_DefaultGroup");

            cur_group_node = append_group(parse, defaultnode);
            if(!cur_group_node)
            {
                fclose(cfp);
                free(tmp_buf);
                free(defaultnode);
                return -1;
            }
            else{
                groupnode = cur_group_node;
            }
        }

        // 多重赋值的情况
        size_t multikeys = strcounts(keyStr, parse->delimiter.keyDelimiter);
        if (multikeys)
        {
            multikeys += 1;
            char **keylist = strsplit(keyStr, parse->delimiter.keyDelimiter);
            if (!keylist)
            {
                // OOM
                fclose(cfp);
                free(cur_group_node);
                free(tmp_buf);
                return -1;
            }

            size_t multivals = strcounts(valStr, parse->delimiter.valueDelimiter) + 1;
            char **vallist = strsplit(valStr, parse->delimiter.valueDelimiter);
            if (!vallist)
            {
                // OOM
                free(keylist);
                fclose(cfp);
                free(cur_group_node);
                free(tmp_buf);
                return -1;
            }

            for (size_t i = 0; i < multikeys; i ++)
            {
                char *strkey = strdup(strskipspace(keylist[i]));
                char *val = i >= multivals ? "" : strdup(strskipspace(vallist[i]));

                if (strkey != NULL && val != NULL) {
                    append_key(groupnode, strkey, val);
                } else {
                    if (strkey != NULL) free(strkey);
                    if (val != NULL) free(val);
                }
                // append_key(groupnode, strdup(strskipspace(keylist[i])), strdup(i >= multivals ? "" : strskipspace(vallist[i])));
            }

            free(keylist);
            free(vallist);
        }
        else
        {
            char *strkey = strdup(keyStr);
            char *val = strdup(valStr);

            if (strkey != NULL && val != NULL) {
                append_key(groupnode, strkey, val);
            } else {
                if (strkey != NULL) free(strkey);
                if (val != NULL) free(val);
            }
            // append_key(groupnode, strdup(keyStr), strdup(valStr));
        }
    }

    fclose(cfp);
    return 0;
}

structParse* S_newParse()
{
    structParse *parse  = (structParse *)calloc(1, sizeof(structParse));
    ASSERT_NOT_NULL(parse, NULL);
    parse->delimiter.assignDelimiter = '=';
    parse->delimiter.keyDelimiter = ',';
    parse->delimiter.valueDelimiter = ',';
    return parse;
}

void S_setKeyDelimiter(structParse *parse, char ch)
{
    parse->delimiter.keyDelimiter = ch;
}

void S_setValueDelimiter(structParse *parse, char ch)
{
    parse->delimiter.valueDelimiter = ch;
}

void S_setAssignmentDelimiter(structParse *parse, char ch)
{
    parse->delimiter.assignDelimiter = ch;
}

int S_parseFile(structParse *parse, const char *filename)
{
    if (access(filename, F_OK))
        return -1;

    if (parse->confpath)
    {
        free(parse->confpath);
        parse->confpath = NULL;
    }

    parse->confpath = (char*)malloc(strlen(filename) + 1);
    ASSERT_NOT_NULL(parse->confpath, -1);
    strcpy(parse->confpath, filename);
    return parseFile(parse, parse->confpath);
}

char* const S_getValue(structParse *parse, const char *group, const char *key)
{
    KconfigureDataNode *groupnode = get_group_node(parse, group);

    ASSERT_NOT_NULL(groupnode, "");

    KconfigureDataNode *keynode = get_key_node(groupnode, key);

    ASSERT_NOT_NULL(keynode, "");

    return keynode->value;
}

void S_setValue(structParse *parse, const char *group, const char *key, KconfigureValue value, KconfigureValueType valType)
{
    return;
}

int  S_writeBack(structParse *parse)
{
    return 0;
}

int  S_write2File(structParse *parse, const char *filename)
{
    return 0;
}

void S_destroyParse(structParse **parse)
{
    if (!parse || !*parse)
        return;
    
    SAFE_FREE((*parse)->confpath);


    free(*parse);
}

char** const S_getKeyList(structParse *parse, const char *group)
{
    KconfigureDataNode *pg = get_group_node(parse, group);
    ASSERT_NOT_NULL(pg, NULL);
    char **res = (char **)calloc(1, (pg->children_nums + 1) * sizeof(char *));
    ASSERT_NOT_NULL(res, NULL);

    for (size_t i = 0; i < pg->children_nums; i ++)
    {
        res[i] = pg->children[i]->key;
    }

    return res;
}

char** const S_getGroupList(structParse *parse)
{
    char **res = (char **)calloc(1, (parse->content.curGroupSize + 1) * sizeof(char *));
    ASSERT_NOT_NULL(res, NULL);

    for (size_t i = 0; i < parse->content.curGroupSize; i ++)
    {
        res[i] = parse->content.groupList[i]->group;
    }

    return res;
}
