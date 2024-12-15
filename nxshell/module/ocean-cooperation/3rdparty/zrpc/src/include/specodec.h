// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZRPC_SPECODEC_H
#define ZRPC_SPECODEC_H

#include <stdint.h>
#include "abstractcodec.h"
#include "abstractdata.h"
#include "specdata.h"

namespace zrpc_ns {

class ZRpcCodeC : public AbstractCodeC {
public:
    ZRpcCodeC();

    ~ZRpcCodeC();

    // overwrite
    void encode(TcpBuffer *buf, AbstractData *data);

    // overwrite
    void decode(TcpBuffer *buf, AbstractData *data);

    const char *encodePbData(SpecDataStruct *data, uint32_t &len);
};

} // namespace zrpc_ns

#endif
