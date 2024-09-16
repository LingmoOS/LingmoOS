// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDE_CRYPTO_H
#define DDE_CRYPTO_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "dde_tc.h"

/*****************************************
 * @brief 初始化加解密环境
 * @param[out] handle TC句柄
 * @return 是否成功
 * ***************************************/
bool cryptoInit(TC_HANDLE* handle);

/*****************************************
 * @brief 释放资源
 * @param[in] handle TC句柄
 * @return 是否成功
 * ***************************************/
bool cryptoFree(TC_HANDLE* handle);

/*****************************************
 * @brief 创建密钥并持久化
 * @param[in] handle TC句柄
 * @param[out] PrimaryIndex 主键索引
 * @param[out] keyIndex 持久化的密钥索引
 * @param[in] auth 密钥认证信息
 * @return 是否成功
 * ***************************************/
bool cryptoCreateKey(TC_HANDLE handle, uint32_t* PrimaryIndex, uint32_t* keyIndex, const TC_BUFFER* auth);

/*****************************************
 * @brief 删除持久化的密钥
 * @param[in] handle TC句柄
 * @param[out] keyIndex 持久化的密钥索引
 * @return 是否成功
 * ***************************************/
bool cryptoDeleteKey(TC_HANDLE handle, uint32_t keyIndex);

/*****************************************
 * @brief 加密
 * @param[in] handle TC句柄
 * @param[in] keyIndex 密钥索引
 * @param[in] input 输入的明文数据
 * @param[out] output 输出的密文数据
 * @param[in] auth 密钥认证信息
 * @return 是否成功
 * ***************************************/
bool cryptoEncrypt(TC_HANDLE handle, uint32_t keyIndex, const TC_BUFFER* input, TC_BUFFER* output, const TC_BUFFER* auth);

/*****************************************
 * @brief 解密
 * @param[in] handle TC句柄
 * @param[in] keyIndex 密钥索引
 * @param[in] input 输入的密文数据
 * @param[out] output 输出的明文数据
 * @param[in] auth 密钥认证信息
 * @return 是否成功
 * ***************************************/
bool cryptoDecrypt(TC_HANDLE handle, uint32_t keyIndex, const TC_BUFFER* input, TC_BUFFER* output, const TC_BUFFER* auth);

#endif // DDE_CRYPTO_H