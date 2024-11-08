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


#ifndef LINGMOSDK_BASE_CONFIGURE_TYPE_H__
#define LINGMOSDK_BASE_CONFIGURE_TYPE_H__

#include <stddef.h>

typedef union KconfigureValue
{
    int asInt;
    long long asLLong;
    double asDouble;
    char *asString;
}KconfigureValue;

typedef enum {
    KconfigureInt,
    KconfigureLLong,
    KconfigureDouble,
    KconfigureString
}KconfigureValueType;

typedef enum {
    KconfigureNodeRoot,
    KconfigureNodeGroup,
    KconfigureNodeNode
}KconfigureNodeType;

typedef struct KconfigureDataNode{
    char *group;
    char *key;
    size_t children_nums;
    size_t max_children_nums;
    char* value;
    // KconfigureValueType valType;
    KconfigureNodeType nodeType;
    struct KconfigureDataNode **children;
}KconfigureDataNode;

#endif
