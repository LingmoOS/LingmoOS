// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OCEAN_TC_H
#define OCEAN_TC_H

#include "ocean_tc_copy.h"

#define OCEAN_TC_NAME(NAME) OCEAN_ ## NAME

extern TC_RC (*OCEAN_TC_NAME(TC_Start))(const uint8_t *device, TC_HANDLE *handle);
extern TC_RC (*OCEAN_TC_NAME(TC_CreatePrimary))(TC_HANDLE handle, const TC_ALG alg_hash, const uint32_t hierarchy, const TC_BUFFER *hierarchy_auth_msg, const TC_ALG alg_primary, const TC_BUFFER *primary_auth_msg, uint32_t *primary_index);
extern TC_RC (*OCEAN_TC_NAME(TC_Create))(TC_HANDLE handle, const TC_ALG alg_hash, const uint32_t primary_index, const TC_BUFFER *primary_auth_msg, const TC_ALG alg_key, const TC_BUFFER *key_auth_msg, uint32_t *key_index);
extern TC_RC (*OCEAN_TC_NAME(TC_Load))(TC_HANDLE handle, const uint32_t key_index, const TC_BUFFER *parent_auth_msg);
extern TC_RC (*OCEAN_TC_NAME(TC_EvictControl))(TC_HANDLE handle, const bool enable, const uint32_t persist_index, const uint32_t key_index, const uint32_t hierarchy, const TC_BUFFER *hierarchy_auth_msg);
extern TC_RC (*OCEAN_TC_NAME(TC_End))(TC_HANDLE *handle);
extern TC_RC (*OCEAN_TC_NAME(TC_Encrypt))(TC_HANDLE handle, const uint32_t key_index, const TC_BUFFER *key_auth_msg, const TC_ALG alg_encrypt, const TC_BUFFER *plain_text, TC_BUFFER *ciphter_text);
extern TC_RC (*OCEAN_TC_NAME(TC_Decrypt))(TC_HANDLE handle, const uint32_t key_index, const TC_BUFFER *key_auth_msg, const TC_ALG alg_decrypt, const TC_BUFFER *ciphter_text, TC_BUFFER *plain_text);

#define OCEAN_TC_INVOKE(func, ...) (func == NULL ? TC_ERR_NULL : func(__VA_ARGS__))

#define TC_Start(...)           OCEAN_TC_INVOKE(OCEAN_TC_NAME(TC_Start), ##__VA_ARGS__)
#define TC_CreatePrimary(...)   OCEAN_TC_INVOKE(OCEAN_TC_NAME(TC_CreatePrimary), ##__VA_ARGS__)
#define TC_Create(...)          OCEAN_TC_INVOKE(OCEAN_TC_NAME(TC_Create), ##__VA_ARGS__)
#define TC_Load(...)            OCEAN_TC_INVOKE(OCEAN_TC_NAME(TC_Load), ##__VA_ARGS__)
#define TC_EvictControl(...)    OCEAN_TC_INVOKE(OCEAN_TC_NAME(TC_EvictControl), ##__VA_ARGS__)
#define TC_End(...)             OCEAN_TC_INVOKE(OCEAN_TC_NAME(TC_End), ##__VA_ARGS__)
#define TC_Encrypt(...)         OCEAN_TC_INVOKE(OCEAN_TC_NAME(TC_Encrypt), ##__VA_ARGS__)
#define TC_Decrypt(...)         OCEAN_TC_INVOKE(OCEAN_TC_NAME(TC_Decrypt), ##__VA_ARGS__)

void oceanTcInit();
void oceanTcClose();

#endif /* OCEAN_TC_H */