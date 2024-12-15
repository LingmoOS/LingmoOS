// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZRPC_RPCCHANNEL_H
#define ZRPC_RPCCHANNEL_H

#include <memory>
#include <google/protobuf/service.h>
#include "netaddress.h"
// #include "co/tcp.h"

namespace zrpc_ns {

class ZRPC_API ZRpcChannel : public google::protobuf::RpcChannel {

public:
    typedef std::shared_ptr<ZRpcChannel> ptr;
    ZRpcChannel(NetAddress::ptr addr, const bool isLong);
    ~ZRpcChannel() override;

    void CallMethod(const google::protobuf::MethodDescriptor *method,
                    google::protobuf::RpcController *controller,
                    const google::protobuf::Message *request,
                    google::protobuf::Message *response,
                    google::protobuf::Closure *done) override;

private:
    NetAddress::ptr m_addr;
    bool isLongConnect { false };
};

} // namespace zrpc

#endif
