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


#ifndef LINGMOSDK_BASE_CONFIGURE_STRUCT_H__
#define LINGMOSDK_BASE_CONFIGURE_STRUCT_H__

#include "datatype.h"

typedef struct {
    KconfigureDataNode **groupList;  // Group List
    size_t maxGroupSize;
    size_t curGroupSize;
}structContenct;

typedef struct {
    struct {
        char keyDelimiter;
        char valueDelimiter;
        char assignDelimiter;
    }delimiter;

    char *confpath;
    structContenct content;
}structParse;

extern structParse* S_newParse();
extern void S_setKeyDelimiter(structParse *parse, char ch);
extern void S_setValueDelimiter(structParse *parse, char ch);
extern void S_setAssignmentDelimiter(structParse *parse, char ch);
extern int S_parseFile(structParse *parse, const char *filename);
extern char* const S_getValue(structParse *parse, const char *group, const char *key);
extern void S_setValue(structParse *parse, const char *group, const char *key, KconfigureValue value, KconfigureValueType valType);
extern char** const S_getKeyList(structParse *parse, const char *group);
extern char** const S_getGroupList(structParse *parse);
extern int  S_writeBack(structParse *parse);
extern int  S_write2File(structParse *parse, const char *filename);
extern void S_destroyParse(structParse **parse);

#endif
