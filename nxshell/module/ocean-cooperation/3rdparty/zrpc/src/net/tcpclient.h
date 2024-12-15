// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZRPC_TCPCLIENT_H
#define ZRPC_TCPCLIENT_H

#include <memory>
#include <google/protobuf/service.h>

#include "netaddress.h"
#include "tcpconnection.h"

namespace zrpc_ns {

class TcpClient {
public:
    typedef std::shared_ptr<TcpClient> ptr;

    TcpClient(NetAddress::ptr addr);

    ~TcpClient();

    bool tryConnect();

    int sendAndRecvData(const std::string &msg_no, SpecDataStruct::pb_ptr &res);

    void stop();

    bool connected();

    bool connect();

    TcpConnection *getConnection();

    void setTimeout(const int v) { m_max_timeout = v; }

    void setTryCounts(const int v) { m_try_counts = v; }

    const std::string &getErrInfo() { return m_err_info; }

    NetAddress::ptr getPeerAddr() const { return m_peer_addr; }

    NetAddress::ptr getLocalAddr() const { return m_local_addr; }

    AbstractCodeC::ptr getCodeC() { return m_codec; }

private:
    tcp::Client *_tcp_cli;

    int m_try_counts{3};      // max try reconnect times
    int m_max_timeout{10000}; // max connect timeout, ms
    bool m_is_stop{false};
    std::string m_err_info; // error info of client

    NetAddress::ptr m_local_addr{nullptr};
    NetAddress::ptr m_peer_addr{nullptr};
    TcpConnection::ptr m_connection{nullptr};

    AbstractCodeC::ptr m_codec{nullptr};

    bool m_connect_succ{false};
    CallBackFunc callback { nullptr };
};

} // namespace zrpc_ns

#endif
