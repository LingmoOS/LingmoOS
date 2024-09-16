// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iostream>
#include <google/protobuf/service.h>
#include "rpcchannel.h"
#include "rpccontroller.h"
#include "netaddress.h"
#include "sample.pb.h"
#include "co/co.h"
#include "co/time.h"

#include "zrpc.h"

void test_client() {
    zrpc_ns::ZRpcClient *client = new zrpc_ns::ZRpcClient("10.8.11.98", 8899, true);
    QueryService_Stub stub(client->getChannel());
    zrpc_ns::ZRpcController *rpc_controller = client->getControler();

    queryAgeReq rpc_req;
    queryAgeRes rpc_res;

    rpc_req.set_id(555);
    rpc_req.set_req_no(666);

    std::cout << "requeset body: " << rpc_req.ShortDebugString() << std::endl;
    stub.query_age(rpc_controller, &rpc_req, &rpc_res, NULL);

    if (rpc_controller->ErrorCode() != 0) {
        std::cout << "Failed to call server, error code: " << rpc_controller->ErrorCode()
                  << ", error info: " << rpc_controller->ErrorText() << std::endl;
        return;
    }

    std::cout << "response body: " << rpc_res.ShortDebugString() << std::endl;
}

int main(int argc, char *argv[]) {
    flag::parse(argc, argv);

    for (int i = 0; i < 16; ++i) {
        go(test_client);
        co::sleep(1);
    }

    sleep::sec(2);
    return 0;
}
