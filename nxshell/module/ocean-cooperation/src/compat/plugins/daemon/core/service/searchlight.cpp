// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QDebug>
#include <utils/utils.h>
#include "searchlight.h"
#include "co/co/sock.h"
#include "co/json.h"
#include "co/time.h"
#include "co/co.h"
#include "co/log.h"
#include "common/constant.h"
#include "ipc/proto/comstruct.h"
#include "jobmanager.h"

//DEF_int32(max_idle, 3000, "max_idle");
DEF_string(udp_ip, "0.0.0.0", "udp_ip");
DEF_int32(udp_port, 30001, "udp_port");
DEF_string(mcast_ip, "239.255.0.1", "mcast_ip");

static QMutex _search_ip_lock;
static QStringList filter;
static std::atomic_bool _send_tcp { false };

namespace searchlight {

Discoverer::Discoverer(const fastring& listen_for_service,
           const on_services_changed_t on_services_changed
          )
    : _listen_for_service(listen_for_service)
    , _on_services_changed(on_services_changed)
{
    {
        QWriteLocker lk(&_discovered_lock);
        _discovered_services.clear();
    }
}

Discoverer::~Discoverer()
{
    _stop = true;
    {
        QWriteLocker lk(&_discovered_lock);
        _discovered_services.clear();
    }
}

void Discoverer::start()
{
    _stop = true;
    sock_t sockfd = co::udp_socket();
    if (sockfd < 0) {
        ELOG << "Failed to create socket";
        return;
    }

    struct sockaddr_in addr;
    bool init = co::init_addr(&addr, FLG_udp_ip.c_str(), FLG_udp_port);
    if (!init) {
        ELOG << "Failed to init address";
        co::close(sockfd);
        return;
    }

    int res = co::bind(sockfd, &addr, sizeof(addr));
    if (res < 0) {
        ELOG << "Failed to bind address";
        co::close(sockfd);
        return;
    }

    //给 socket 设置 SO_REUSEADDR 选项，一般 server 端的 listening socket 需要设置这个选项，防止 server 重启后 bind 失败。
    co::set_reuseaddr(sockfd);

recheck:
    // 将套接字加入到组播组
    struct ip_mreq mreq{};
    mreq.imr_multiaddr.s_addr = inet_addr(FLG_mcast_ip.c_str());
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (co::setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) < 0) {
        ELOG << "Failed to join multicast group, retry after 3s.";
        sleep::ms(3000); // 注册组播失败，3秒后重试
        goto recheck;
    }

    struct sockaddr_in cli;
    int len = sizeof(cli);
    char buffer[1024];

    _stop = false;

    _timer.restart();
    // 定时更新发现设备
    QUNIGO([this](){
        while (!_stop) {
            sleep::ms(1000); //co::sleep(1000);
            remove_idle_services();
            // callback discovery changed
            QList<service> _changed;
            {
                QReadLocker lk(&_discovered_lock);
                _changed = _change_sevices;
                _change_sevices.clear();
            }
            if (_changed.isEmpty())
                continue;
            _on_services_changed(_changed);
        }
    });

    LOG << "discoverer server start";
    // 接收组播数据包
    while (!_stop) {
        memset(buffer, 0, sizeof(buffer));
        int recv_len = co::recvfrom(sockfd, buffer, sizeof(buffer), &cli, &len);
        if (recv_len < 0) {
            // LOG << "discoverer server recvfrom error: " << co::strerror();
            co::sleep(100);
            continue;
        }

        fastring msg(buffer, static_cast<size_t>(recv_len));
        handle_message(msg, co::addr2str(&cli));
    }

    // 关闭套接字
    co::close(sockfd);

    _stop = true;
}

bool Discoverer::started() {
    return !_stop;
}

void Discoverer::exit()
{
    _stop = true;
}

void Discoverer::setSearchIp(const QString &ip)
{
    QMutexLocker lk(&_search_ip_lock);
    filter.clear();
    _send_tcp = true;
    if (!ip.isEmpty() && !filter.contains(ip)) {
        filter.append(ip);
    }
}

void Discoverer::handle_message(const fastring& message, const fastring& sender_endpoint,
                                const bool isFilter)
{
    // 处理接收到的数据
    // LOG << "server recv ==== " << message << " from " << sender_endpoint;
    co::Json node;
    if (!node.parse_from(message)) {
        DLOG << "Invalid service, ignore!!!!!";
        return;
    }

    fastring name = node.get("name").as_string();
    fastring info = node.get("info").as_string();
    QString  ip = node.get("info","os","ipv4").as_string().c_str();


    QString endpoint(sender_endpoint.c_str());
    endpoint = endpoint.left(endpoint.indexOf(":"));
    // 判断同网段
    auto preHost = ip.lastIndexOf(".") > ip.size()
            ? ip : ip.mid(0, ip.lastIndexOf("."));
    fastring self_ip = Util::getFirstIp();
    bool filterContain { false };
    QStringList filters;
    {
        QMutexLocker lk(&_search_ip_lock);
        filterContain = filter.contains(ip);
        if (filterContain && isFilter)
            _send_tcp = false;
    }
    if (message.starts_with(_listen_for_service) && ip != QString(self_ip.c_str())
            && (filterContain || !isFilter || QString(self_ip.c_str()).startsWith(preHost))) {
        // 找到最近的时间修改，只发送改变了的
        handleChanges(endpoint, info, _timer.ms());
    } else {
        auto discovered_service = service
            {
                name,
                endpoint.toStdString(),
                info,
                0,
                _timer.ms(),
            };
        //DLOG << "ignoring: " << discovered_service;
    }
}

bool Discoverer::remove_idle_services()
{
    auto dead_line = _timer.ms() - 3000;//FLG_max_idle;
    bool removed = false;

    QWriteLocker lk(&_discovered_lock);
    for (auto i = _discovered_services.begin(); i != _discovered_services.end();)
    {
        if (i.value()->last_seen < dead_line) {
            service t(*(i.value()));
            i = _discovered_services.erase(i);
            t.flags = 1;
            _change_sevices.removeOne(t);
            _change_sevices.append(t);
            removed = true;
        } else {
            ++i;
        }
    }

    return removed;
}

void Discoverer::handleChanges(const QString &endpoint, const fastring &info, const qint64 time)
{
    if (endpoint.isEmpty()) {
        ELOG << " ip is null !!!!! endpoint = " << endpoint.toStdString() << " info = " << info;
        return;
    }
    QWriteLocker lk(&_discovered_lock);
    auto key = endpoint;
    auto _ser = _discovered_services.value(key);
    if (_ser.isNull()) {
        auto discovered_service = service
            {
                _listen_for_service,
                endpoint.toStdString(),
                info,
                0,
                _timer.ms(),
            };
        _discovered_services.insert(endpoint, QSharedPointer<service>(new service(discovered_service)));
        _change_sevices.append(discovered_service);
        return;
    }
    _ser->last_seen = time;
    if (_ser->info.compare(info) != 0) {
        _ser->info = info;
        service t(*_ser);
        t.flags = 2;
        _change_sevices.removeOne(t);
        _change_sevices.append(t);
    }
}


Announcer::Announcer(const fastring& service_name,
          const uint16_t service_port,
          const fastring& info
          )
    : _service_name(service_name)
    , _service_port(service_port)
    , _base_info(info)
{
    _app_infos.clear();
}

Announcer::~Announcer()
{
    _stop = true;
}

void Announcer::updateBase(const fastring &info)
{
    _base_info = info;
}

fastring Announcer::baseInfo() const
{
    return _base_info;
}

void Announcer::appendApp(const fastring &info)
{
    int idx = sameApp(info);
    if (idx >= 0) {
        //remove old one
        _app_infos.remove(idx);
    }
    _app_infos.push_back(info);
}

void Announcer::removeApp(const fastring &info)
{
    int idx = sameApp(info);
    if (idx >= 0) {
        //remove find one
        _app_infos.remove(idx);
    }
}

void Announcer::removeAppbyName(const fastring &name)
{
    for (size_t i = 0; i < _app_infos.size(); ++i) {
        co::Json oajson;
        if (!oajson.parse_from(_app_infos[i])) {
            ELOG << "remove incorrect app node format:" << _app_infos[i];
            _app_infos.remove(i);
            continue;
        }
        fastring appname = oajson.get("appname").as_string();
        if (appname.compare(name) == 0) {
            _app_infos.remove(i);
        }
    }
}

void Announcer::start(handleTcpDiscover handle)
{
    _stop = true;
    sock_t sockfd = co::udp_socket();
    if (sockfd < 0) {
        ELOG << "Failed to create socket";
        return;
    }

    // 设置允许发送组播数据包回环：关闭，过滤来自本机的包
    int enable = 0;
    if (co::setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, &enable, sizeof(enable)) < 0) {
        ELOG << "Failed to set socket options";
        co::close(sockfd);
        return;
    }

    // 填充目标组播地址和端口号
    struct sockaddr_in dest_addr;
    int len = sizeof(dest_addr);
    bool init = co::init_addr(&dest_addr, FLG_mcast_ip.c_str(), FLG_udp_port);
    if (!init) {
        ELOG << "Failed to set destination address";
        co::close(sockfd);
        return;
    }

    _stop = false;

    LOG << "announcer server start";
    // 发送数据包 int sendto(sock_t fd, const void* buf, int n, const void* dst_addr, int addrlen, int ms=-1);
    while (!_stop) {
        fastring message = udpSendPackage();

        // DLOG << "UDP send: === " << message;
        int send_len = co::sendto(sockfd, message.c_str(), static_cast<int>(message.size()), &dest_addr, len);
        if (send_len < 0 || Util::getFirstIp() == "")
            ELOG << "Failed to send data";

        co::sleep(1000); // announcer every second

        QMutexLocker lk(&_search_ip_lock);
        if (_send_tcp && !filter.empty()) {
            handle(filter.first());
        }
    }

    // 关闭套接字
    co::close(sockfd);

    _finished = true;
    _stop = true;
}

bool Announcer::started() {
    return !_stop;
}

void Announcer::exit()
{
    _stop = true;
}

fastring Announcer::udpSendPackage()
{
    co::Json node;
    node.add_member("name", _service_name);
    node.add_member("port", _service_port);
    co::Json baseJson;
    baseJson.parse_from(_base_info);
    //NodeInfo
    co::Json nodeinfo;
    NodePeerInfo nodepeer;
    nodepeer.from_json(baseJson);
    nodepeer.ipv4 = Util::getFirstIp();
    nodeinfo.add_member("os", nodepeer.as_json());
    co::Json appinfos;
    for (size_t i = 0; i < _app_infos.size(); ++i) {
        co::Json appjson;
        if (appjson.parse_from(_app_infos[i])) {
            appinfos.push_back(appjson);
        }
    }
    nodeinfo.add_member("apps", appinfos);

    // update the last info
    node.remove("info");
    node.add_member("info", nodeinfo);
    return node.str();
}

fastring Announcer::nodeInfoStr()
{
    co::Json baseJson;
    baseJson.parse_from(_base_info);
    //NodeInfo
    co::Json nodeinfo;
    NodePeerInfo nodepeer;
    nodepeer.from_json(baseJson);
    nodepeer.ipv4 = Util::getFirstIp();
    nodeinfo.add_member("os", nodepeer.as_json());
    co::Json appinfos;
    for (size_t i = 0; i < _app_infos.size(); ++i) {
        co::Json appjson;
        if (appjson.parse_from(_app_infos[i])) {
            appinfos.push_back(appjson);
        }
    }
    nodeinfo.add_member("apps", appinfos);

    return nodeinfo.str();
}

int Announcer::sameApp(const fastring &info)
{
    co::Json appjson;
    if (!appjson.parse_from(info)) {
        ELOG << "incorrect app node info:" << info;
        return -1;
    }
    fastring appname = appjson.get("appname").as_string();
    for (size_t i = 0; i < _app_infos.size(); ++i) {
        co::Json oajson;
        if (!oajson.parse_from(_app_infos[i])) {
            ELOG << "remove incorrect app node format:" << _app_infos[i];
            _app_infos.remove(i);
            continue;
        }
        fastring name = oajson.get("appname").as_string();
        if (name.compare(appname) == 0) {
            return i;
        }
    }
    return -1;
}

} // searchlight
