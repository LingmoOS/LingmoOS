// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZRPC_UTIL_H
#define ZRPC_UTIL_H

#include <stdint.h>
#include <string.h>
#include "co/byte_order.h"
#include "co/rand.h"

namespace zrpc_ns {

class Util {
public:
    static uint32 netByteToInt32(const char *buf) {
        uint32 tmp;
        memcpy(&tmp, buf, sizeof(tmp));
        return ntoh32(tmp);
    }

    static std::string genMsgNumber() {
        int t_msg_req_len = 8;
        return std::string(co::randstr(t_msg_req_len).c_str());
    }
};

} // namespace zrpc_ns

#endif
