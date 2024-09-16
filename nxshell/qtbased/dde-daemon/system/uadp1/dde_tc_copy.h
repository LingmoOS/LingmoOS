// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// This file is copied from libtc-api

#ifndef DDE_TC_COPY_H
#define DDE_TC_COPY_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef uint32_t TC_RC;
typedef void* TC_HANDLE;

typedef struct{
    uint8_t         *buffer;
    uint32_t         size;
} TC_BUFFER;

typedef enum tc_alg_en {
    TC_NULL       = 0,
    TC_RSA,
    TC_SYMMETRIC,
    TC_SM2,
    TC_SHA1,
    TC_SHA256,
    TC_SM3,
    TC_SM4,
    TC_MAX
} TC_ALG;

#define NV_DEFAULT_BUFFER_SIZE 512

#define TC_TPM2_RH_OWNER       0x40000001
#define TC_TPM2_RH_ENDORSEMENT 0x4000000B
#define TC_TPM2_RH_PLATFORM    0x4000000C
#define TC_TPM2_RH_NULL        0x40000007

#define TC_SUCCESS                  0
#define TC_ERR_NULL                 1
#define TC_API_UNDEFINE             2
#define TC_TPM2_CONTEXT_INIT        4
#define TC_TPM2_CONTEXT_FREE        5
#define TC_TPM2_PUBLIC              6
#define TC_TPM2_PRIVATE             7
#define TC_TPM2_AUTH                8
#define TC_TPM2_PUBLIC_KEY_RSA      9
#define TC_TPM2_MAX_BUFFER          10
#define TC_TPM2_NAME                11
#define TC_NODE_OVERCAPACITY        15
#define TC_PARENT_INDEX             16
#define TC_OBJECT_INDEX             17
#define TC_COMMAND_START            18
#define TC_COMMAND_END              19
#define TC_COMMAND_LOAD             20
#define TC_COMMAND_EVICTCONTROL     21
#define TC_COMMAND_ENCRYPT          22
#define TC_COMMAND_DECRYPT          23
#define TC_COMMAND_CREATEPRIMARY    24
#define TC_COMMAND_CREATE           25
#define TC_COMMAND_SIGN             26
#define TC_COMMAND_VERIFYSIGNATURE  27
#define TC_COMMAND_HASH             28
#define TC_COMMAND_NVDEFINE         29
#define TC_COMMAND_NVRELEASE        30
#define TC_COMMAND_NVWRITE          31
#define TC_COMMAND_NVREAD           32
#define TC_NV_READPUBLIC            33
#define TC_NV_SIZE                  34
#define TC_NV_MAXBUFFER             35
#define TC_NV_OFFSET                36

#endif // DDE_TC_COPY_H