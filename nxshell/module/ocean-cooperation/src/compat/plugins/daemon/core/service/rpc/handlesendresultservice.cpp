// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "handlesendresultservice.h"
#include "common/commonstruct.h"
#include "common/constant.h"
#include "service/ipc/sendipcservice.h"
#include "ipc/proto/chan.h"
#include "ipc/bridge.h"
#include "service/comshare.h"
#include "utils/config.h"
#include "service/rpc/sendrpcservice.h"

#include "co/co.h"

HandleSendResultService::HandleSendResultService(QObject *parent) : QObject (parent)
{

}

void HandleSendResultService::handleSendResultMsg(const QString appName, const QString msg)
{
    SendResult res;
    co::Json res_json;
    if (!res_json.parse_from(msg.toStdString().c_str())) {
        ELOG << "handleSendResultMsg parse SendResult error!!!!!!!!";
        return;
    }
    res.from_json(res_json);

    if (res.errorType < INVOKE_OK) {
        SendStatus st;
        st.type = static_cast<int32>(res.protocolType);
        st.status = res.errorType;
        st.msg = msg.toStdString();

        // notifySendStatus
        QString jsonMsg = st.as_json().str().c_str();
        SendIpcService::instance()->handleSendToClient(appName, FRONT_SEND_STATUS, jsonMsg);
        return;
    }
    if (res.protocolType == LOGIN_INFO) {
        handleLogin(appName, res.data.c_str());
    }
}

void HandleSendResultService::handleLogin(const QString &appName, const QString &msg)
{
    co::Json res_json;
    UserLoginResultInfo res;
    if (!res_json.parse_from(msg.toStdString())) {
        ELOG << "handleLogin parse SendResult error!!!!!!!! msg:" << msg.toStdString();
        // error by can not connect target IP address.
        res.result = false;
        res.appName = appName.toStdString();
    } else {
        res.from_json(res_json);
        if (res.result) {
            // TODO: save the target peer info into target's map
            fastring token = res.token;
            PeerInfo target_info = res.peer;
            Comshare::instance()->updateStatus(CURRENT_STATUS_TRAN_CONNECT);
            // login successful
            DaemonConfig::instance()->saveAuthed(token);
            SendRpcService::instance()->addPing(res.appName.c_str());
        }
    }

    //cbConnect {GenericResult}
    co::Json req = {
        { "id", 0 },
        { "result", res.result ? 1 : 0 },
        // { "msg", (ip + " " + appName).toStdString() },
        { "msg", res.appName },
        { "isself", true},
    };

    QString jsonMsg = req.str().c_str();
    SendIpcService::instance()->handleSendToClient(appName, FRONT_CONNECT_CB, jsonMsg);
}
