// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZRPC_ABSTRACTCODEC_H
#define ZRPC_ABSTRACTCODEC_H

#include <string>
#include <memory>
#include "../net/tcpbuffer.h"
#include "abstractdata.h"

namespace zrpc_ns {

class AbstractCodeC {

public:
    typedef std::shared_ptr<AbstractCodeC> ptr;

    AbstractCodeC() {}

    virtual ~AbstractCodeC() {}

    virtual void encode(TcpBuffer *buf, AbstractData *data) = 0;

    virtual void decode(TcpBuffer *buf, AbstractData *data) = 0;
};

} // namespace zrpc_ns

#endif
