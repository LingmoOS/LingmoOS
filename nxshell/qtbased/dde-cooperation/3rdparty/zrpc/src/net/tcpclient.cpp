// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//#include <sys/socket.h>
//#include <arpa/inet.h>
#include <sstream>
#include "co/log.h"
#include "netaddress.h"
#include "tcpclient.h"
#include "specodec.h"

namespace zrpc_ns {

TcpClient::TcpClient(NetAddress::ptr addr)
    : m_peer_addr(addr) {

    m_codec = std::make_shared<ZRpcCodeC>();

    char *ip = m_peer_addr.get()->getIP();
    uint16 port = static_cast<uint16>(m_peer_addr.get()->getPort());
    bool ssl = m_peer_addr.get()->isSSL();

    _tcp_cli = new tcp::Client(ip, port, ssl);

    m_connection = std::make_shared<TcpConnection>(this, _tcp_cli, PERPKG_MAX_LEN, m_peer_addr);
}

TcpClient::~TcpClient() {
    if (this->connected()) {
        this->stop();
        // DLOG << "~TcpClient() close : " << _tcp_cli->socket();
    }
}

bool TcpClient::tryConnect() {
    if (!this->connected() && !this->connect()) {
        if (callback) {
            fastring ip = m_peer_addr ? m_peer_addr->getIP() : "";
            uint16 port = m_peer_addr ? static_cast<uint16>(m_peer_addr->getPort()) : 0;
            callback(1, ip, port); // 1 超时
        }
        return false;
    } else {
        return true;
    }
}

bool TcpClient::connected() {
    return _tcp_cli->connected();
}

bool TcpClient::connect() {
    // DLOG << "try connect...";
    return _tcp_cli->connect(m_max_timeout);
}

TcpConnection *TcpClient::getConnection() {
    if (!m_connection.get()) {
        m_connection = std::make_shared<TcpConnection>(this, _tcp_cli, PERPKG_MAX_LEN, m_peer_addr);
    }
    return m_connection.get();
}

int TcpClient::sendAndRecvData(const std::string &msg_no, SpecDataStruct::pb_ptr &res) {
    if (!this->connected()) {
        std::stringstream ss;
        ss << "connect peer addr[" << m_peer_addr->toString()
           << "] error. sys error=" << strerror(errno);
        m_err_info = ss.str();
        return -1;
    }

    m_connection->setUpClient();
    m_connection->output();

    while (!m_connection->getResPackageData(msg_no, res)) {
        // DLOG << "redo getResPackageData";
        m_connection->input();

        if (m_connection->getState() == Closed) {
            ELOG << "peer close";
            goto err_deal;
        }

        m_connection->execute();
    }

    m_err_info = "";
    return 0;

err_deal:

    // FIXME: distory m_connection ?
    this->stop();
    return -1;
}

void TcpClient::stop() {
    _tcp_cli->disconnect();
    if (!m_is_stop) {
        m_is_stop = true;
    }
}

} // namespace zrpc_ns
