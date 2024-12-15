// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZRPC_TCPCONNECTION_H
#define ZRPC_TCPCONNECTION_H

#include <memory>
#include <vector>
#include <queue>
#include <map>
#include "co/log.h"
#include "co/tcp.h"

#include "tcpbuffer.h"
#include "specodec.h"
#include "netaddress.h"

using CallBackFunc = std::function<void(int, const fastring &, const uint16)>;
namespace zrpc_ns {

class TcpServer;
class TcpClient;

// the max socket buffer len
#define PERPKG_MAX_LEN 16 * 1024

enum TcpConnectionState {
    NotConnected = 1,   // can do io
    Connected = 2,   // can do io
    HalfClosing = 3,   // server call shutdown, write half close. can read,but can't write
    Closed = 4,   // can't do io
};

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{

public:
    typedef std::shared_ptr<TcpConnection> ptr;

    TcpConnection(TcpServer *tcp_svr,
                  tcp::Connection *serv_conn,
                  int buff_size,
                  NetAddress::ptr peer_addr);

    TcpConnection(TcpClient *tcp_cli,
                  tcp::Client *cli_conn,
                  int buff_size,
                  NetAddress::ptr peer_addr);

    void setCallBack(const CallBackFunc &call);

    void setUpClient();

    ~TcpConnection();

    void initBuffer(int size);

    enum ConnectionType {
        ServerConnection = 1,   // owned by tcp_server
        ClientConnection = 2,   // owned by tcp_client
    };

public:
    void shutdownConnection();

    TcpConnectionState getState();

    void setState(const TcpConnectionState &state);

    TcpBuffer *getInBuffer();

    TcpBuffer *getOutBuffer();

    AbstractCodeC::ptr getCodec() const;

    bool getResPackageData(const std::string &msg_req, SpecDataStruct::pb_ptr &pb_struct);

    fastring getRemoteIp();

public:
    void MainServerLoopCorFunc();

    bool input();

    void execute();

    void output();

    void initServer();

    bool waited();

private:
    int64 read_hook(char *buf, int len);
    int64 write_hook(const void *buf, int count);
    void clearClient();

private:
    TcpServer *m_tcp_svr { nullptr };
    TcpClient *m_tcp_cli { nullptr };

    NetAddress::ptr m_peer_addr;

    tcp::Connection *m_serv_conn { nullptr };
    tcp::Client *m_cli_conn { nullptr };
    int m_trans_timeout { 3000 };   // max receive or send timeout, ms

    TcpConnectionState m_state { TcpConnectionState::Connected };
    ConnectionType m_connection_type { ServerConnection };

    TcpBuffer::ptr m_read_buffer;
    TcpBuffer::ptr m_write_buffer;

    AbstractCodeC::ptr m_codec;

    bool m_stop { false };

    std::map<std::string, std::shared_ptr<SpecDataStruct>> m_reply_datas;

    fastring remoteIP;

    CallBackFunc callback { nullptr };
    int64 _rev_start_time = 0;
};

}   // namespace zrpc_ns

#endif
