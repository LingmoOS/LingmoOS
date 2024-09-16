// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "threads/thread.h"

#include "protoendpoint.h"

void ProtoEndpoint::setCallbacks(std::shared_ptr<SessionCallInterface> callbacks)
{
    _callbacks = callbacks;
}

void ProtoEndpoint::sendDisRequest()
{
    proto::DisconnectRequest dis;
    this->request(dis);
}

proto::OriginMessage ProtoEndpoint::syncRequest(const std::string &target, const proto::OriginMessage &msg)
{
    _active_traget = target;
    _self_request.store(true, std::memory_order_relaxed);

    try {
        return this->request(msg).get();
    } catch (const std::exception &ex) {
        // 捕获并处理异常
        std::cout << "syncRequest throw exception: " << ex.what() << std::endl;
    }

    return proto::OriginMessage();
}

void ProtoEndpoint::asyncRequestWithHandler(const std::string &target, const proto::OriginMessage &request, RpcHandler resultHandler)
{
    asyncRequest(target, request);  // 发送异步请求

    int32_t type = request.mask;  // 获取请求的类型
    // 启动一个新的线程来处理结果
    std::thread handlerThread([this, type, resultHandler]() {
        std::string response = getResponse(type);  // 获取响应
        resultHandler(type, response);  // 调用结果处理函数
    });

    // 在启动的新线程中处理结果，主线程可以继续执行其他任务
    handlerThread.detach();
}


void ProtoEndpoint::asyncRequest(const std::string &target, const proto::OriginMessage &msg)
{
    _active_traget = target;
    _self_request.store(true, std::memory_order_relaxed);

    // new request comeing, reset all pending.
    this->reset_requests();
    //std::cout << "pending request: " << _responses_by_type.size() << std::endl;

    int32_t type = msg.mask;
    std::future<proto::OriginMessage> futureResponse = this->request(msg);

    _responses_by_type.insert(std::make_pair(type, std::move(futureResponse)));
}

std::string ProtoEndpoint::getResponse(int32_t type)
{
    std::scoped_lock locker(this->_lock);

    auto it = _responses_by_type.find(type);
    if (it != _responses_by_type.end())
    {
        std::future<proto::OriginMessage>& future = it->second;

        int wait_cout = 0;
        do {
            if (future.valid() && future.wait_for(std::chrono::milliseconds(wait_cout > 0 ? 1 : 0)) == std::future_status::ready) {
                // 如果有响应，立即返回
                try {
                    auto res = future.get();
                    _responses_by_type.erase(it);
                    return res.json_msg;
                } catch (const std::exception &ex) {
                    // 捕获并处理异常
                    std::cout << "future throw exception: " << ex.what() << std::endl;
                    break;
                }
            }
            wait_cout++;
            CppCommon::Thread::Yield();
        } while (wait_cout < 3000); // timeout 3s

        _responses_by_type.erase(it);
    }
    return "";
}
