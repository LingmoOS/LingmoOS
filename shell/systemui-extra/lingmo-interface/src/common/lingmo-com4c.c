/*
 * Copyright (C) 2021 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "lingmo-com4c.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <iniparser/iniparser.h>

#include "lingmo-ini.h"

#define LSB_RELEASE_FILE "/etc/lsb-release"	    /* lsb-release文件路径 */
#define OS_RELEASE_FILE "/etc/os-release"	    /* os-release文件路径 */
#define KYINFO_FILE "/etc/.kyinfo"	            /* kyinfo文件路径 */
#define CPUINFO_FILE "/proc/cpuinfo"	        /* cpuinfo文件路径 */
#define RELEASEFILE_LINE_MAX		256			/* 最大解析行行数 */
#define RELEASEFILE_LINE_MAX_LEN	256		    /* 最大解析行长度 */
#define RELEASEFILE_KEY_MAX_LEN		128			/* 最大key长度 */ 
#define RELEASEFILE_VALUE_MAX_LEN	256         /* 最大value长度 预留'\0', '='*/
#define RELEASEFILE_SPLIT_EQUAL		'='			/* key和value的分隔符 */
#define RELEASEFILE_SPLIT_COLON		':'			/* key和value的分隔符 */
#define PROJECT_CODENAME    "PROJECT_CODENAME"
#define CPUINFO_MODELNAME      "model name"

// 根据key获取value，成功返回 >0 否则 返回 <= 0
static int file_get_keyvalue(const char *path, const char *key, char *value, int value_max_len, const char chSplit)
{
    if (path == NULL || key == NULL || value == NULL || value_max_len <= 0) {
        return -1;
    }        
	int ret = 0;
	int cnt = 0;
	int key_len = 0; 
	int line_len = 0;
	int value_len = 0;
	FILE *fp = NULL;
	char line_buf[RELEASEFILE_LINE_MAX_LEN] = {0}; 
	
	/* 合法性判断 */
	if (NULL == path || NULL == key || NULL == value || 0 >= value_max_len) {
		return -1;
	}
	
	key_len = strlen(key);
	if (0 >= key_len || RELEASEFILE_KEY_MAX_LEN < key_len) {
		return -2;
	}

	if (F_OK != access(path, F_OK)) {
		return -3;
	}

	/* 打开文件 */
	fp = fopen(path, "r");
	if (NULL == fp) {
		return -4;
	}

	/* 遍历行，匹配Key */
	while(NULL != fgets(line_buf, RELEASEFILE_LINE_MAX_LEN-1, fp)) {
		line_len = strlen(line_buf);
		if (line_len > key_len) {
			/* 找到key */
            //找到非空格的首字符
            char *pStart = line_buf;
            int nSplitIndex = 0;
            while(pStart != (line_buf+line_len) && isspace(*pStart)) { 
                ++pStart; ++nSplitIndex;
            }
			if (0 == strncmp(pStart, key, key_len)) {
                pStart = pStart + key_len;
                nSplitIndex = nSplitIndex + key_len;
                //跳过空格字符
                while(pStart != (line_buf+line_len) && isspace(*pStart)) { 
                    ++pStart; ++nSplitIndex;
                }
                if (pStart != (line_buf+line_len) && *pStart == chSplit) {
                    ret = 1;
                    key_len = nSplitIndex;
                    break;
                }
			}
		}
		memset(line_buf, 0, sizeof(line_buf));
		
		/* 最大查找行限制 */
		cnt++;
		if (RELEASEFILE_LINE_MAX < cnt) {
			break;
		}
	}

	/* 关闭文件 */
	fclose(fp);
    fp = NULL;
	
	/* 找到key，返回value */
	if (1 == ret) {
		value_len = line_len - key_len - 1;
		if (0 < value_len) {
			if (value_len > value_max_len) {
				value_len = value_max_len;
			}
			/* 拷贝value */ 
			snprintf(value, value_max_len, "%s", &line_buf[key_len + 1]);
			value[value_len] = '\0';
			
			/* 去掉结尾的换行符 */
            strstrip(value, 1);
		} else {
			value[0] = '\0';
		}
	}
	return ret;
}

/**
 * @fn kdk_get_xdgsessiontype
 * @brief 获取区分x11与wayland的环境变量值
 * @retval char* !NULL 成功 否则失败
 */
char* kdk_get_xdgsessiontype()
{
    return getenv("XDG_SESSION_TYPE");
}

int kdk_get_lsbrelease(const char *key, char *value, int value_max_len)
{
	int nRet = -1;
	if (NULL == key || NULL == value || 0 >= value_max_len)
		return nRet;
	dictionary *iniHandle = iniparser_load(LSB_RELEASE_FILE);
	if (iniHandle) {
		char sessionKey[256] = {0};
		snprintf(sessionKey, 256, ":%s", key);
		const char *tempValue = iniparser_getstring(iniHandle, sessionKey, "");
		snprintf(value, value_max_len, "%s", tempValue);
		iniparser_freedict(iniHandle);
		iniHandle = NULL;
		nRet = strlen(value);
	}
    return nRet;
}

int kdk_get_osrelease(const char *key, char *value, int value_max_len)
{
    int nRet = -1;
	if (NULL == key || NULL == value || 0 >= value_max_len)
		return nRet;
	dictionary *iniHandle = iniparser_load(OS_RELEASE_FILE);
	if (iniHandle) {
		char sessionKey[256] = {0};
		snprintf(sessionKey, 256, ":%s", key);
		const char *tempValue = iniparser_getstring(iniHandle, sessionKey, "");
		snprintf(value, value_max_len, "%s", tempValue);
		iniparser_freedict(iniHandle);
		iniHandle = NULL;
		nRet = strlen(value);
	}
    return nRet;
}

int kdk_get_prjcodename(char *value, int value_max_len)
{
    int nResult = kdk_get_lsbrelease(PROJECT_CODENAME, value, value_max_len);
    if (nResult <= 0) {
        nResult = kdk_get_osrelease(PROJECT_CODENAME, value, value_max_len);
    }
    return nResult;
}

int kdk_get_kyinfo(const char *session, const char *key, char *value, int value_max_len)
{
    int nRet = -1;
	if (NULL == session || NULL == key || NULL == value || 0 >= value_max_len)
		return nRet;
	dictionary *iniHandle = iniparser_load(KYINFO_FILE);
	if (iniHandle) {
		char sessionKey[256] = {0};
		snprintf(sessionKey, 256, "%s:%s", session, key);
		const char *tempValue = iniparser_getstring(iniHandle, sessionKey, "");
		snprintf(value, value_max_len, "%s", tempValue);
		iniparser_freedict(iniHandle);
		iniHandle = NULL;
		nRet = strlen(value);
	}
    return nRet;
}

int kdk_get_cpumodelname(char *modelName, int max_len)
{
    return file_get_keyvalue(CPUINFO_FILE, CPUINFO_MODELNAME, modelName, max_len, RELEASEFILE_SPLIT_COLON);
}

int kdk_get_spechdplatform(char *platformName, int max_len)
{
    if (platformName == NULL || max_len <= 0)
        return -1;
    char strDefault[128] = "default";
    int validLen = strlen(strDefault);
    validLen = validLen > max_len ? max_len : validLen;
	snprintf(platformName, max_len, "%s", strDefault);
    return validLen;
}