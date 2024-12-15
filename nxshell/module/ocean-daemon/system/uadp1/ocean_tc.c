// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <stdio.h>
#include <dlfcn.h>
#include "ocean_tc.h"

static const char* libTcApiPath = "libtc-api.so";
static void* libTcApi = NULL;

extern TC_RC (*OCEAN_TC_NAME(TC_Start))(const uint8_t *device, TC_HANDLE *handle) = NULL;
extern TC_RC (*OCEAN_TC_NAME(TC_CreatePrimary))(TC_HANDLE handle, const TC_ALG alg_hash, const uint32_t hierarchy, const TC_BUFFER *hierarchy_auth_msg, const TC_ALG alg_primary, const TC_BUFFER *primary_auth_msg, uint32_t *primary_index) = NULL;
extern TC_RC (*OCEAN_TC_NAME(TC_Create))(TC_HANDLE handle, const TC_ALG alg_hash, const uint32_t primary_index, const TC_BUFFER *primary_auth_msg, const TC_ALG alg_key, const TC_BUFFER *key_auth_msg, uint32_t *key_index) = NULL;
extern TC_RC (*OCEAN_TC_NAME(TC_Load))(TC_HANDLE handle, const uint32_t key_index, const TC_BUFFER *parent_auth_msg) = NULL;
extern TC_RC (*OCEAN_TC_NAME(TC_EvictControl))(TC_HANDLE handle, const bool enable, const uint32_t persist_index, const uint32_t key_index, const uint32_t hierarchy, const TC_BUFFER *hierarchy_auth_msg) = NULL;
extern TC_RC (*OCEAN_TC_NAME(TC_End))(TC_HANDLE *handle) = NULL;
extern TC_RC (*OCEAN_TC_NAME(TC_Encrypt))(TC_HANDLE handle, const uint32_t key_index, const TC_BUFFER *key_auth_msg, const TC_ALG alg_encrypt, const TC_BUFFER *plain_text, TC_BUFFER *ciphter_text) = NULL;
extern TC_RC (*OCEAN_TC_NAME(TC_Decrypt))(TC_HANDLE handle, const uint32_t key_index, const TC_BUFFER *key_auth_msg, const TC_ALG alg_decrypt, const TC_BUFFER *ciphter_text, TC_BUFFER *plain_text) = NULL;

#ifdef __USE_GNU
    #define OCEAN_DLOPEN(lmid, file, mode)    dlmopen(lmid, file, mode)
    #define OCEAN_DLVSYM(handle, func, v)     dlvsym(handle, func, v)
#endif

#define OCEAN_DLOPEN(file, mode)      dlopen(file, mode)
#define OCEAN_DLSYM(handle, func)     dlsym(handle, func)
#define OCEAN_DLCLOSE(handle)         dlclose(handle)
#define OCEAN_DLERROR()               dlerror()

#define OCEAN_DL_CALL(ret, func, ...)                             \
        do                                                      \
        {                                                       \
            ret = func(__VA_ARGS__);                            \
            if (ret == NULL)                                    \
            {                                                   \
                fprintf(stderr, "%s\n", OCEAN_DLERROR());         \
            }                                                   \
        }                                                       \
        while(0)

#define OCEAN_TC_LOAD_SYM(lib, func) OCEAN_DL_CALL(OCEAN_TC_NAME(func), OCEAN_DLSYM, lib, #func)

#define OCEAN_TC_LOAD(lib, func)                                  \
        do                                                      \
        {                                                       \
            if (lib != NULL)                                    \
                OCEAN_TC_LOAD_SYM(lib, func);                     \
            if (OCEAN_TC_NAME(func) != NULL)                      \
            {                                                   \
                printf("%s @ %p\n", #func, OCEAN_TC_NAME(func));  \
            }                                                   \
        }while(0)

void oceanTcInit()
{
    OCEAN_DL_CALL(libTcApi, OCEAN_DLOPEN, libTcApiPath, RTLD_LAZY);
    OCEAN_TC_LOAD(libTcApi, TC_Start);
    OCEAN_TC_LOAD(libTcApi, TC_CreatePrimary);
    OCEAN_TC_LOAD(libTcApi, TC_Create);
    OCEAN_TC_LOAD(libTcApi, TC_Load);
    OCEAN_TC_LOAD(libTcApi, TC_EvictControl);
    OCEAN_TC_LOAD(libTcApi, TC_End);
    OCEAN_TC_LOAD(libTcApi, TC_Encrypt);
    OCEAN_TC_LOAD(libTcApi, TC_Decrypt);
}


void oceanTcClose()
{
    if (libTcApi != NULL)
        dlclose(libTcApi);

    libTcApi = NULL;
}