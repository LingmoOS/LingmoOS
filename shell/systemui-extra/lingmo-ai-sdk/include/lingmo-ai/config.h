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

#ifndef AI_CONFIG_H
#define AI_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

enum ErrorCode {
    CONFIG_SUCCEEDED = 0,
    CONFIG_FAILED = 1
};

enum Capability {
    CAPABILITY_INVALID = -1,

    CAPABILITY_FIRST = 0,
    CAPABILITY_NLP = CAPABILITY_FIRST,
    CAPABILITY_SPEECH = 1,
    CAPABILITY_VISION = 2,

    CAPABILITY_COUNT
};

enum DeployPolicy {
    DEPLOY_POLICY_INVALID = -1,
    DEPLOY_POLICY_FIRST = 0,
    LOCAL = DEPLOY_POLICY_FIRST,
    PUBLIC_CLOUD = 1,
    PRIVATE_CLOUD = 2,

    DEPLOY_POLICY_COUNT
};

// 获取能力列表长度
unsigned long get_capability_list_length();
// 获取能力列表。调用方需要确保参数list指向的内存
// 必须具有大于等于get_capability_list_length()*sizeof(Capability)
ErrorCode get_capability_list(Capability* list);

// 重置AI能力可用性开关为默认值
void global_settings_reset_enabled();
// 获取AI能力可用性开关的状态
int global_settings_is_enabled();
// 设置AI能力可用性开关的状态
int global_settings_set_enabled(int enabled);

// 重置某种AI具体能力可用性开关为默认值
void capability_settings_reset_enabled(Capability capability);
// 获取某种AI具体能力可用性开关的状态
int capability_settings_is_enabled(Capability capability);
// 设置某种AI具体能力可用性开关的状态
int capability_settings_set_enabled(Capability capability, int enabled);
// 重置某种AI具体能力的部署策略为默认值
void capability_settings_reset_deploy_policy(Capability capability);
// 获取某种AI具体能力的部署策略
DeployPolicy capability_settings_get_deploy_policy(Capability capability);
// 设置某种AI具体能力的部署策略
int capability_settings_set_deploy_policy(Capability capability, DeployPolicy policy);
//获取某种AI具体能力支持的模型列表的长度
unsigned long capability_settings_get_model_list_length(Capability capability, DeployPolicy policy);
// 获取指定索引对应的某种具体能力模型的字节长度
unsigned long capability_settings_get_model_length_by_index(Capability capability, DeployPolicy policy, unsigned long index);
// 获取指定索引对应的某种具体能力模型。调用方需要确保model指向的
// 内存具有足够的空间来容纳capability_settings_get_model_length_by_index(index, policy)
// 数量的char数据
ErrorCode capability_settings_get_model_by_index(Capability capability, DeployPolicy policy, unsigned long index, char* model);
// 重置某种AI具体能力使用的模型为默认值
void capability_settings_reset_model(Capability capability, DeployPolicy policy);
// 获取某种AI具体能力使用的模型的字节长度
unsigned long capability_settings_get_model_length(Capability capability, DeployPolicy policy);
// 获取某种AI具体能力使用的模型。调用方需要确保model
// 指向的内存的大小应该大于等于capability_settings_get_model_length()
ErrorCode capability_settings_get_model(Capability capability, DeployPolicy policy, char* model);
// 设置某种AI具体能力使用的模型。调用方需要确保传入的模型存在于
// capability_settings_get_model_by_index()函数返回的模型列表中，否则，
// 下面的函数会调用失败
ErrorCode capability_settings_set_model(Capability capability, DeployPolicy policy, const char* model);
// 重置某种AI具体能力使用的某种模型的配置为默认值
void capability_settings_reset_model_config(Capability capability, DeployPolicy policy, const char* model);
// 获取某种AI具体能力使用的某种模型的配置的字节长度
unsigned long capability_settings_get_model_config_length(Capability capability, DeployPolicy policy, const char* model);
// 获取某种AI具体能力使用的某种模型的配置。调用方需要确保config
// 指向的内存的大小应该大于等于capability_settings_reset_model_config_length()
ErrorCode capability_settings_get_model_config(Capability capability, DeployPolicy policy, const char* model, char* json);
// 设置某种AI具体能力使用的某种模型的配置。根据当前使用的公有云模型的不同，
// json内容的具体key和value是不一样的
// 
// 目前百度平台所有能力的配置是：
// {
//     "appId": "xxx",
//     "apiKey": "xxx",
//     "secretKey": "xxx"
// }
//
// 科大讯飞平台音频相关的配置比较特殊：
// {
//     "appId": "xxx",
//     "apiKeyRealTime": "xxx", // 实时语音识别
//     "apiKeyContinuous": "xxx", // 流式语音识别/合成
//     "secretKeyContinuous": "xxx" // 流式语音识别/合成
// }
// 科大讯飞其他能力的配置：
// {
//     "appId": "xxx",
//     "apiKey": "xxx",
//     "secretKey": "xxx"
// }
ErrorCode capability_settings_set_model_config(Capability capability, DeployPolicy policy, const char* model, const char* json);
//获取某种AI具体能力支持的模型的密钥配置JSON字符串的长度
unsigned long capability_settings_get_model_key_config_length(Capability capability, DeployPolicy policy, const char* model);
// 获取某种AI具体能力支持的模型的密钥配置JSON字符串。调用方需要确保json指向的
// 内存具有足够的空间来容纳capability_settings_get_model_key_config_length(index, policy, model)
// 数量的char数据
ErrorCode capability_settings_get_model_key_config(Capability capability, DeployPolicy policy, const char* model, char* json);

#ifdef __cplusplus
}
#endif
#endif
