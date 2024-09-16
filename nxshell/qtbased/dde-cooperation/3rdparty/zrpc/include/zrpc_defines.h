// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZRPC_DEFINES_H
#define ZRPC_DEFINES_H

#ifdef WIN32
    #ifdef EXPORT_ZRPC_API
        #define ZRPC_API __declspec(dllexport)
    #else
        #define ZRPC_API __declspec(dllimport)
    #endif
#else
    #define ZRPC_API
#endif

#endif // ZRPC_DEFINES_H
