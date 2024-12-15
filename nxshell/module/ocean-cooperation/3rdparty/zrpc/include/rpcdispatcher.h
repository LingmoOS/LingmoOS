// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZRPC_RPCDISPATCHER_H
#define ZRPC_RPCDISPATCHER_H

#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <map>
#include <memory>

#include "abstractdispatcher.h"
#include "specdata.h"
// #include "co/tcp.h"

namespace zrpc_ns {

class ZRPC_API ZRpcDispacther : public AbstractDispatcher {
public:
    typedef std::shared_ptr<google::protobuf::Service> service_ptr;

    ZRpcDispacther() = default;
    ~ZRpcDispacther() = default;

    void dispatch(AbstractData *data, TcpConnection *conn);

    bool parseServiceFullName(const std::string &full_name,
                              std::string &service_name,
                              std::string &method_name);

    void registerService(service_ptr service);

public:
    // all services should be registerd on there before progress start
    // key: service_name
    std::map<std::string, service_ptr> m_service_map;
};

} // namespace zrpc

#endif
