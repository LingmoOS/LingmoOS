/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MODEL_CONFIG_H
#define MODEL_CONFIG_H

#include "remotemodelvendor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SUCCEEDED = 0,

    FAILED = 1
} ModelConfigResult;

// 获取当前系统远端模型供应商列表的长度
unsigned int get_remote_model_vendor_list_length();
// 获取当前系统远端模型供应商列表
const RemoteModelVendorId* get_remote_model_vendor_list();

// 获取指定供应商的远端模型列表的长度
unsigned int get_remote_model_list_length(RemoteModelVendorId vendor);
// 获取指定供应商的远端模型列表
const RemoteModelId* get_remote_model_list(RemoteModelVendorId vendor);

// 使用json字符串设置远端模型配置的参数。不同远端模型供应商的
// 配置参数可能不一样。以百度为例，json中需要提供apiKey和secretKey
ModelConfigResult set_remote_model_config(RemoteModelVendorId vendor, 
    RemoteModelId modelId, const char* json);

// 删除指定模型供应商指定的远端模型的配置
ModelConfigResult remove_remote_model_config(RemoteModelVendorId vendor,
    RemoteModelId modelId);

// 获取全部远端模型供应商的模型配置信息的字节长度
unsigned int get_remote_model_config_length();
// 1. 获取全部远端模型供应商的模型配置信息，
// SDK会将get_remote_model_config_length()长度的
// 字节信息复制到json指向的地址中
// 2. 调用方需要确保json指向的地址存在足够的内存空间，
// 防止出现写入越界
ModelConfigResult get_remote_model_config(char* json);

// 获取默认的远端模型供应商
RemoteModelVendorId get_default_remote_model_vendor();
// 设置默认的远端模型供应商
void set_default_remote_model_vendor(RemoteModelVendorId vendor);

#ifdef __cplusplus
}
#endif

#endif