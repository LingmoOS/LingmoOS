// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <google/protobuf/service.h>
#include <google/protobuf/stubs/callback.h>
#include "rpccontroller.h"

namespace zrpc_ns {

void ZRpcController::Reset() {
}

bool ZRpcController::Failed() const {
    return m_is_failed;
}

std::string ZRpcController::ErrorText() const {
    return m_error_info;
}

void ZRpcController::StartCancel() {
}

void ZRpcController::SetFailed(const std::string &reason) {
    m_is_failed = true;
    m_error_info = reason;
}

bool ZRpcController::IsCanceled() const {
    return false;
}

void ZRpcController::NotifyOnCancel(google::protobuf::Closure *callback) {
}

void ZRpcController::SetErrorCode(const int error_code) {
    m_error_code = error_code;
}

int ZRpcController::ErrorCode() const {
    return m_error_code;
}

const std::string &ZRpcController::MsgSeq() const {
    return m_msg_req;
}

void ZRpcController::SetMsgReq(const std::string &msg_req) {
    m_msg_req = msg_req;
}

void ZRpcController::SetError(const int err_code, const std::string &err_info) {
    SetFailed(err_info);
    SetErrorCode(err_code);
}

void ZRpcController::SetPeerAddr(NetAddress::ptr addr) {
    m_peer_addr = addr;
}

void ZRpcController::SetLocalAddr(NetAddress::ptr addr) {
    m_local_addr = addr;
}
NetAddress::ptr ZRpcController::PeerAddr() {
    return m_peer_addr;
}

NetAddress::ptr ZRpcController::LocalAddr() {
    return m_local_addr;
}

void ZRpcController::SetTimeout(const int timeout) {
    m_timeout = timeout;
}
int ZRpcController::Timeout() const {
    return m_timeout;
}

void ZRpcController::SetMethodName(const std::string &name) {
    m_method_name = name;
}

std::string ZRpcController::GetMethodName() {
    return m_method_name;
}

void ZRpcController::SetMethodFullName(const std::string &name) {
    m_full_name = name;
}

std::string ZRpcController::GetMethodFullName() {
    return m_full_name;
}

} // namespace zrpc_ns