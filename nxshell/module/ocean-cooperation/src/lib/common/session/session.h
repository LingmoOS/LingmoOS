// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SESSION_H
#define SESSION_H

#include "proto_final_protocol.h"
#include <iostream>

enum SessionState {
    RPC_PINGOUT = -3,
    RPC_ERROR = -2,
    RPC_DISCONNECTED = -1,
    RPC_DISCONNECTING = 0,
    RPC_CONNECTING = 1,
    RPC_CONNECTED = 2,
};

class SessionCallInterface : public std::enable_shared_from_this<SessionCallInterface>
{
public:
    virtual void onReceivedMessage(const proto::OriginMessage &request, proto::OriginMessage *response) = 0;

    virtual bool onStateChanged(int state, std::string &msg) = 0;
};

#endif // SESSION_H
