// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <google/protobuf/service.h>
#include "zrpc.h"
#include "co/log.h"
#include "net/tcpserver.h"

namespace zrpc_ns {

ZRpcClient::ZRpcClient(const char *ip, uint16 port, bool ssl, const bool isLong) {
    zrpc_ns::NetAddress::ptr addr = std::make_shared<zrpc_ns::NetAddress>(ip, port, ssl);
    m_channel = std::make_shared<ZRpcChannel>(addr, isLong);

    m_controller = std::make_shared<ZRpcController>();
    // default timeout is 5 seconds
    m_controller.get()->SetTimeout(5000);
}

void ZRpcClient::setTimeout(uint32 timeout) {
    m_controller.get()->SetTimeout(static_cast<int>(timeout));
}

class ZRpcServerImpl {
public:
    ZRpcServerImpl(uint16 port, char *key, char *crt) {
        // ip is localhost: "0.0.0.0"
        zrpc_ns::NetAddress::ptr addr = std::make_shared<zrpc_ns::NetAddress>("0.0.0.0", port, key, crt);
        _tcpserver = std::make_shared<TcpServer>(addr);
    }

    ~ZRpcServerImpl() = default;

    TcpServer::ptr getServer() { return _tcpserver; }

    bool start() {
        if (_tcpserver == nullptr) {
            ELOG << "ZRPCServer::init failed!";
            return false;
        }

        _tcpserver->start();
        return true;
    }

    bool checkConnected() {
        if (_tcpserver == nullptr)
            return false;
        return _tcpserver->checkConnected();
    }

private:
    TcpServer::ptr _tcpserver{nullptr};
};

ZRpcServer::ZRpcServer(uint16 port, char *key, char *crt) {
    _p = co::make<ZRpcServerImpl>(port, key, crt);
}

ZRpcServer::~ZRpcServer() {
    co::del(static_cast<ZRpcServerImpl *>(_p));
}

bool ZRpcServer::doregister(std::shared_ptr<google::protobuf::Service> service) {
    TcpServer::ptr tcpserver = (static_cast<ZRpcServerImpl *>(_p))->getServer();
    return tcpserver->registerService(service);
}

bool ZRpcServer::start() {
    return (static_cast<ZRpcServerImpl *>(_p))->start();
}

void ZRpcServer::setCallBackFunc(const std::function<void (int, const fastring &, const uint16)> &call)
{
    (static_cast<ZRpcServerImpl *>(_p))->getServer()->setCallBackFunc(call);
}

bool ZRpcServer::checkConnected()
{
    return (static_cast<ZRpcServerImpl *>(_p))->checkConnected();
}

} // namespace zrpc_ns
