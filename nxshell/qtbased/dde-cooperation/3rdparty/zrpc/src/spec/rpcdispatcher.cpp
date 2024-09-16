// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <sstream>

#include "abstractdispatcher.h"
#include "errorcode.h"
#include "specdata.h"
#include "rpcdispatcher.h"
#include "rpccontroller.h"
#include "rpcclosure.h"
#include "specodec.h"
#include "utils.h"

namespace zrpc_ns {

class TcpBuffer;

void ZRpcDispacther::dispatch(AbstractData *data, TcpConnection *conn) {
    SpecDataStruct *tmp = dynamic_cast<SpecDataStruct *>(data);

    if (tmp == nullptr) {
        ELOG << "dynamic_cast error";
        return;
    }

    // LOG << "begin to dispatch client request, msgno=" << tmp->msg_req;

    std::string service_name;
    std::string method_name;

    SpecDataStruct reply_pk;
    reply_pk.service_full_name = tmp->service_full_name;
    reply_pk.msg_req = tmp->msg_req;
    if (reply_pk.msg_req.empty()) {
        reply_pk.msg_req = Util::genMsgNumber();
    }

    if (!parseServiceFullName(tmp->service_full_name, service_name, method_name)) {
        ELOG << reply_pk.msg_req << "|parse service name " << tmp->service_full_name << "error";

        reply_pk.err_code = ERROR_PARSE_SERVICE_NAME;
        std::stringstream ss;
        ss << "cannot parse service_name:[" << tmp->service_full_name << "]";
        reply_pk.err_info = ss.str();
        conn->getCodec()->encode(conn->getOutBuffer(), dynamic_cast<AbstractData *>(&reply_pk));
        return;
    }

    auto it = m_service_map.find(service_name);
    if (it == m_service_map.end() || !((*it).second)) {
        reply_pk.err_code = ERROR_SERVICE_NOT_FOUND;
        std::stringstream ss;
        ss << "not found service_name:[" << service_name << "]";
        ELOG << reply_pk.msg_req << "|" << ss.str();
        reply_pk.err_info = ss.str();

        conn->getCodec()->encode(conn->getOutBuffer(), dynamic_cast<AbstractData *>(&reply_pk));

        // LOG << "end dispatch client request, msgno=" << tmp->msg_req;
        return;
    }

    service_ptr service = (*it).second;

    const google::protobuf::MethodDescriptor *method = service->GetDescriptor()->FindMethodByName(
        method_name);
    if (!method) {
        reply_pk.err_code = ERROR_METHOD_NOT_FOUND;
        std::stringstream ss;
        ss << "not found method_name:[" << method_name << "]";
        ELOG << reply_pk.msg_req << "|" << ss.str();
        reply_pk.err_info = ss.str();
        conn->getCodec()->encode(conn->getOutBuffer(), dynamic_cast<AbstractData *>(&reply_pk));
        return;
    }

    google::protobuf::Message *request = service->GetRequestPrototype(method).New();
    // DLOG << reply_pk.msg_req << "|request.name = " << request->GetDescriptor()->full_name();

    if (!request->ParseFromString(tmp->pb_data)) {
        reply_pk.err_code = ERROR_FAILED_SERIALIZE;
        std::stringstream ss;
        ss << "faild to parse request data, request.name:[" << request->GetDescriptor()->full_name()
           << "]";
        reply_pk.err_info = ss.str();
        ELOG << reply_pk.msg_req << "|" << ss.str();
        delete request;
        conn->getCodec()->encode(conn->getOutBuffer(), dynamic_cast<AbstractData *>(&reply_pk));
        return;
    }

    // LOG << "============================================================";
    // LOG << reply_pk.msg_req << "|Get client request data:" << request->ShortDebugString();
    // LOG << "============================================================";

    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // DLOG << reply_pk.msg_req << "|response.name = " << response->GetDescriptor()->full_name();

    ZRpcController rpc_controller;
    rpc_controller.SetMsgReq(reply_pk.msg_req);
    rpc_controller.SetMethodName(method_name);
    rpc_controller.SetMethodFullName(tmp->service_full_name);

    std::function<void()> reply_package_func = []() {};

    ZRpcClosure closure(reply_package_func);
    service->CallMethod(method, &rpc_controller, request, response, &closure);

    // DLOG << "Call [" << reply_pk.service_full_name << "] succ, now send reply package";

    if (!(response->SerializeToString(&(reply_pk.pb_data)))) {
        reply_pk.pb_data = "";
        ELOG << reply_pk.msg_req << "|reply error! encode reply package error";
        reply_pk.err_code = ERROR_FAILED_SERIALIZE;
        reply_pk.err_info = "failed to serilize relpy data";
    } else {
        // LOG << "============================================================";
        // LOG << reply_pk.msg_req << "|Set server response data:" << response->ShortDebugString();
        // LOG << "============================================================";
    }

    delete request;
    delete response;

    conn->getCodec()->encode(conn->getOutBuffer(), dynamic_cast<AbstractData *>(&reply_pk));
}

bool ZRpcDispacther::parseServiceFullName(const std::string &full_name,
                                          std::string &service_name,
                                          std::string &method_name) {
    if (full_name.empty()) {
        ELOG << "service_full_name empty";
        return false;
    }
    std::size_t i = full_name.find(".");
    if (i == full_name.npos) {
        ELOG << "not found [.]";
        return false;
    }

    service_name = full_name.substr(0, i);
    // DLOG << "service_name = " << service_name;
    method_name = full_name.substr(i + 1, full_name.length() - i - 1);
    // DLOG << "method_name = " << method_name;

    return true;
}

void ZRpcDispacther::registerService(service_ptr service) {
    std::string service_name = service->GetDescriptor()->full_name();
    m_service_map[service_name] = service;
    // LOG << "succ register service[" << service_name << "]!";
}

} // namespace zrpc_ns
