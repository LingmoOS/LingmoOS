/*
* liblingmosdk-diagnosetest's Library
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
* Authors: Zhen Sun <sunzhen1@kylinos.cn>
*
*/
#ifndef LIBKYDATACOLLECT
#define LIBKYDATACOLLECT

#include <stdbool.h>

#ifdef __cplusplus
    extern "C"{
#endif

/**
 * @brief 事件类型
 * 
 */
typedef enum
{
    KEVENT_CLICK = 0,
    KEVENT_VISIT = 1,
    KEVENT_SEARCH = 2,
    KEVENT_EXPOSURE = 3,
    KEVENT_STARTUP = 4,
    KEVENT_EXIT = 5,
    KEVENT_SHARE = 6,
    KEVENT_CUSTOM = 100
} KEventType;

/**
 * @brief 终端类型 0，桌面应用；1，web应用
 * 
 */
typedef enum
{
    KEVENTSOURCE_DESKTOP = 0,
    KEVENTSOURCE_WEB = 1
} KEventSourceType;

/**
 * @brief 扩展属性，可为事件添加额外属性
 * 
 */
typedef struct 
{
    char* key;
    char* value;
}KCustomProperty;

/**
 * @brief 全局属性，由接口自动填充
 * 
 */
typedef struct 
{
    char* umid; //机器码
    char* userId; //用户Id
    char* sourceIp; //ip地址
    char* sourceMac; //mac地址
    char* serviceKey; //服务序列号
}KGlobalProperty;

typedef struct {
    char* eventCode; //必填
    KEventType eventType; //必填
    KEventSourceType eventSourceType; //必填
    char timeStamp[14]; //自动填充
    char* pageName; //必填
    char* pageUrl; //非必填
    char* pagePath; //非必填
    int duration; //自动填充
    char* appName; //自动填充
    char* appVersion; //自动填充
    char* searchWord; //搜索事件必填
    char* entrancePageFlag; //非必填
    char* exitPageFlag; //非必填
    KCustomProperty* customProperty; //非必填
    KGlobalProperty* globalProperty; //自动填充
    int customPropertyLength; //自动填充
}KTrackData;

/**
 * @brief 初始化数据结构，完成部分字段的自动填充
 * 
 * @param sourceType 应用类型 0，桌面应用 1，web应用 
 * @param evnetType 事件类型 
 * @return KTrackData*  返回初始化后的结构体 
 */
extern KTrackData* kdk_dia_data_init(KEventSourceType sourceType, KEventType evnetType);

/**
 * @brief 释放结构体内存
 * 
 * @param node 
 */
extern void kdk_dia_data_free(KTrackData* node);

/**
 * @brief 上传程序启动事件
 * 
 */
extern void kdk_dia_upload_program_startup();

/**
 * @brief 上传程序退出事件
 * 
 */
extern void kdk_dia_upload_program_exit();

/**
 * @brief 
 * 
 * @param node 
 * @param eventCode 事件描述
 * @param pageName 所在界面名称
 * @param content 搜索内容
 */
extern void kdk_dia_upload_search_content(KTrackData*node,char* eventCode, char* pageName, char*content);

/**
 * @brief 默认事件上传接口，只包含所有必填字段和自动填充字段
 * 
 * @param node 
 * @param eventCode 事件描述 
 * @param pageName 所在界面名称 
 */
extern void kdk_dia_upload_default(KTrackData*node,char* eventCode, char* pageName);

/**
 * @brief 添加页面信息，包括url和path
 * 
 * @param node 
 * @param pageUrl 页面url
 * @param pagePath url去除域名后的内容 
 */
extern void kdk_dia_append_page_info(KTrackData* node, char* pageUrl, char* pagePath);

/**
 * @brief 添加页面标识
 * 
 * @param node 
 * @param is_entrancePage 入口页标识,是否会话的入口页面
 * @param is_exitPageFlag 出口页标识，是否访问会话的退出页面
 */
extern void kdk_dia_append_page_flag(KTrackData*node, bool is_entrancePage, bool is_exitPageFlag);

/**
 * @brief 事件自定义属性,map对象,key-value键值对
 * 
 * @param node 
 * @param property 自定义属性
 * @param property_length 自定义属性数量
 */
extern void kdk_dia_append_custom_property(KTrackData*node,KCustomProperty* property, int property_length);

#ifdef __cplusplus
    }
#endif

#endif