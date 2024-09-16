// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdio.h>
#include <dlfcn.h>
#include "dde_tc.h"

static const char* libTcApiPath = "libtc-api.so";
static void* libTcApi = NULL;

extern TC_RC (*DDE_TC_NAME(TC_Start))(const uint8_t *device, TC_HANDLE *handle) = NULL;
extern TC_RC (*DDE_TC_NAME(TC_CreatePrimary))(TC_HANDLE handle, const TC_ALG alg_hash, const uint32_t hierarchy, const TC_BUFFER *hierarchy_auth_msg, const TC_ALG alg_primary, const TC_BUFFER *primary_auth_msg, uint32_t *primary_index) = NULL;
extern TC_RC (*DDE_TC_NAME(TC_Create))(TC_HANDLE handle, const TC_ALG alg_hash, const uint32_t primary_index, const TC_BUFFER *primary_auth_msg, const TC_ALG alg_key, const TC_BUFFER *key_auth_msg, uint32_t *key_index) = NULL;
extern TC_RC (*DDE_TC_NAME(TC_Load))(TC_HANDLE handle, const uint32_t key_index, const TC_BUFFER *parent_auth_msg) = NULL;
extern TC_RC (*DDE_TC_NAME(TC_EvictControl))(TC_HANDLE handle, const bool enable, const uint32_t persist_index, const uint32_t key_index, const uint32_t hierarchy, const TC_BUFFER *hierarchy_auth_msg) = NULL;
extern TC_RC (*DDE_TC_NAME(TC_End))(TC_HANDLE *handle) = NULL;
extern TC_RC (*DDE_TC_NAME(TC_Encrypt))(TC_HANDLE handle, const uint32_t key_index, const TC_BUFFER *key_auth_msg, const TC_ALG alg_encrypt, const TC_BUFFER *plain_text, TC_BUFFER *ciphter_text) = NULL;
extern TC_RC (*DDE_TC_NAME(TC_Decrypt))(TC_HANDLE handle, const uint32_t key_index, const TC_BUFFER *key_auth_msg, const TC_ALG alg_decrypt, const TC_BUFFER *ciphter_text, TC_BUFFER *plain_text) = NULL;

#ifdef __USE_GNU
    #define DDE_DLOPEN(lmid, file, mode)    dlmopen(lmid, file, mode)
    #define DDE_DLVSYM(handle, func, v)     dlvsym(handle, func, v)
#endif

#define DDE_DLOPEN(file, mode)      dlopen(file, mode)
#define DDE_DLSYM(handle, func)     dlsym(handle, func)
#define DDE_DLCLOSE(handle)         dlclose(handle)
#define DDE_DLERROR()               dlerror()

#define DDE_DL_CALL(ret, func, ...)                             \
        do                                                      \
        {                                                       \
            ret = func(__VA_ARGS__);                            \
            if (ret == NULL)                                    \
            {                                                   \
                fprintf(stderr, "%s\n", DDE_DLERROR());         \
            }                                                   \
        }                                                       \
        while(0)

#define DDE_TC_LOAD_SYM(lib, func) DDE_DL_CALL(DDE_TC_NAME(func), DDE_DLSYM, lib, #func)

#define DDE_TC_LOAD(lib, func)                                  \
        do                                                      \
        {                                                       \
            if (lib != NULL)                                    \
                DDE_TC_LOAD_SYM(lib, func);                     \
            if (DDE_TC_NAME(func) != NULL)                      \
            {                                                   \
                printf("%s @ %p\n", #func, DDE_TC_NAME(func));  \
            }                                                   \
        }while(0)

void ddeTcInit()
{
    DDE_DL_CALL(libTcApi, DDE_DLOPEN, libTcApiPath, RTLD_LAZY);
    DDE_TC_LOAD(libTcApi, TC_Start);
    DDE_TC_LOAD(libTcApi, TC_CreatePrimary);
    DDE_TC_LOAD(libTcApi, TC_Create);
    DDE_TC_LOAD(libTcApi, TC_Load);
    DDE_TC_LOAD(libTcApi, TC_EvictControl);
    DDE_TC_LOAD(libTcApi, TC_End);
    DDE_TC_LOAD(libTcApi, TC_Encrypt);
    DDE_TC_LOAD(libTcApi, TC_Decrypt);
}


void ddeTcClose()
{
    if (libTcApi != NULL)
        dlclose(libTcApi);

    libTcApi = NULL;
}