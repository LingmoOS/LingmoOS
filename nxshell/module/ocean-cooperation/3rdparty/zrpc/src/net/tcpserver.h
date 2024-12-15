// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZRPC_TCPSERVER_H
#define ZRPC_TCPSERVER_H

#include <map>
#include <google/protobuf/service.h>
#include "netaddress.h"
#include "tcpconnection.h"
#include "abstractcodec.h"
#include "abstractdispatcher.h"
#include "zrpc_defines.h"

namespace zrpc_ns {

ZRPC_API class TcpServer {

public:
    typedef std::shared_ptr<TcpServer> ptr;

    TcpServer(NetAddress::ptr addr);

    ~TcpServer();

    void on_connection_cb(tcp::Connection conn);

    void start();

    bool started();

    void exit();

    bool registerService(std::shared_ptr<google::protobuf::Service> service);

    void setCallBackFunc(const CallBackFunc &callback);

    bool checkConnected();

public:
    AbstractDispatcher::ptr getDispatcher();

    AbstractCodeC::ptr getCodec();

    NetAddress::ptr getPeerAddr();

    NetAddress::ptr getLocalAddr();

private:
    TcpConnection::ptr addClient(tcp::Connection *conntion);

private:
    tcp::Server _tcp_serv;

    bool _started;
    bool _stopped;

    NetAddress::ptr m_addr;

    int m_tcp_counts{0};

    bool m_is_stop_accept{false};

    AbstractDispatcher::ptr m_dispatcher;

    AbstractCodeC::ptr m_codec;

    std::map<int, std::shared_ptr<TcpConnection>> m_clients;

    CallBackFunc callback { nullptr };
};

} // namespace zrpc_ns

#endif
