// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZRPC_RPCCLOSURE_H
#define ZRPC_RPCCLOSURE_H

#include <google/protobuf/stubs/callback.h>
#include <functional>
#include <memory>
#include "zrpc_defines.h"

namespace zrpc_ns {

class ZRpcClosure : public google::protobuf::Closure {
public:
    typedef std::shared_ptr<ZRpcClosure> ptr;
    explicit ZRpcClosure(std::function<void()> cb)
        : m_cb(cb) {}

    ~ZRpcClosure() = default;

    void Run() {
        if (m_cb) {
            m_cb();
        }
    }

private:
    std::function<void()> m_cb{nullptr};
};

} // namespace zrpc_ns

#endif
