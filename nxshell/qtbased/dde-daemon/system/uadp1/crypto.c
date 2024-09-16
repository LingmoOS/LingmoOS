// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "crypto.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DDE_TC_KEY_BASE 0x81000001
#define DDE_TC_KEY_MAX (DDE_TC_KEY_BASE + 64)

// 为了避免被内核摔黑锅，使用 come to nothing 来表示函数调用失败
#define DDE_TC_CHECK(FUNC, ...)                                     \
        do                                                          \
        {                                                           \
            TC_RC err = FUNC(__VA_ARGS__);                          \
            if (err != TC_SUCCESS)                                  \
            {                                                       \
                fprintf(stderr, #FUNC " come to nothing %u\n", err);\
                return false;                                       \
            }                                                       \
        }while(0)

/*****************************************
 * @brief 初始化加解密环境
 * @param[out] handle TC句柄
 * @param[out] PrimaryIndex 主键索引
 * @param[out] keyIndex 密钥索引
 * @return 是否成功
 * ***************************************/
bool cryptoInit(TC_HANDLE* handle)
{
    ddeTcInit();
    // 初始化可信环境
    DDE_TC_CHECK(TC_Start, "tabrmd", handle);

    return true;
}

/*****************************************
 * @brief 创建密钥并持久化
 * @param[in] handle TC句柄
 * @param[out] PrimaryIndex 主键索引
 * @param[out] keyIndex 持久化的密钥索引
 * @param[in] auth 密钥认证信息
 * @return 是否成功
 * ***************************************/
bool cryptoCreateKey(TC_HANDLE handle, uint32_t* PrimaryIndex, uint32_t* keyIndex, const TC_BUFFER* auth)
{
    // 创建主键并获取索引
    DDE_TC_CHECK(TC_CreatePrimary, handle, TC_SHA256, TC_TPM2_RH_ENDORSEMENT, NULL, TC_RSA, auth, PrimaryIndex);

    // 创建密钥并获取索引
    uint32_t rawKeyIndex;
    DDE_TC_CHECK(TC_Create, handle, TC_SHA256, *PrimaryIndex, auth, TC_RSA, auth, &rawKeyIndex);
    DDE_TC_CHECK(TC_Load, handle, rawKeyIndex, auth);

    // 创建持久化密钥
    for(*keyIndex = DDE_TC_KEY_BASE; (*keyIndex) < DDE_TC_KEY_MAX; (*keyIndex)++)
    {
        if(TC_SUCCESS == TC_EvictControl(handle, true, *keyIndex, rawKeyIndex, TC_TPM2_RH_OWNER, NULL))
        {
            return true;
        }
    }

    return false;
}

/*****************************************
 * @brief 删除持久化的密钥
 * @param[in] handle TC句柄
 * @param[in] keyIndex 持久化的密钥索引
 * @return 是否成功
 * ***************************************/
bool cryptoDeleteKey(TC_HANDLE handle, uint32_t keyIndex)
{
    DDE_TC_CHECK(TC_EvictControl, handle, false, keyIndex, keyIndex, TC_TPM2_RH_OWNER, NULL);

    return true;
}

/*****************************************
 * @brief 释放资源
 * @param[in] handle TC句柄
 * @return 是否成功
 * ***************************************/
bool cryptoFree(TC_HANDLE* handle)
{
    DDE_TC_CHECK(TC_End, handle);
    ddeTcClose();

    return true;
}

/*****************************************
 * @brief 加密
 * @param[in] handle TC句柄
 * @param[in] keyIndex 密钥索引
 * @param[in] input 输入的明文数据
 * @param[out] output 输出的密文数据
 * @return 是否成功
 * ***************************************/
bool cryptoEncrypt(TC_HANDLE handle, uint32_t keyIndex, const TC_BUFFER* input, TC_BUFFER* output, const TC_BUFFER* auth)
{
    // 加密
    DDE_TC_CHECK(TC_Encrypt, handle, keyIndex, auth, TC_RSA, input, output);

    return true;
}

/*****************************************
 * @brief 解密
 * @param[in] handle TC句柄
 * @param[in] keyIndex 密钥索引
 * @param[in] input 输入的密文数据
 * @param[out] output 输出的明文数据
 * @return 是否成功
 * ***************************************/
bool cryptoDecrypt(TC_HANDLE handle, uint32_t keyIndex, const TC_BUFFER* input, TC_BUFFER* output, const TC_BUFFER* auth)
{
    // 解密
    DDE_TC_CHECK(TC_Decrypt, handle, keyIndex, auth, TC_RSA, input, output);

    return true;
}