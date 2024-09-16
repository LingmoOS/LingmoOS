// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "discoveryjob.h"
#include "searchlight.h"
#include "ipc/proto/comstruct.h"
#include "ipc/bridge.h"
#include "service/ipc/sendipcservice.h"
#include "service/rpc/remoteservice.h"
#include "service/comshare.h"

#include "common/constant.h"
#include "co/log.h"
#include "co/json.h"
#include "co/co.h"
#include "utils/utils.h"

#include <QMap>
#include <QtConcurrent>

DiscoveryJob::DiscoveryJob(QObject *parent)
    : QObject(parent)
{
    {
        QWriteLocker lk(&_dis_lock);
        _dis_node_maps.clear();
    }
}

DiscoveryJob::~DiscoveryJob()
{
    {
        QWriteLocker lk(&_dis_lock);
        _dis_node_maps.clear();
    }
    // free discoverer pointer
    if (_discoverer_p) {
        auto p = (searchlight::Discoverer*)_discoverer_p;
        if (!p->started()) co::del(p);
        _discoverer_p = nullptr;
    }

    // free announcer pointer
    if (_announcer_p) {
        auto p = (searchlight::Announcer*)_announcer_p;
        if (!p->started()) co::del(p);
        _announcer_p = nullptr;
    }
}

void DiscoveryJob::discovererRun()
{
    _discoverer_p = co::make<searchlight::Discoverer>(
        "{\"name\":\"ulink_service\",\"port\"",
        [this](const QList<searchlight::Discoverer::service> & services)
        {
            QWriteLocker lk(&_dis_lock);
            for(auto& service : services) {
                co::Json node;
                node.parse_from(service.info);
                co::Json osjson = node.get("os");
                if (osjson.is_null() || !osjson.has_member("uuid")) {
                    //DLOG << "found error format: " << service;
                    continue;
                }

                fastring uid = osjson.get("uuid").as_string();

                auto it = _dis_node_maps.find(uid);
                if (it != _dis_node_maps.end()) {
                    if (service.flags == 1) { // 下线
                        it->second.second = false;
                        continue;
                    }
                    // has been recorded, markd it exist
                    it->second.second = true;
                    if (it->second.first.compare(service.info) != 0)
                        compareOldAndNew(uid, service.info.c_str(), it);
                } else { // 上线
                    //new node discovery.
                    //DLOG << "new peer found: " << node.str();
                    emit sigNodeChanged(true, QString(service.info.c_str()));
                    _dis_node_maps.insert(uid, std::make_pair(service.info, true));
                }
            }
            // loop and notify all not exist node.
            for (auto it = _dis_node_maps.begin(); it != _dis_node_maps.end(); ++it) {
                if (it->second.second)
                    continue;
                auto msg = it->second.first;
                co::Json node;
                node.parse_from(it->second.first);
                NodeInfo nodeInfo;
                nodeInfo.from_json(node);
                //DLOG << "peer losted: " << it->second.first;
                nodeInfo.apps.clear();
                emit sigNodeChanged(false, QString(nodeInfo.as_json().str().c_str()));
                _dis_node_maps.erase(it);
            }
        }
    );
    ((searchlight::Discoverer*)_discoverer_p)->start();
}

void DiscoveryJob::announcerRun(const fastring &info)
{
    _announcer_p = co::make<searchlight::Announcer>("ulink_service", UNI_RPC_PORT_BASE, info);

    ((searchlight::Announcer*)_announcer_p)->start([this](const QString &ip){
        UNIGO([this, ip](){
            auto selfIp = Util::getFirstIp();
            if (selfIp.empty())
                return;
            RemoteServiceSender sender("dde-cooperation", ip, 51597, false);
            DiscoverInfo req, res;
            req.ip = selfIp;
            req.msg = ((searchlight::Announcer*)_announcer_p)->udpSendPackage();
            auto result = sender.doSendProtoMsg(DISCOVER_BY_TCP, req.as_json().str().c_str(), QByteArray());
            if (result.errorType < INVOKE_OK)
                return;

            co::Json json;
            if (!json.parse_from(result.data))
                return;
            res.from_json(json);
            handleUpdPackage(res.ip.c_str(), res.msg.c_str());
        });
    });
}

void DiscoveryJob::stopDiscoverer()
{
    if (_discoverer_p == nullptr)
        return;
    ((searchlight::Discoverer*)_discoverer_p)->exit();
}

void DiscoveryJob::stopAnnouncer()
{
    if (_announcer_p == nullptr)
        return;
    ((searchlight::Announcer*)_announcer_p)->exit();
    while (!((searchlight::Announcer*)_announcer_p)->finished()) {
        co::sleep(100);
    }

}

void DiscoveryJob::updateAnnouncBase(const fastring info)
{
    ((searchlight::Announcer*)_announcer_p)->updateBase(info);
}

void DiscoveryJob::updateAnnouncApp(bool remove, const fastring info)
{
    if (remove) {
        ((searchlight::Announcer*)_announcer_p)->removeApp(info);
    } else {
        ((searchlight::Announcer*)_announcer_p)->appendApp(info);
    }
}

void DiscoveryJob::updateAnnouncShare(const bool remove, const fastring connectIP)
{
    QMutexLocker lk(&_lock);
    auto _base = ((searchlight::Announcer*)_announcer_p)->baseInfo();
    co::Json _base_json;
    if (!_base_json.parse_from(_base)) {
        ELOG << "parse from base info error !!!";
        return;
    }

    NodePeerInfo _info;
    _info.from_json(_base_json);
    if (remove) {
        _info.share_connect_ip = "";
    } else if (!connectIP.empty()) {
        _info.share_connect_ip = connectIP;
    } else {
        return;
    }
    updateAnnouncBase(_info.as_json().str());
}

void DiscoveryJob::removeAppbyName(const fastring name)
{
    if (name.compare("dde-cooperation") == 0)
        updateAnnouncShare(true);
    ((searchlight::Announcer*)_announcer_p)->removeAppbyName(name);
}

fastring DiscoveryJob::baseInfo() const
{
    QMutexLocker lk(&_lock);
    return ((searchlight::Announcer*)_announcer_p)->baseInfo();
}

co::list<fastring> DiscoveryJob::getNodes()
{
    co::list<fastring> notes;
    QReadLocker lk(&_dis_lock);
    for (auto it = _dis_node_maps.begin(); it != _dis_node_maps.end(); ++it) {
        notes.push_back(it->second.first);
    }
    return notes;
}

void DiscoveryJob::searchDeviceByIp(const QString &ip, const bool remove)
{
    if (remove) {
        ((searchlight::Discoverer*)_discoverer_p)->setSearchIp("");
        return;
    }

    auto offline = Util::getFirstIp().empty();
    if (offline) {
        notifySearchResult(false, ip.toStdString().c_str());
        return;
    }

    int64 startTime = QDateTime::currentMSecsSinceEpoch();
    RemoteServiceSender sender("dde-cooperation", ip, 51597, false);
    Comshare::instance()->searchIp(QString::number(quintptr(&sender))+ip, startTime);
    PingPong ping;
    ping.ip = "search-ping";
    auto result = sender.doSendProtoMsg(SEARCH_DEVICE_BY_IP, ping.as_json().str().c_str(), QByteArray());
    if (!Comshare::instance()->checkSearchRes(QString::number(quintptr(&sender))+ip, startTime))
        return;
    if (result.errorType < INVOKE_OK){
        // 通知前端搜索失败
        // 通知前端搜索结果
        WLOG << "try SEARCH_DEVICE_BY_IP invoke FAILED!";
        notifySearchResult(false, ip.toStdString().c_str());

        ((searchlight::Discoverer*)_discoverer_p)->setSearchIp("");
        return;
    }
    ((searchlight::Discoverer*)_discoverer_p)->setSearchIp(ip);
    emit sigNodeChanged(true, result.data.c_str());

    notifySearchResult(true, result.data);
}

void DiscoveryJob::notifySearchResult(bool result, const fastring &info)
{
    SearchDeviceResult ev;
    ev.result = result;
    ev.msg = info;

    QString jsonMsg = ev.as_json().str().c_str();
    SendIpcService::instance()->handleSendToClient("dde-cooperation", FRONT_SEARCH_IP_DEVICE_RESULT, jsonMsg);
}

fastring DiscoveryJob::udpSendPackage()
{
    return ((searchlight::Announcer*)_announcer_p)->udpSendPackage();
}

fastring DiscoveryJob::nodeInfoStr()
{
    return ((searchlight::Announcer*)_announcer_p)->nodeInfoStr();
}

void DiscoveryJob::handleUpdPackage(const QString &ip, const QString &msg)
{
    return ((searchlight::Discoverer*)_discoverer_p)->
            handle_message(msg.toStdString(), ip.toStdString(), false);
}

void DiscoveryJob::compareOldAndNew(const fastring &uid, const QString &cur,
                                    const co::lru_map<fastring, std::pair<fastring, bool>>::iterator &it)
{
    co::Json oldnode, curnode;
    oldnode.parse_from(it->second.first);
    curnode.parse_from(cur.toStdString());
    NodeInfo oldInfo, curInfo;
    oldInfo.from_json(oldnode);
    curInfo.from_json(curnode);

    if (!oldInfo.apps.empty() && curInfo.apps.empty()) {
        //node has been unregister or losted.
        emit sigNodeChanged(false, cur);
        _dis_node_maps.erase(it);
    } else if (oldInfo.apps.empty() && !curInfo.apps.empty()){
        //node info has been updated, force update now.
        _dis_node_maps.erase(it);
        emit sigNodeChanged(true, cur);
        _dis_node_maps.insert(uid, std::make_pair(cur.toStdString(), true));
    } else if (!oldInfo.apps.empty() && !curInfo.apps.empty()) {
        QMap<QString, fastring> oldname, curname;
        for (const auto &app : curInfo.apps) {
            curname.insert(app.appname.c_str(), app.json);
        }
        bool up = false, down = false;
        for (const auto &app : oldInfo.apps) {
            oldname.insert(app.appname.c_str(), app.json);
            // 新的不包含老的，下线
            if (!curname.contains(app.appname.c_str())) {
                down = true;
            } else { // 对比内容是否相当
                if (curname.value(app.appname.c_str()).compare(app.json) != 0)
                    up = true;
            }
        }
        if (down) {
            //node has been unregister or losted.
            emit sigNodeChanged(false, cur);
            _dis_node_maps.erase(it);
        }

        for (const auto &app : curname.keys()) {
            // 老的不包含新的，上线
            if (!oldname.contains(app)) {
                up = true;
                break;
            }
        }

        if (up) {
            //node info has been updated, force update now.
            if (!down)
                _dis_node_maps.erase(it);
            emit sigNodeChanged(true, cur);
            _dis_node_maps.insert(uid, std::make_pair(cur.toStdString(), true));
        }
    }
}
