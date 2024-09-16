// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZRPC_ABSTRACTDISPATCHER_H
#define ZRPC_ABSTRACTDISPATCHER_H

#include <memory>
#include <google/protobuf/service.h>
#include "abstractdata.h"
#include "../net/tcpconnection.h"

namespace zrpc_ns {

class TcpConnection;

class AbstractDispatcher {
public:
    typedef std::shared_ptr<AbstractDispatcher> ptr;

    AbstractDispatcher() {}

    virtual ~AbstractDispatcher() {}

    virtual void dispatch(AbstractData *data, TcpConnection *conn) = 0;
};

} // namespace zrpc_ns

#endif
