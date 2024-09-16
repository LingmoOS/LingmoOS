// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZRPC_ZRPC_H
#define ZRPC_ZRPC_H

#include <google/protobuf/service.h>
#include <memory>
#include <stdio.h>
#include <functional>
#include "rpcchannel.h"
#include "rpccontroller.h"

namespace zrpc_ns {

class ZRPC_API ZRpcClient {

public:
    ZRpcClient(const char *ip, uint16 port, bool ssl = true, const bool isLong = false);

    void setTimeout(uint32 timeout);

    ZRpcChannel* getChannel() const { return m_channel.get(); }
    ZRpcController* getControler() const { return m_controller.get(); }

private:
    ZRpcChannel::ptr m_channel{nullptr};
    ZRpcController::ptr m_controller{nullptr};
};

class ZRPC_API ZRpcServer {

public:
    ZRpcServer(uint16 port, char *key, char *crt);
    ~ZRpcServer();

    template <class T>
    bool registerService()
    {
        return doregister(std::make_shared<T>());
    }

    bool start();

    void setCallBackFunc(const std::function<void(int, const fastring &, const uint16)> &call);

    bool checkConnected();
private:
    bool doregister(std::shared_ptr<google::protobuf::Service> service);

    void* _p;
};

} // namespace zrpc

#endif
