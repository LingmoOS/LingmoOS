// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZRPC_RPCCONRTOLLER_H
#define ZRPC_RPCCONRTOLLER_H

#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include <stdio.h>
#include <memory>
#include "netaddress.h"
#include "zrpc_defines.h"

namespace zrpc_ns {

class ZRPC_API ZRpcController : public google::protobuf::RpcController {

public:
    typedef std::shared_ptr<ZRpcController> ptr;
    // Client-side methods ---------------------------------------------

    ZRpcController() = default;

    ~ZRpcController() = default;

    void Reset() override;

    bool Failed() const override;

    // Server-side methods ---------------------------------------------

    std::string ErrorText() const override;

    void StartCancel() override;

    void SetFailed(const std::string &reason) override;

    bool IsCanceled() const override;

    void NotifyOnCancel(google::protobuf::Closure *callback) override;

    // common methods

    int ErrorCode() const;

    void SetErrorCode(const int error_code);

    const std::string &MsgSeq() const;

    void SetMsgReq(const std::string &msg_req);

    void SetError(const int err_code, const std::string &err_info);

    void SetPeerAddr(NetAddress::ptr addr);

    void SetLocalAddr(NetAddress::ptr addr);

    NetAddress::ptr PeerAddr();

    NetAddress::ptr LocalAddr();

    void SetTimeout(const int timeout);

    int Timeout() const;

    void SetMethodName(const std::string &name);

    std::string GetMethodName();

    void SetMethodFullName(const std::string &name);

    std::string GetMethodFullName();

private:
    int m_error_code{0};      // error_code, identify one specific error
    std::string m_error_info; // error_info, details description of error
    std::string m_msg_req;    // msg_req, identify once rpc request and response
    bool m_is_failed{false};
    bool m_is_cancled{false};
    NetAddress::ptr m_peer_addr;
    NetAddress::ptr m_local_addr;

    int m_timeout{5000};       // max call rpc timeout
    std::string m_method_name; // method name
    std::string m_full_name;   // full name, like server.method_name
};

} // namespace zrpc

#endif
