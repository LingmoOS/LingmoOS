// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZRPC_SPECDATA_H
#define ZRPC_SPECDATA_H

#include <stdint.h>
#include <vector>
#include <string>
#include <memory>
#include "abstractdata.h"
#include "co/log.h"
#include "co/fastring.h"

namespace zrpc_ns {

class SpecDataStruct : public AbstractData {
public:
    typedef std::shared_ptr<SpecDataStruct> pb_ptr;
    SpecDataStruct() = default;
    ~SpecDataStruct() = default;
    SpecDataStruct(const SpecDataStruct &) = default;
    SpecDataStruct &operator=(const SpecDataStruct &) = default;
    SpecDataStruct(SpecDataStruct &&) = default;
    SpecDataStruct &operator=(SpecDataStruct &&) = default;

    /*
    **  min of package is: 1 + 4 + 4 + 4 + 4 + 4 + 4 + 1 = 26 bytes
    **
    */

    // char start;                      // indentify start of a spec data protocal data
    uint32_t pk_len{0};             // len of all package(include start char and end char)
    uint32_t msg_req_len{0};        // len of msg_req
    std::string msg_req;           // msg_req, which identify a request
    uint32_t service_name_len{0};   // len of service full name
    std::string service_full_name; // service full name, like QueryService.query_name
    uint32_t err_code{0}; // err_code, 0 -- call rpc success, otherwise -- call rpc failed. it only
                         // be seted by RpcController
    uint32_t err_info_len{0}; // len of err_info
    std::string err_info; // err_info, empty -- call rpc success, otherwise -- call rpc failed, it
                          // will display details of reason why call rpc failed. it only be seted by
                          // RpcController
    std::string pb_data;  // business pb data
    int32_t check_num{-1}; // check_num of all package. to check legality of data
    // char end;                        // identify end of a spec data protocal data
};

} // namespace zrpc_ns

#endif
