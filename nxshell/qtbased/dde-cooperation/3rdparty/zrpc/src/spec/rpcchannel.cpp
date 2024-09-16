// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <memory>
#include <google/protobuf/service.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include "netaddress.h"
#include "errorcode.h"
#include "rpcchannel.h"
#include "rpccontroller.h"
#include "specodec.h"
#include "specdata.h"
#include "co/log.h"
#include "../net/tcpclient.h"

#include "utils.h"

namespace zrpc_ns {

TcpClient::ptr m_clientlong = nullptr;
ZRpcChannel::ZRpcChannel(NetAddress::ptr addr, const bool isLong)
    : m_addr(addr)
    , isLongConnect (isLong)
{
}

ZRpcChannel::~ZRpcChannel()
{
    if (m_clientlong && isLongConnect)
        m_clientlong = nullptr;
}

void ZRpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                             google::protobuf::RpcController *controller,
                             const google::protobuf::Message *request,
                             google::protobuf::Message *response,
                             google::protobuf::Closure *done) {

    SpecDataStruct pb_struct;
    ZRpcController *rpc_controller = dynamic_cast<ZRpcController *>(controller);
    if (!rpc_controller) {
        ELOG << "call failed. falid to dynamic cast ZRpcController";
        return;
    }
    if (isLongConnect && m_clientlong == nullptr)
        m_clientlong = std::make_shared<TcpClient>(m_addr);

    TcpClient::ptr m_client = isLongConnect ? m_clientlong : nullptr;
    if (m_client.get() == nullptr) {        
        m_client = std::make_shared<TcpClient>(m_addr);
    }

    if (!m_client->tryConnect()) {
        rpc_controller->SetError(ERROR_FAILED_CONNECT, "failed to connect");
        ELOG << "client can not connect to server: " << m_addr.get()->toString();
        return;
    }
    // rpc_controller->SetLocalAddr(m_client->getLocalAddr());
    // rpc_controller->SetPeerAddr(m_client->getPeerAddr());

    pb_struct.service_full_name = method->full_name();
    // DLOG << "call service_name = " << pb_struct.service_full_name;
    if (!request->SerializeToString(&(pb_struct.pb_data))) {
        ELOG << "serialize send package error";
        return;
    }

    if (!rpc_controller->MsgSeq().empty()) {
        pb_struct.msg_req = rpc_controller->MsgSeq();
    } else {
        pb_struct.msg_req = Util::genMsgNumber();
        // DLOG << "generate new msgno = " << pb_struct.msg_req;
        rpc_controller->SetMsgReq(pb_struct.msg_req);
    }

    AbstractCodeC::ptr m_codec = m_client->getConnection()->getCodec();
    m_codec->encode(m_client->getConnection()->getOutBuffer(), &pb_struct);
    if (!pb_struct.encode_succ) {
        rpc_controller->SetError(ERROR_FAILED_ENCODE, "encode data error");
        return;
    }

    // LOG << "============================================================";
    // LOG << pb_struct.msg_req << "|" << rpc_controller->PeerAddr()->toString()
    //     << "|. Set client send request data:" << request->ShortDebugString();
    // LOG << "============================================================";
    m_client->setTimeout(rpc_controller->Timeout());

    SpecDataStruct::pb_ptr res_data;
    int rt = m_client->sendAndRecvData(pb_struct.msg_req, res_data);
    if (rt != 0) {
        rpc_controller->SetError(rt, m_client->getErrInfo());
        ELOG << pb_struct.msg_req
             << "|call rpc occur client error, service_full_name=" << pb_struct.service_full_name
             << ", error_code=" << rt << ", error_info = " << m_client->getErrInfo();
        return;
    }

    if (!response->ParseFromString(res_data->pb_data)) {
        rpc_controller->SetError(ERROR_FAILED_DESERIALIZE,
                                 "failed to deserialize data from server");
        ELOG << pb_struct.msg_req << "|failed to deserialize data";
        return;
    }
    if (res_data->err_code != 0) {
        ELOG << pb_struct.msg_req << "|server reply error_code=" << res_data->err_code
             << ", err_info=" << res_data->err_info;
        rpc_controller->SetError(static_cast<int>(res_data->err_code), res_data->err_info);
        return;
    }

    // LOG << "============================================================";
    // LOG << pb_struct.msg_req << "|" << rpc_controller->PeerAddr()->toString()
    //     << "|call rpc server [" << pb_struct.service_full_name << "] succ"
    //     << ". Get server reply response data:" << response->ShortDebugString();
    // LOG << "============================================================";

    // excute callback function
    if (done) {
        done->Run();
    }
}

} // namespace zrpc_ns
